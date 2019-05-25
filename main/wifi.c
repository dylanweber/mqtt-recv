/*
	MQTT Reciever - wifi.c
	Copyright 2019 Dylan Weber

	Licensed under the Apache License, Version 2.0 (the "License");
	you may not use this file except in compliance with the License.
	You may obtain a copy of the License at

		http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language governing permissions and
	limitations under the License.
 */
#include "wifi.h"

static const char *TAG = "wifi";

static EventGroupHandle_t s_wifi_event_group;

const int WIFI_CONNECTED_BIT = 0x1;

static int s_retry_num = 0;

static esp_err_t event_handler(void *ctx, system_event_t *event) {
	switch (event->event_id) {
		case SYSTEM_EVENT_STA_START:
			tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_STA, CONFIG_DHCP_NAME);
			esp_wifi_connect();
			break;
		case SYSTEM_EVENT_STA_GOT_IP:
			ESP_LOGI(TAG, "Recieved IP address:%s",
					 ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
			s_retry_num = 0;
			xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
			break;
		case SYSTEM_EVENT_STA_DISCONNECTED: {
			ESP_LOGI(TAG, "Connection failed\n");
			if (s_retry_num < CONFIG_ESP_MAXIMUM_RETRY) {
				esp_wifi_connect();
				xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
				s_retry_num++;
				ESP_LOGI(TAG, "Retrying connection");
			} else {
				remove("/spiffs/wifi.ssid");
				remove("/spiffs/wifi.pass");
				esp_err_t ret = wifi_startap();
				if (ret == ESP_OK)
					start_httpserver();
			}
			break;
		}
		default:
			break;
	}
	return ESP_OK;
}

esp_err_t wifi_restore() {
	size_t len;
	char *ssid;
	char *pass;

	FILE *fp = fopen("/spiffs/wifi.ssid", "r");
	if (fp == NULL) {
		ESP_LOGE(TAG, "Failed to read wifi ssid file.");
		return ESP_FAIL;
	}

	fseek(fp, 0, SEEK_END);
	len = ftell(fp);

	ssid = malloc(len * sizeof(*ssid));
	fseek(fp, 0, SEEK_SET);
	fread(ssid, len, 1, fp);
	fclose(fp);

	fp = fopen("/spiffs/wifi.pass", "r");
	if (fp == NULL) {
		ESP_LOGE(TAG, "Failed to read wifi pass file.");
		return ESP_FAIL;
	}

	fseek(fp, 0, SEEK_END);
	len = ftell(fp);

	pass = malloc(len * sizeof(*pass));
	fseek(fp, 0, SEEK_SET);
	fread(pass, len, 1, fp);
	fclose(fp);

	wifi_connect(ssid, pass);

	free(ssid);
	free(pass);
	return ESP_OK;
}

esp_err_t wifi_connect(char *ssid, char *pass) {
	s_wifi_event_group = xEventGroupCreate();

	tcpip_adapter_init();
	esp_event_loop_init(event_handler, NULL);

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	esp_wifi_init(&cfg);
	wifi_config_t wifi_config = {.sta = {.ssid = {0}, .password = {0}}};

	strncpy((char *)wifi_config.sta.ssid, ssid, 32);
	strncpy((char *)wifi_config.sta.password, pass, 64);

	esp_wifi_set_mode(WIFI_MODE_STA);
	esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
	esp_err_t ret = esp_wifi_start();

	if (ret != ESP_OK) {
		return ESP_FAIL;
	}

	ESP_LOGI(TAG, "WIFI init finished.");
	ESP_LOGI(TAG, "Connected to AP SSID: \"%s\" password:\"%s\"", ssid, pass);
	return ESP_OK;
}

esp_err_t wifi_startap() {
	char ssid[20];
	uint8_t mac[6];

	esp_err_t ret = esp_efuse_mac_get_default(mac);
	if (ret != ESP_OK) {
		ESP_LOGE(TAG, "Failed to get MAC address.");
		return ESP_FAIL;
	}

	memset(ssid, 0, sizeof(ssid));
	sprintf(ssid, "%x%x%x%x%x%x-setup", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

	ESP_LOGI(TAG, "Starting AP...");

	s_wifi_event_group = xEventGroupCreate();

	tcpip_adapter_init();
	esp_event_loop_init(event_handler, NULL);

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	esp_wifi_init(&cfg);
	wifi_config_t wifi_config = {.ap = {.ssid = {0}, .max_connection = 1}};

	strncpy((char *)wifi_config.ap.ssid, ssid, 32);

	esp_wifi_set_mode(WIFI_MODE_AP);
	esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config);
	ret = esp_wifi_start();
	if (ret != ESP_OK) {
		ESP_LOGE(TAG, "Failed to start AP wifi.");
		return ESP_FAIL;
	}

	return ESP_OK;
}
