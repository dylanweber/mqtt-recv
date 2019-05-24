/*
	MQTT Reciever - main.c
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
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "http_serv.h"
#include "init.h"
#include "lwip/err.h"
#include "lwip/sys.h"

#include <string.h>

static EventGroupHandle_t s_wifi_event_group;

const int WIFI_CONNECTED_BIT = 0x1;

static const char *TAG = "main";

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
			if (s_retry_num < CONFIG_ESP_MAXIMUM_RETRY) {
				esp_wifi_connect();
				xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
				s_retry_num++;
				ESP_LOGI(TAG, "Retrying connection");
			}
			ESP_LOGI(TAG, "Connection failed\n");
			break;
		}
		default:
			break;
	}
	return ESP_OK;
}

void wifi_init_sta() {
	s_wifi_event_group = xEventGroupCreate();

	tcpip_adapter_init();
	esp_event_loop_init(event_handler, NULL);

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	esp_wifi_init(&cfg);
	wifi_config_t wifi_config = {
		.sta = {.ssid = CONFIG_ESP_WIFI_SSID, .password = CONFIG_ESP_WIFI_PASSWORD},
	};

	esp_wifi_set_mode(WIFI_MODE_STA);
	esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
	esp_wifi_start();

	ESP_LOGI(TAG, "WIFI init finished.");
	ESP_LOGI(TAG, "Connected to AP SSID:%s password:%s", CONFIG_ESP_WIFI_SSID,
			 CONFIG_ESP_WIFI_PASSWORD);
}

void app_main() {
	esp_err_t ret = app_init();

	if (ret != ESP_OK) {
		ESP_LOGE(TAG, "Failure initializing.");
		return;
	}

	ESP_LOGI(TAG, "Initialized.");
	wifi_init_sta();
	start_httpserver();
}
