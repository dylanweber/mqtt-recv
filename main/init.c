/*	Dylan Weber
 *	Doorbell Reciever
 * 	5/21/2019
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
	fread(buf, 1, sizeof(buf), fp);
	fclose(fp);

	ESP_LOGI(TAG, "Read from example.txt: %s", buf);

	return ESP_OK;
}
