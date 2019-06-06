/*
	MQTT Reciever - discov.h
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
#ifndef DISCOV_H
#define DISCOV_H

#include "esp_log.h"
#include "esp_system.h"
#include "mdns.h"
#include "wifi.h"

/** Starts discovery mechanism via mDNS
 *
 * Params:
 * MQTT IP - A 255-byte buffer for the MQTT broker hostname.
 * MQTT Port - A pointer to a 16-bit integer for the MQTT broker port.
 */
esp_err_t init_mdns(char *, uint16_t *);

#endif  // DISCOV_H
