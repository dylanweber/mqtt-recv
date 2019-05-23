/*	Dylan Weber
 *	Doorbell Reciever
 * 	5/21/2019
 */
#include "esp_err.h"
#include "esp_log.h"

#include <esp_http_server.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

httpd_handle_t start_httpserver();
static esp_err_t index_get_handler(httpd_req_t *);
static esp_err_t submit_post_handler(httpd_req_t *);
static esp_err_t favicon_get_handler(httpd_req_t *);

// clang-format off
static const httpd_uri_t index_uri = {
	.uri = "/",
	.method = HTTP_GET,
	.handler = index_get_handler,
	.user_ctx = NULL
};

static const httpd_uri_t post_uri = {
	.uri = "/submit",
	.method = HTTP_POST,
	.handler = submit_post_handler,
	.user_ctx = NULL
};

static const httpd_uri_t favicon_uri = {
	.uri = "/favicon.ico",
	.method = HTTP_GET,
	.handler = favicon_get_handler,
	.user_ctx = NULL
};
