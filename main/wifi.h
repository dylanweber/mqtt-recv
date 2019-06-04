/*
	MQTT Reciever - wifi.h
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
#ifndef WIFI_H
#define WIFI_H

#include "esp_err.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "http_serv.h"
#include "lwip/err.h"
#include "lwip/sys.h"

#include <stdio.h>
#include <string.h>

/** Restores saved connections, returns ESP_OK if successfully finished or ESP_FAIL otherwise.
 */
esp_err_t wifi_restore();

/** Connects to the given wireless network, returns ESP_OK if successful finished or ESP_FAIL
 * otherwise.
 *
 *  Params:
 *  SSID - 32 character buffer
 *  Password - 64 character buffer
 *  BSSID - 6 byte buffer to address of AP (optional)
 */
esp_err_t wifi_connect(char *, char *, uint8_t *);

/** Disconnects from current wireless network, clears retry number.
 */
esp_err_t wifi_disconnect();

/// Starts access point, clears retry number.
esp_err_t wifi_startap();

/** Scans for nearby access points, returns ESP_OK if successful or ESP_FAIL if not.
 *
 * Params:
 * Network list - pointer of a char** to store the network list
 */
esp_err_t wifi_scan(char ***);

/// Frees the network list returned from wifi_scan(char ***).
void free_scan(void *);

/// Number of connection attempts to a wifi network, -1 if not attempting or broadcasting AP.
int s_retry_num;

#endif  // WIFI_H
