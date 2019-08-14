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

	mqtt_client = NULL;
	configure_clear_interrupt(&mqtt_client);
	configure_ext_interrupt(&mqtt_client);

	esp_err_t conn_ret = wifi_restore();
	if (conn_ret != ESP_OK) {
		setup_routine();
		return;
	} else {
		mqtt_routine(&mqtt_client);
	}
	ESP_LOGI(TAG, "Finished startup.");

	// Task to send MQTT message must run on main task.
	while (true) {
		if (xSemaphoreTake(mqtt_semaphore, portMAX_DELAY) == pdTRUE) {
			ESP_LOGI(TAG, "Sending MQTT message...");
			char number[10];
			sprintf(number, "%u", mqtt_rolling_code);
			esp_mqtt_client_publish(mqtt_client, "/doorbell/roll", number, 0, 2, true);
			vTaskDelay(400 / portTICK_PERIOD_MS);
			xSemaphoreTake(mqtt_semaphore, 0);
			vTaskDelay(4600 / portTICK_PERIOD_MS);
		}
	}
}
