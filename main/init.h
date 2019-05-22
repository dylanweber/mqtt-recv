/*	Dylan Weber
 *	Doorbell Reciever
 * 	5/21/2019
 */
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "nvs_flash.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

esp_err_t app_init();
