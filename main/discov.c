/*
	MQTT Reciever - discov.c
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

static const char *TAG = "discov";

esp_err_t init_mdns() {
	esp_err_t ret = mdns_init();
	if (ret != ESP_OK) {
		ESP_LOGE(TAG, "Failure initializing mDNS");
		return ESP_FAIL;
	}

	mdns_hostname_set(CONFIG_MDNS_NAME);
	mdns_instance_name_set(CONFIG_MDNS_INSTANCE);
	return ESP_OK;
}
