#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
//#include <freertos/FreeRTOS.h>
//#include <freertos/task.h>
//#include <freertos/event_groups.h>
//#include <freertos/semphr.h>
//#include <freertos/queue.h>
//#include <esp_system.h>
//#include <esp_wifi.h>
//#include <esp_event.h>
//#include <esp_log.h>
//#include <esp_err.h>
//#include <esp_ota_ops.h>
//#include <esp_http_client.h>
//#include <esp_https_ota.h>
//#include "esp_console.h"
//#include "argtable3/argtable3.h"
//
//#include <nvs_flash.h>
//#include <lwip/err.h>
//#include <lwip/sockets.h>
//#include <lwip/sys.h>
//#include <lwip/netdb.h>
//#include <lwip/dns.h>
//#include <lwip/dns.h>
//#include <esp_wifi.h>
//#include <time.h>
//#include <mbedtls/aes.h>
//#include <cJSON.h>

#include "app.h"

#define DEFAULT_HOST "116.122.252.139"
#define MODEL_TYPE_ARECAAIR 4

#define MAX_HTTP_OUTPUT_BUFFER 2048

static struct
{
	struct arg_str *host;
	struct arg_int *type;
	struct arg_end *end;
}factory_args;

char local_response_buffer[MAX_HTTP_OUTPUT_BUFFER] = {0};

//uint16_t _http_event_handler(esp_http_client_event_t *evt)
//{
//	static char *output_buffer;  // Buffer to store response of http request from event handler
//	static int output_len;       // Stores number of bytes read
//	switch(evt->event_id) {
//		case HTTP_EVENT_ERROR:
//			printf("HTTP_EVENT_ERROR\n");
//			break;
//		case HTTP_EVENT_ON_CONNECTED:
//			printf("HTTP_EVENT_ON_CONNECTED\n");
//			break;
//		case HTTP_EVENT_HEADER_SENT:
//			printf("HTTP_EVENT_HEADER_SENT\n");
//			break;
//		case HTTP_EVENT_ON_HEADER:
//			printf("HTTP_EVENT_ON_HEADER, key=%s, value=%s\n", evt->header_key, evt->header_value);
//			break;
//		case HTTP_EVENT_ON_DATA:
//			printf("HTTP_EVENT_ON_DATA, len=%d\n", evt->data_len);
//			if (esp_http_client_is_chunked_response(evt->client)) {
//				// If user_data buffer is configured, copy the response into the buffer
//				if (evt->user_data) {
//					memcpy(evt->user_data + output_len, evt->data, evt->data_len);
//				} else {
//					if (output_buffer == NULL) {
//						output_buffer = (char *) malloc(esp_http_client_get_content_length(evt->client));
//						output_len = 0;
//						if (output_buffer == NULL) {
//							printf("Failed to allocate memory for output buffer\n");
//							return ESP_FAIL;
//						}
//					}
//					memcpy(output_buffer + output_len, evt->data, evt->data_len);
//				}
//				output_len += evt->data_len;
//			}
//
//			break;
//		case HTTP_EVENT_ON_FINISH:
//			printf("HTTP_EVENT_ON_FINISH\n");
//			if (output_buffer != NULL) {
//				free(output_buffer);
//				output_buffer = NULL;
//			}
//			output_len = 0;
//			break;
//		case HTTP_EVENT_DISCONNECTED:
//			printf("HTTP_EVENT_DISCONNECTED\n");
//			if (output_buffer != NULL) {
//				free(output_buffer);
//				output_buffer = NULL;
//			}
//			output_len = 0;
//			break;
//	}
//	return ESP_OK;
//}

