/*
	MQTT Reciever - init.h
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
#ifndef INIT_H
#define INIT_H

#define BUTTON_NUM GPIO_NUM_4
#define BUTTON_MSK (1ULL << BUTTON_NUM)

#define EXT_NUM GPIO_NUM_5
#define EXT_MSK (1ULL << EXT_NUM)

#define ALLOC_FLAGS 0

#define STATUS_NUM GPIO_NUM_22
#define STATUS_MSK (1ULL << STATUS_NUM)

#include "discov.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "interrupt.h"
#include "mqtt.h"
#include "mqtt_client.h"
#include "nvs_flash.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

struct interrupt_info {
	int button;
	esp_mqtt_client_handle_t *mqtt_handle;
};

/// Initializes vital ESP32 functions
esp_err_t app_init();
/// Configures the clear button interrupt on GPIO4
void configure_clear_interrupt();
/// Configures the external peripheral interrupt on GPIO5
void configure_ext_interrupt(esp_mqtt_client_handle_t *);
/// Turns on status LED.
void setup_status_led();
/// Starts MQTT client.
void mqtt_routine(esp_mqtt_client_handle_t *);
/// Starts setup server.
void setup_routine();
/// Saves the MQTT rolling code for restart.
void save_recovery();
/// Retrieves the MQTT rolling code previously saved.
void attempt_recovery();

/// GPIO event queue used for storing GPIO interrupt information
QueueHandle_t gpio_event_queue;

/// Handler for MQTT client.
esp_mqtt_client_handle_t mqtt_client;

#endif  // INIT_H
