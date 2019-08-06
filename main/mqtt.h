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
#include "wifi.h"

/** Starts the MQTTS client.
 *
 * Params:
 * Broker - The mDNS hostname of the broker as a string.
 * Port - The port of the MQTT broker.
 * Client - A pointer to return the client handle.
 */
esp_err_t start_mqtt(char *, uint16_t, esp_mqtt_client_handle_t *);

/// MQTTS client event handler function.
esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t);
/// Checks if rolling code exists on the broker.
void mqtt_rolling_timeout(void *);
/// Sends MQTT messages based on semaphore.
void mqtt_semaphore_check(void *);

/// Number of connection attempts to the MQTT broker, -1 if not attempting to connect.
int mqtt_retry_num;

/// Whether device connected or not to MQTT broker.
bool mqtt_connected;

/// Whether device connected once to MQTT broker.
bool mqtt_connect_once;

/// Rolling code used for confirming doorbell rings.
unsigned int mqtt_rolling_code;

/// Semaphore for sending MQTT messages.
SemaphoreHandle_t mqtt_semaphore;

#endif  // MQTT_H