//int download_serial(const char *host, const int type)
//{
//	char mac[6], mac_str[20];
//	cJSON *pItem = NULL, *pResp = NULL;
//	char *pPrintedData = NULL, *serial = NULL;
//	esp_err_t err;
//	int ret = -1;
//
//	PersistDataInfoT	*pPersistDataInfo = NULL;
//	GetPersistDataInfo(&pPersistDataInfo);
//
//	esp_http_client_config_t config = {
//		.host = DEFAULT_HOST,
//		.port = 1080,
//		.path = "/HfauFactory/api/gateway/monitor",
//		.event_handler = _http_event_handler,
//		.user_data = local_response_buffer,        // Pass address of local buffer to get response
//		.disable_auto_redirect = true,
//	};
//
//	if(host)
//	{
//		config.host = host;
//	}
//
//	esp_http_client_handle_t client = esp_http_client_init(&config);
//
//	esp_base_mac_addr_get((unsigned char*)mac);
//	if (esp_base_mac_addr_get((unsigned char*)mac) != ESP_OK) {
//		esp_efuse_mac_get_default((unsigned char*)mac);
//	}
//
//	snprintf(mac_str, 20, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
//
//	pItem = cJSON_CreateObject();
//	cJSON_AddStringToObject(pItem,"macAdd", mac_str);
//	if(type > 0)
//	{
//		cJSON_AddNumberToObject(pItem,"type", type);
//	}
//	else
//	{
//		cJSON_AddNumberToObject(pItem,"type", MODEL_TYPE_ARECAAIR);
//	}
//	pPrintedData = cJSON_Print(pItem);
//
//	esp_http_client_set_method(client, HTTP_METHOD_POST);
//	esp_http_client_set_header(client, "Content-Type", "application/json");
//	esp_http_client_set_post_field(client, pPrintedData, strlen(pPrintedData));
//	err = esp_http_client_perform(client);
//	if (err == ESP_OK) {
//		printf("HTTP POST Status = %d, content_length = %d\n",
//				esp_http_client_get_status_code(client),
//				esp_http_client_get_content_length(client));
//		pResp = cJSON_Parse(local_response_buffer);
//		printf("resp : %s\n", local_response_buffer);
//		if(pResp)
//		{
//			if(cJSON_HasObjectItem(pResp, "serial"))
//			{
//				serial = cJSON_GetObjectItem(pResp, "serial")->valuestring;
//				SetValueStr(PROP_NAME_SYS_SERIAL, serial);
//				printf("serial : %s -> %s\n", pPersistDataInfo->Serial, serial);
//				strncpy(pPersistDataInfo->Serial, serial, MAX_SERIAL_STR_LEN);
//				ret = 0;
//			}
//		}
//	} else {
//		printf("HTTP POST request failed: %s\n", esp_err_to_name(err));
//	}
//
//	cJSON_Delete(pItem);
//	if(pResp) cJSON_Delete(pResp);
//	esp_http_client_cleanup(client);
//
//	return ret;
//}

void FactoryTask()
{
	PersistDataInfoT *pPersistDataInfo = NULL;
	WifiInfoT *pWifiInfo = NULL;
	int ret;

	GetPersistDataInfo(&pPersistDataInfo);
	GetWifiInfo(&pWifiInfo);

//	while(1)
//	{
//		if(pWifiInfo->Status == 0)
//		{
////			sleep(3);
//			continue;
//		}

		if(strlen(pPersistDataInfo->Serial) == 0)
		{
//			ret = download_serial(NULL, -1);
			if(ret == 0)
			{
				printf("Serial download succeed\n");
				SetValueU8(PROP_NAME_SYS_FACTORY_DONE, 1);
			}
		}
//		else
//		{
////			sleep(30);
//			continue;
//		}

//		sleep(30);
//	}
}

static int CommandFactory(int argc, char **argv)
{
	int result = 0;

	int nerrors = arg_parse(argc, argv, (void **) &factory_args);
	if (nerrors != 0)
	{
		arg_print_errors(stderr, factory_args.end, argv[0]);
		return 1;
	}

	if(argc == 1)
	{
//		result = download_serial(NULL, -1);
	}
	else if(argc == 2)
	{
//		result = download_serial(factory_args.host->sval[0], -1);
	}
	else if(argc == 3)
	{
//		result = download_serial(factory_args.host->sval[0], factory_args.type->ival[0]);
	}
	else
	{
		arg_print_errors(stderr, factory_args.end, argv[0]);
	}

	return result;
}

void Register_CommandFactory(void)
{
//	factory_args.host = arg_str0(NULL, NULL, "<str>", "host to connect, test server : 121.170.188.27");
//	factory_args.type = arg_int0(NULL, NULL, "TYPE", "device type, ARECAAIR : 4");
//	factory_args.end = arg_end(2);
//
//	const esp_console_cmd_t cmd_factory =
//	{
//		.command = "factory",
//		.help = "factory <host> <type>",
//		.hint = NULL,
//		.func = &CommandFactory,
//		.argtable = &factory_args
//	};
//
//	ESP_ERROR_CHECK( esp_console_cmd_register(&cmd_factory) );
}

//void InitFactory(void)
//{
//	xTaskCreatePinnedToCore(FactoryTask, "Factory Task", 4096, NULL, 5, NULL, 0);
//}

