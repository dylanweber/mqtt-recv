/*
	MQTT Reciever - mqtt.c
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
#include "mqtt.h"

static const char *TAG = "mqtt";

esp_err_t start_mqtt(char *mqtt_broker, uint16_t port, esp_mqtt_client_handle_t *ret_client) {
	mqtt_retry_num = -1;
	mqtt_connected = false;
	mqtt_connect_once = false;
	mqtt_rolling_code = 0;

	esp_mqtt_client_config_t mqtt_config = {.uri = NULL,
											.event_handle = mqtt_event_handler,
											.port = port,
											.client_id = CONFIG_MDNS_NAME,
											.cert_pem = NULL,
											.refresh_connection_after_ms = 7200000};

	size_t ip_len = strlen("mqtts://.lan") + strlen(mqtt_broker) + 1;
	char *final_uri = malloc(ip_len * sizeof(*final_uri));
	if (final_uri == NULL) {
		ESP_LOGE(TAG, "Failed to allocate memory.");
		return ESP_FAIL;
	}
	sprintf(final_uri, "mqtts://%s.lan", mqtt_broker);
	mqtt_config.uri = final_uri;

	char *buffer;
	size_t buffer_len;

	FILE *fp = fopen("/spiffs/comb.pem", "r");
	if (fp == NULL) {
		ESP_LOGE(TAG, "Failed to open certificate file.");
		free(final_uri);
		return ESP_FAIL;
	}

	fseek(fp, 0, SEEK_END);
	buffer_len = ftell(fp);
	buffer = malloc((buffer_len + 1) * sizeof(*buffer));
	if (buffer == NULL) {
		ESP_LOGE(TAG, "Failed to allocate memory.");
		return ESP_FAIL;
	}
	buffer[buffer_len] = '\0';

	fseek(fp, 0, SEEK_SET);
	fread(buffer, buffer_len, 1, fp);
	fclose(fp);

	mqtt_config.cert_pem = buffer;

	// ESP_LOGI(TAG, "Cert:\n%s", buffer);
	ESP_LOGI(TAG, "Connecting to %s...", final_uri);
	esp_event_loop_create_default();
	esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_config);
	esp_err_t ret = esp_mqtt_client_start(client);
	if (ret != ESP_OK) {
		ESP_LOGI(TAG, "Failed to start MQTT client...");
		free(buffer);
		free(final_uri);
		return ESP_FAIL;
	}

	// free(buffer);  // Certificate required in memory.
	free(final_uri);
	ESP_LOGI(TAG, "useful number: %p", ret_client);
	if (ret_client != NULL) {
		*ret_client = client;
	} else {
		ESP_LOGI(TAG, "Successfully tested MQTT client... restarting...");
		esp_restart();
	}
	mqtt_retry_num = 0;
	return ESP_OK;
}

esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event) {
	esp_mqtt_client_handle_t client = event->client;
	switch (event->event_id) {
		case MQTT_EVENT_CONNECTED:
			mqtt_connected = true;
			mqtt_connect_once = true;
			ESP_LOGI(TAG, "Connected to MQTT broker.");
			esp_mqtt_client_subscribe(client, "/doorbell/roll", 2);
			ESP_LOGI(TAG, "new config: %d", wifi_new_config);
			if (wifi_new_config) {
				esp_mqtt_client_stop(client);
				wifi_disconnect();
				esp_restart();
			}
			xTaskCreate(mqtt_rolling_timeout, "rolling_timeout", 4096, &mqtt_client,
						tskIDLE_PRIORITY, NULL);
			break;
		case MQTT_EVENT_DISCONNECTED:
			mqtt_connected = false;
			mqtt_retry_num++;
			if ((mqtt_retry_num > CONFIG_ESP_MAXIMUM_RETRY && !mqtt_connect_once) ||
				(mqtt_retry_num > 5 * CONFIG_ESP_MAXIMUM_RETRY && mqtt_connect_once)) {
				ESP_LOGI(TAG, "Restarting WiFi configuration for MQTT broker...");
				wifi_disconnect();
				if (wifi_new_config) {
					remove("/spiffs/wifi.ssid");
					remove("/spiffs/wifi.pass");
					remove("/spiffs/wifi.bssid");
				}
				esp_restart();
				wifi_restore();
			} else if (mqtt_retry_num >= 0) {
				ESP_LOGI(TAG, "Reconnecting to MQTT broker...");
			}
			ESP_LOGI(TAG, "Disconnected from MQTT broker.");
			break;
		case MQTT_EVENT_DATA:
			ESP_LOGI(TAG, "MQTT_EVENT_DATA");
			printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
			printf("DATA=%.*s\r\n", event->data_len, event->data);
			char data_cpy[10];
			memset(data_cpy, 0, sizeof(data_cpy));
			strncpy(data_cpy, event->data, (event->data_len < 10) ? event->data_len : 10);
			if (strncmp(event->topic, "/doorbell/roll", event->topic_len) == 0) {
				unsigned int new_code = strtoul(data_cpy, NULL, 10);
				if (new_code <= 0) {
					mqtt_rolling_code = 1;
				} else if (new_code > mqtt_rolling_code) {
					mqtt_rolling_code = new_code;
				} else if (new_code < mqtt_rolling_code) {
					char number[10];
					sprintf(number, "%u", mqtt_rolling_code);
					esp_mqtt_client_publish(client, "/doorbell/roll", number, 0, 2, true);
				}
			}
			break;
		default:
			ESP_LOGI(TAG, "Other event: %d", event->event_id);
			break;
	}
	return ESP_OK;
}

void mqtt_rolling_timeout(void *params) {
	esp_mqtt_client_handle_t client = (esp_mqtt_client_handle_t)params;
	vTaskDelay(8000 / portTICK_PERIOD_MS);
	if (mqtt_connected && mqtt_rolling_code == 0) {
		ESP_LOGI(TAG, "Rolling code timout...");
		char number[10];
		sprintf(number, "%u", ++mqtt_rolling_code);
		esp_mqtt_client_publish(client, "/doorbell/roll", number, 0, 2, true);
	}
	vTaskDelete(NULL);
}
