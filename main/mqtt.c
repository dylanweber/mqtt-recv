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

esp_err_t start_mqtt(char *mqtt_broker, uint16_t port) {
	esp_mqtt_client_config_t mqtt_config = {.uri = NULL,
											.event_handle = mqtt_event_handler,
											.port = port,
											.cert_pem = NULL,
											.refresh_connection_after_ms =
												500};  // Change to event loop handle?

	size_t ip_len = strlen("mqtts://.lan") + strlen(mqtt_broker) + 1;
	char *final_uri = malloc(ip_len * sizeof(*final_uri));
	sprintf(final_uri, "mqtts://%s.lan", mqtt_broker);
	mqtt_config.uri = final_uri;

	char *buffer;
	size_t buffer_len;

	FILE *fp = fopen("/spiffs/comb.pem", "r");
	if (fp == NULL) {
		ESP_LOGE(TAG, "Failed to open certificate file.");
		return ESP_FAIL;
	}

	fseek(fp, 0, SEEK_END);
	buffer_len = ftell(fp);
	buffer = malloc((buffer_len + 1) * sizeof(*buffer));
	buffer[buffer_len] = '\0';

	fseek(fp, 0, SEEK_SET);
	fread(buffer, buffer_len, 1, fp);
	fclose(fp);

	mqtt_config.cert_pem = buffer;

	ESP_LOGI(TAG, "Connecting to %s...", final_uri);
	esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_config);
	esp_mqtt_client_start(client);

	// free(buffer); // Certificate required in memory.
	free(final_uri);
	return ESP_OK;
}

esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event) {
	return ESP_OK;
}
