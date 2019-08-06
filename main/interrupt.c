/*
	MQTT Reciever - interrupt.c
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
#include "interrupt.h"

static const char *TAG = "int";

void IRAM_ATTR gpio_isr_handler(void *params) {
	struct interrupt_info *button_int_info = (struct interrupt_info *)params;
	xQueueSendFromISR(gpio_event_queue, &button_int_info, NULL);
}

void gpio_event_task(void *params) {
	struct interrupt_info *button_int_info;
	while (true) {
		if (xQueueReceive(gpio_event_queue, &button_int_info, portMAX_DELAY)) {
			if (button_int_info->button == BUTTON_NUM && wifi_retry_num >= 0) {
				// struct interrupt_info *button_temp;
				// if (xQueueReceive(gpio_event_queue, &button_temp, 3000 / portTICK_PERIOD_MS) &&
				// 	button_temp->button == BUTTON_NUM) {
				// 	ESP_LOGI(TAG, "Too soon.");
				// 	continue;
				// }
				// if (!xQueueReceive(gpio_event_queue, &button_temp, 3000 / portTICK_PERIOD_MS) ||
				// 	button_temp->button != BUTTON_NUM) {
				// 	ESP_LOGI(TAG, "Too late.");
				// 	continue;
				// }
				if (button_int_info->mqtt_handle != NULL) {
					ESP_LOGI(TAG, "useful number: %p", button_int_info->mqtt_handle);
					if (*button_int_info->mqtt_handle == NULL) {
						ESP_LOGI(TAG, "MQTT handle is NULL.");
					} else {
						esp_mqtt_client_stop(*button_int_info->mqtt_handle);
						mqtt_connected = false;
					}
				} else {
					ESP_LOGI(TAG, "Cannot stop MQTT server, reference is NULL...");
				}
				ESP_LOGI(TAG, "Button pressed.");
				remove("/spiffs/wifi.ssid");
				remove("/spiffs/wifi.pass");
				remove("/spiffs/wifi.bssid");
				wifi_disconnect();
				setup_routine();
			} else if (button_int_info->button == EXT_NUM) {
				if (*button_int_info->mqtt_handle != NULL && mqtt_rolling_code != 0) {
					// char number[10];
					// sprintf(number, "%u", ++mqtt_rolling_code);
					// esp_mqtt_client_publish(client, "/doorbell/roll", number, 0, 2, true);
					mqtt_rolling_code++;
					xSemaphoreGive(mqtt_semaphore);
					ESP_LOGI(TAG, "Publishing chime press: #%d", mqtt_rolling_code);
				}
				ESP_LOGI(TAG, "Doorbell chime.");
			}
		} else {
			ESP_LOGI(TAG, "No interrupt.");
		}
	}
}
