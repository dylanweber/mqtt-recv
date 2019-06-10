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

esp_err_t start_mqtt(char *mqtt_broker, uint16_t port, esp_mqtt_client_handle_t **ret_client) {
	mqtt_retry_num = -1;

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

	ESP_LOGI(TAG, "Cert:\n%s", buffer);
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
	*ret_client = malloc(sizeof(**ret_client));
	**ret_client = client;
	mqtt_retry_num = 0;
	return ESP_OK;
}

esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event) {
	esp_mqtt_client_handle_t client = event->client;
	switch (event->event_id) {
		case MQTT_EVENT_CONNECTED:
			ESP_LOGI(TAG, "Connected to MQTT broker.");
			esp_mqtt_client_subscribe(client, "/topic/test", 2);
			break;
		case MQTT_EVENT_DISCONNECTED:
			mqtt_retry_num++;
			if (mqtt_retry_num > CONFIG_ESP_MAXIMUM_RETRY) {
				ESP_LOGI(TAG, "Restarting WiFi configuration for MQTT broker...");
				wifi_disconnect();
				vTaskDelay(1000 / portTICK_PERIOD_MS);
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
			break;
		default:
			ESP_LOGI(TAG, "Other event: %d", event->event_id);
			break;
	}
	return ESP_OK;
}
