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
#include "http_serv.h"
#include "init.h"
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
	esp_err_t conn_ret = wifi_restore();
	if (conn_ret != ESP_OK) {
		esp_err_t ret = wifi_startap();
		if (ret == ESP_OK)
			start_httpserver();
	} else {
		// start_mqtt();
	}
}
