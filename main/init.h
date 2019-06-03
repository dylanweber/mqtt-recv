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
#define ALLOC_FLAGS 0

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
#include "nvs_flash.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

/// Initializes vital ESP32 functions
esp_err_t app_init();
/// Configures the clear button interrupt on GPIO4
void configure_clear_interrupt();

/// GPIO event queue used for storing GPIO interrupt information
QueueHandle_t gpio_event_queue;

#endif  // INIT_H
