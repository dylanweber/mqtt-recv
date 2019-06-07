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
#include "discov.h"
#include "http_serv.h"
#include "init.h"
#include "mqtt.h"
#include "wifi.h"

#include <stdio.h>
#include <string.h>

static const char *TAG = "main";

void app_main() {
	esp_err_t ret = app_init();

	if (ret != ESP_OK) {
		ESP_LOGE(TAG, "Failure initializing.");
		return;
	}

	ESP_LOGI(TAG, "Initialized.");
	esp_mqtt_client_handle_t mqtt_client = NULL;
	esp_err_t conn_ret = wifi_restore();
	if (conn_ret != ESP_OK) {
		char **network_list;
		esp_err_t ret = wifi_scan(&network_list);
		if (ret != ESP_OK) {
			ESP_LOGE(TAG, "Failed to scan Wi-Fi.");
			return;
		}
		ret = wifi_startap();
		if (ret == ESP_OK) {
			start_httpserver(network_list);
		}
	} else {
		char mqtt_hostname[255] = {'\0'};
		uint16_t port = 0;
		init_mdns(mqtt_hostname, &port);
		if (mqtt_hostname != NULL && port != 0) {
			vTaskDelay(2000 / portTICK_PERIOD_MS);
			start_mqtt(mqtt_hostname, port, &mqtt_client);
		} else {
			ESP_LOGI(TAG, "Failed discovery.");
		}
	}
	configure_clear_interrupt(mqtt_client);
	ESP_LOGI(TAG, "Finished startup.");
}
