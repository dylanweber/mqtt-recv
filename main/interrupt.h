/*
	MQTT Reciever - interrupt.h
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
#ifndef INTERRUPT_H
#define INTERRUPT_H

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "init.h"
#include "mqtt_client.h"
#include "wifi.h"

/// GPIO event handler that stores information in the GPIO event queue
void IRAM_ATTR gpio_isr_handler(void *);
/// GPIO event task that waits for queue events and handles them
void gpio_event_task(void *);

#endif  // INTERRUPT_H
