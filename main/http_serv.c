/*	Dylan Weber
 *	Doorbell Reciever
 * 	5/21/2019
 */
#include "http_serv.h"

static const char *TAG = "http";

httpd_handle_t start_httpserver() {
	httpd_handle_t server = NULL;
	httpd_config_t server_config = HTTPD_DEFAULT_CONFIG();

	if (httpd_start(&server, &server_config) == ESP_OK) {
		httpd_register_uri_handler(server, &index_uri);
		httpd_register_uri_handler(server, &post_uri);
	}

	return server;
}

static esp_err_t index_get_handler(httpd_req_t *req) {
	char *buffer;
	size_t buffer_len;

	httpd_resp_set_hdr(req, "Cache-Control", "no-cache");
	httpd_resp_send(req, "Hello, world!\n", strlen("Hello, world!\n"));
	return ESP_OK;
}

static esp_err_t submit_post_handler(httpd_req_t *req) {
	char *buffer;
	size_t buffer_len;

	httpd_resp_set_hdr(req, "Cache-Control", "no-cache");
	httpd_resp_send(req, "Recieved!\n", strlen("Recieved!\n"));
	return ESP_OK;
}
