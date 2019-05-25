/*
	MQTT Reciever - init.c
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
#include "init.h"

static const char *TAG = "init";

esp_err_t app_init() {
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	esp_vfs_spiffs_conf_t config = {.base_path = "/spiffs",
									.partition_label = "storage",
									.max_files = 5,
									.format_if_mount_failed = true};

	ret = esp_vfs_spiffs_register(&config);

	if (ret != ESP_OK) {
		if (ret == ESP_FAIL) {
			ESP_LOGE(TAG, "Failed to mount or format filesystem.");
		} else if (ret == ESP_ERR_NOT_FOUND) {
			ESP_LOGE(TAG, "Failed to find SPIFFS patition.");
		} else {
			ESP_LOGE(TAG, "Other SPIFFS error: %s", esp_err_to_name(ret));
		}
		return ESP_FAIL;
	}

	FILE *fp = fopen("/spiffs/example.txt", "r");
	if (fp == NULL) {
		ESP_LOGE(TAG, "Failed to open example file.");
		return ESP_FAIL;
	}

	char buf[64];
	memset(buf, 0, sizeof(buf));
	fread(buf, 1, sizeof(buf) - 1, fp);
	fclose(fp);

	ESP_LOGI(TAG, "Read from example.txt: %s", buf);

	return ESP_OK;
}
