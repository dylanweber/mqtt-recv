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
	struct interrupt_info *button_int_info = (struct interrupt_info *)params;
	while (true) {
		if (xQueueReceive(gpio_event_queue, &button_int_info, portMAX_DELAY)) {
			if (button_int_info->button == BUTTON_NUM && wifi_retry_num >= 0) {
				esp_mqtt_client_stop(button_int_info->mqtt_handle);
				ESP_LOGI(TAG, "Button pressed.");
				remove("/spiffs/wifi.ssid");
				remove("/spiffs/wifi.pass");
				remove("/spiffs/wifi.bssid");
				wifi_disconnect();
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
		} else {
			ESP_LOGI(TAG, "No interrupt.");
		}
	}
}
