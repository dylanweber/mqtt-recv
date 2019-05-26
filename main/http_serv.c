/*
	MQTT Reciever - http_serv.c
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
#include "http_serv.h"

#define min(a, b) ((a) < (b) ? (a) : (b))

static const char *TAG = "http";

static TaskHandle_t restart_task = NULL;

httpd_handle_t start_httpserver(char **network_list) {
	httpd_handle_t server = NULL;
	httpd_config_t server_config = HTTPD_DEFAULT_CONFIG();

	server_config.global_user_ctx = (void *)network_list;
	server_config.global_user_ctx_free_fn = free_scan;

	if (httpd_start(&server, &server_config) == ESP_OK) {
		httpd_register_uri_handler(server, &index_uri);
		httpd_register_uri_handler(server, &post_uri);
		httpd_register_uri_handler(server, &favicon_uri);
	}

	return server;
}

void stop_httpserver(httpd_handle_t server) {
	if (restart_task != NULL) {
		vTaskDelete(restart_task);
	}

	httpd_stop(server);
}

esp_err_t index_get_handler(httpd_req_t *req) {
	char *buffer;
	size_t buffer_len;

	httpd_resp_set_hdr(req, "Cache-Control", "no-cache");

	FILE *fp = fopen("/spiffs/index.html", "r");
	if (fp == NULL) {
		ESP_LOGE(TAG, "Failed to open index file.");
		return ESP_FAIL;
	}

	fseek(fp, 0, SEEK_END);
	buffer_len = ftell(fp);
	buffer = malloc((buffer_len + 1) * sizeof(*buffer));
	buffer[buffer_len] = '\0';

	fseek(fp, 0, SEEK_SET);
	fread(buffer, buffer_len, 1, fp);
	fclose(fp);

	// use template to generate wifi scan autocomplete

	char *template_str = strstr(buffer, "{%TEMPLATE}");
	char **network_list = httpd_get_global_user_ctx(req->handle);

	if (template_str == NULL || network_list == NULL) {
		httpd_resp_send(req, buffer, buffer_len);
		free(buffer);
	} else {
		httpd_resp_send_chunk(req, buffer, (template_str - buffer));
		char option_buffer[50] = {0};
		int i;
		for (i = 0; network_list[i] != NULL; i++) {
			strcpy(option_buffer, "<option>");
			strcpy(option_buffer + strlen("<option>"), network_list[i]);
			strcpy(option_buffer + strlen("<option>") + strlen(network_list[i]), "</option>");
			httpd_resp_send_chunk(req, option_buffer, strlen(option_buffer));
		}
		httpd_resp_send_chunk(req, template_str + strlen("{%TEMPLATE}"),
							  strlen(template_str) - strlen("{%TEMPLATE}"));
		httpd_resp_send_chunk(req, NULL, 0);
	}
	return ESP_OK;
}

esp_err_t submit_post_handler(httpd_req_t *req) {
	char *buffer;
	size_t buffer_len;

	// Decode POST data

	char content[500];
	memset(content, 0, sizeof(content));

	size_t recv_size = min(req->content_len, sizeof(content));
	int ret = httpd_req_recv(req, content, recv_size);
	if (ret <= 0) {
		if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
			httpd_resp_send_408(req);
		}
		return ESP_FAIL;
	}

	char ssid[97];
	memset(ssid, 0, sizeof(ssid));
	esp_err_t err_ret = httpd_query_key_value(content, "ssid", ssid, 97);
	if (err_ret != ESP_OK) {
		httpd_resp_send_500(req);
		return ESP_FAIL;
	}

	char pass[211];
	memset(pass, 0, sizeof(pass));
	err_ret = httpd_query_key_value(content, "pass", pass, 211);
	if (err_ret != ESP_OK) {
		httpd_resp_send_500(req);
		return ESP_FAIL;
	}

	char *decoded_ssid = urldecode(ssid);
	char *decoded_pass = urldecode(pass);

	ESP_LOGI(TAG, "SSID: %s", decoded_ssid);
	ESP_LOGI(TAG, "Password: %s", decoded_pass);

	// Save Wi-Fi information

	FILE *save_fp = fopen("/spiffs/wifi.ssid", "w");
	if (save_fp == NULL) {
		ESP_LOGE(TAG, "Failed to open wifi ssid file.");
		return ESP_FAIL;
	}

	fwrite(decoded_ssid, strlen(decoded_ssid), 1, save_fp);
	fclose(save_fp);

	save_fp = fopen("/spiffs/wifi.pass", "w");
	if (save_fp == NULL) {
		ESP_LOGE(TAG, "Failed to open wifi pass file.");
		return ESP_FAIL;
	}

	fwrite(decoded_pass, strlen(decoded_pass), 1, save_fp);
	fclose(save_fp);

	free(decoded_ssid);
	free(decoded_pass);

	httpd_resp_set_hdr(req, "Cache-Control", "no-cache");

	FILE *fp = fopen("/spiffs/saved.html", "r");
	if (fp == NULL) {
		ESP_LOGE(TAG, "Failed to open post file.");
		return ESP_FAIL;
	}

	fseek(fp, 0, SEEK_END);
	buffer_len = ftell(fp);
	buffer = malloc((buffer_len + 1) * sizeof(*buffer));
	buffer[buffer_len] = '\0';

	fseek(fp, 0, SEEK_SET);
	fread(buffer, buffer_len, 1, fp);
	fclose(fp);

	httpd_resp_send(req, buffer, buffer_len);
	free(buffer);

	xTaskCreate(delayed_restart, "restart", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY,
				&restart_task);
	return ESP_OK;
}

esp_err_t favicon_get_handler(httpd_req_t *req) {
	char *buffer;
	size_t buffer_len;

	httpd_resp_set_hdr(req, "Content-Type", "image/x-icon");

	FILE *fp = fopen("/spiffs/favicon.ico", "r");
	if (fp == NULL) {
		ESP_LOGE(TAG, "Failed to open favicon file.");
		return ESP_FAIL;
	}

	fseek(fp, 0, SEEK_END);
	buffer_len = ftell(fp);
	buffer = malloc((buffer_len + 1) * sizeof(*buffer));
	buffer[buffer_len] = '\0';

	fseek(fp, 0, SEEK_SET);
	fread(buffer, buffer_len, 1, fp);
	fclose(fp);

	httpd_resp_send(req, buffer, buffer_len);
	free(buffer);
	return ESP_OK;
}

void delayed_restart(void *params) {
	vTaskDelay(2500 / portTICK_PERIOD_MS);
	esp_wifi_stop();
	esp_wifi_deinit();
	vTaskDelay(100 / portTICK_PERIOD_MS);
	esp_restart();
}

char *urldecode(char *encoded_string) {
	int i;
	int n = 0;
	int val;
	int encoded_string_len = strlen(encoded_string);
	int string_len = 0;

	for (i = 0; i < encoded_string_len; i++) {
		if (encoded_string[i] == '%') {
			string_len -= 1;
		} else {
			string_len++;
		}
	}

	char *return_string = malloc(string_len + 1);
	char *hex_value = malloc(3);

	for (i = 0; encoded_string[i] != '\0'; i++, n++) {
		if (encoded_string[i] == '%') {
			cleancpy(hex_value, encoded_string + i + 1, 2);
			val = strtol(hex_value, NULL, 16);
			if (val != 0) {
				return_string[n] = val;
			}
			i += 2;
		} else if (encoded_string[i] == '+') {
			return_string[n] = ' ';
		} else {
			return_string[n] = encoded_string[i];
		}
	}

	return_string[string_len] = '\0';
	return return_string;
}

void cleancpy(char *dest, char *src, int len) {
	if (len > 0)
		strncpy(dest, src, len);
	dest[len] = '\0';
}
