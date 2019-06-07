/*
	MQTT Reciever - mqtt.h
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
#ifndef MQTT_H
#define MQTT_H

#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "mqtt_client.h"

/** Starts the MQTTS client.
 *
 * Params:
 * Broker - The mDNS hostname of the broker as a string.
 * Port - The port of the MQTT broker.
 */
esp_err_t start_mqtt(char *, uint16_t);

/// MQTTS client event handler function.
esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t);

#endif  // MQTT_H
