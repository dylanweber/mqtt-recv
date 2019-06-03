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

esp_err_t wifi_restore();
esp_err_t wifi_connect(char *, char *, uint8_t *);
esp_err_t wifi_disconnect();
esp_err_t wifi_startap();
esp_err_t wifi_scan(char ***);
void free_scan(void *);

int s_retry_num;

#endif  // WIFI_H
