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

const int WIFI_CONNECTED_BIT_4 = BIT0;
const int WIFI_CONNECTED_BIT_6 = BIT1;

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
			xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT_4);
			break;
		case SYSTEM_EVENT_AP_STA_GOT_IP6:
			xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT_6);
			break;
		case SYSTEM_EVENT_STA_CONNECTED:
			tcpip_adapter_create_ip6_linklocal(TCPIP_ADAPTER_IF_STA);
			break;
		case SYSTEM_EVENT_STA_DISCONNECTED: {
			ESP_LOGI(TAG, "Connection failed\n");
			if (s_retry_num < CONFIG_ESP_MAXIMUM_RETRY) {
				esp_wifi_connect();
				xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT_4);
				xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT_6);
				s_retry_num++;
				vTaskDelay(4000 / portTICK_PERIOD_MS);
				ESP_LOGI(TAG, "Retrying connection...");
			} else {
#ifdef DELETE_ON_FAIL
				remove("/spiffs/wifi.ssid");
				remove("/spiffs/wifi.pass");
				remove("/spiffs/wifi.bssid");
#endif
				char **network_list;
				esp_err_t ret = wifi_scan(&network_list);
				if (ret != ESP_OK) {
					ESP_LOGE(TAG, "Failed to scan Wi-Fi.");
				}
				ret = wifi_startap();
				if (ret == ESP_OK) {
					start_httpserver(network_list);
				}
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

	ssid = malloc((len + 1) * sizeof(*ssid));
	fseek(fp, 0, SEEK_SET);
	fread(ssid, len, 1, fp);
	ssid[len] = '\0';
	fclose(fp);

	fp = fopen("/spiffs/wifi.pass", "r");
	if (fp == NULL) {
		ESP_LOGE(TAG, "Failed to read wifi pass file.");
		return ESP_FAIL;
	}

	fseek(fp, 0, SEEK_END);
	len = ftell(fp);

	pass = malloc((len + 1) * sizeof(*pass));
	fseek(fp, 0, SEEK_SET);
	fread(pass, len, 1, fp);
	pass[len] = '\0';
	fclose(fp);

	uint8_t bssid[6];
	fp = fopen("/spiffs/wifi.bssid", "r");
	if (fp == NULL) {
		ESP_LOGI(TAG, "BSSID file not found.");
		wifi_connect(ssid, pass, NULL);
	} else {
		fseek(fp, 0, SEEK_SET);
		fread(bssid, 6, 1, fp);
		fclose(fp);

		ESP_LOGI(TAG, "BSSID is: %x:%x:%x:%x:%x:%x", bssid[0], bssid[1], bssid[2], bssid[3],
				 bssid[4], bssid[5]);
		wifi_connect(ssid, pass, bssid);
	}

	free(ssid);
	free(pass);
	return ESP_OK;
}

esp_err_t wifi_connect(char *ssid, char *pass, uint8_t *bssid) {
	s_wifi_event_group = xEventGroupCreate();

	tcpip_adapter_init();
	esp_event_loop_init(event_handler, NULL);

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	esp_wifi_init(&cfg);

	wifi_config_t wifi_config = {.sta = {.ssid = {0}, .password = {0}}};

	strncpy((char *)wifi_config.sta.ssid, ssid, 32);
	strncpy((char *)wifi_config.sta.password, pass, 64);

	if (bssid != NULL) {
		ESP_LOGI(TAG, "Attempting to connect using BSSID...");
		wifi_config.sta.bssid_set = true;
		memcpy(wifi_config.sta.bssid, bssid, 6);
	}

	esp_wifi_set_storage(WIFI_STORAGE_RAM);
	esp_wifi_set_mode(WIFI_MODE_STA);
	esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
	esp_err_t ret = esp_wifi_start();

	if (ret != ESP_OK) {
		return ESP_FAIL;
	}

	ESP_LOGI(TAG, "WIFI init finished.");
	ESP_LOGI(TAG, "Connecting to AP SSID: \"%s\" password:\"%s\"", wifi_config.sta.ssid,
			 wifi_config.sta.password);
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

	esp_wifi_set_storage(WIFI_STORAGE_RAM);
	esp_wifi_set_mode(WIFI_MODE_AP);
	esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config);
	ret = esp_wifi_start();
	if (ret != ESP_OK) {
		ESP_LOGE(TAG, "Failed to start AP wifi.");
		return ESP_FAIL;
	}

	return ESP_OK;
}

esp_err_t wifi_scan(char ***network_list) {
	tcpip_adapter_init();

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	esp_wifi_init(&cfg);
	esp_wifi_set_storage(WIFI_STORAGE_RAM);
	esp_wifi_set_mode(WIFI_MODE_STA);
	esp_wifi_start();

	wifi_scan_config_t scan_config = {.ssid = NULL,
									  .bssid = NULL,
									  .channel = 0,
									  .show_hidden = false,
									  .scan_type = WIFI_SCAN_TYPE_ACTIVE,
									  .scan_time = {.active = {.min = 100, .max = 250}}};

	esp_err_t ret = esp_wifi_scan_start(&scan_config, true);
	if (ret != ESP_OK) {
		ESP_LOGE(TAG, "Cannot start scan: %s", esp_err_to_name(ret));
		return ESP_FAIL;
	}
	esp_wifi_scan_stop();

	uint16_t num_ap = 0;
	ret = esp_wifi_scan_get_ap_num(&num_ap);
	if (ret != ESP_OK) {
		ESP_LOGE(TAG, "Cannot get AP number.");
		return ESP_FAIL;
	}

	ESP_LOGI(TAG, "There are %d APs found.", num_ap);

	wifi_ap_record_t *wifi_aps = malloc(num_ap * sizeof(*wifi_aps));
	if (wifi_aps == NULL) {
		return ESP_FAIL;
	}

	ret = esp_wifi_scan_get_ap_records(&num_ap, wifi_aps);

	char **name_list = malloc((num_ap + 1) * sizeof(*name_list));
	*network_list = name_list;
	if (name_list == NULL) {
		return ESP_FAIL;
	}

	int i;
	for (i = 0; i < num_ap; i++) {
		wifi_ap_record_t curr_ap = wifi_aps[i];
		ESP_LOGI(TAG, "Found AP: %s", curr_ap.ssid);
		name_list[i] = malloc(33 * sizeof(**name_list));
		if (name_list[i] == NULL) {
			return ESP_FAIL;
		}

		memset(name_list[i], 0, 33);
		strncpy(name_list[i], (char *)curr_ap.ssid, 32);
	}
	name_list[i] = NULL;

	return ESP_OK;
}

void free_scan(void *data) {
	char **network_list = (char **)data;
	int i;
	for (i = 0; network_list[i] != NULL; i++) {
		free(network_list[i]);
	}
	free(network_list);
}
