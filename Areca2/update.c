#include <string.h>
//#include <freertos/FreeRTOS.h>
//#include <freertos/task.h>
//#include <freertos/event_groups.h>
//#include <esp_system.h>
//#include <esp_event.h>
//#include <esp_log.h>
//#include <esp_ota_ops.h>
//#include <esp_http_client.h>
//#include <esp_https_ota.h>
//#include <nvs.h>
//#include <nvs_flash.h>
//#include <argtable3/argtable3.h>
//#include <esp_console.h>
#include "app.h"

#define	HTTPS_SERVER_URL	"https://192.168.50.2/images/areca.bin"
#define DEFAULT_TIMEOUT		2000

esp_http_client_config_t	HttpConfig;
esp_https_ota_config_t		HttpsOtaConfig;	

static const char *TAG = "ota";

extern const uint8_t ca_start[]   asm("_binary_ca_crt_start");
extern const uint8_t ca_end[]   asm("_binary_ca_crt_end");

extern const uint8_t client_start[]   asm("_binary_client_crt_start");
extern const uint8_t client_end[]   asm("_binary_client_crt_end");

extern const uint8_t client_key_start[]   asm("_binary_client_key_start");
extern const uint8_t client_key_end[]   asm("_binary_client_key_end");

static esp_err_t ValidateImageHeader(esp_app_desc_t *new_app_info)
{
	if (new_app_info == NULL) {
		return ESP_ERR_INVALID_ARG;
	}

	const esp_partition_t *running = esp_ota_get_running_partition();
	esp_app_desc_t running_app_info;
	if (esp_ota_get_partition_description(running, &running_app_info) == ESP_OK) 
	{
		ESP_LOGI(TAG, "Running firmware version: %s", running_app_info.version);
	}

	/* check version info */
	if (memcmp(new_app_info->version, running_app_info.version, sizeof(new_app_info->version)) == 0) 
	{
		ESP_LOGW(TAG, "Current running version is the same as a new. We will not continue the update.");
		return ESP_FAIL;
	}

    return ESP_OK;
}

void OTATask(void *pArg)
{
	esp_err_t ota_finish_err = ESP_OK, err = ESP_OK, Ret = ESP_OK;
	esp_https_ota_handle_t https_ota_handle = NULL;
	esp_https_ota_config_t *pHttps_ota_config = NULL;
    esp_app_desc_t app_desc;

	WifiInfoT	*pWifiInfo = NULL;
	GetWifiInfo(&pWifiInfo);

	if((pArg == NULL) || (pWifiInfo == NULL))
	{
		vTaskDelete(NULL);
		return;
	}
	
	while(1)
	{
		if(pWifiInfo->Status == 1)
		{
			break;
		}
		
		sleep(3);
	}
	
	pHttps_ota_config = (esp_https_ota_config_t *)pArg;
	
	err = esp_https_ota_begin(pHttps_ota_config, &https_ota_handle);
	if (err != ESP_OK) 
	{
		goto ota_end;
	}

	err = esp_https_ota_get_img_desc(https_ota_handle, &app_desc);
	if (err != ESP_OK) 
	{
		goto ota_end;
	}

	err = ValidateImageHeader(&app_desc);
	if (err != ESP_OK) 
	{
		goto ota_end;
	}

	while (1) 
	{
		err = esp_https_ota_perform(https_ota_handle);
		if (err != ESP_ERR_HTTPS_OTA_IN_PROGRESS) 
		{
			break;
		}
	}

	if (esp_https_ota_is_complete_data_received(https_ota_handle) != true) 
	{
		// the OTA image was not completely received and user can customise the response to this situation.
		ESP_LOGE(TAG, "Complete data was not received.");
	}

ota_end:
	ota_finish_err = esp_https_ota_finish(https_ota_handle);
	if ((err == ESP_OK) && (ota_finish_err == ESP_OK)) 
	{
		Ret = SetValueU8(PROP_NAME_APP_UPGRADE, 0);
		printf("ESP_HTTPS_OTA upgrade successful. Rebooting ... \r\n");
		vTaskDelay(1000 / portTICK_PERIOD_MS);
		esp_restart();
	} 
	else 
	{
		if (ota_finish_err == ESP_ERR_OTA_VALIDATE_FAILED) 
		{
			ESP_LOGE(TAG, "Image validation failed, image is corrupted");
		}

		vTaskDelete(NULL);
	}

	return;
}

void InitHTTPSOTAServer(void) 
{

	memset(&HttpConfig,0,sizeof(HttpConfig));
	HttpConfig.url = HTTPS_SERVER_URL;
	HttpConfig.cert_pem = (char *)ca_start;
	HttpConfig.client_cert_pem = (char *)client_start;
	HttpConfig.client_key_pem = (char *)client_key_start;
	HttpConfig.timeout_ms = DEFAULT_TIMEOUT;
	HttpConfig.transport_type = HTTP_TRANSPORT_OVER_SSL;
    HttpConfig.skip_cert_common_name_check = true;
	
	memset(&HttpsOtaConfig,0,sizeof(HttpsOtaConfig));
	HttpsOtaConfig.http_config = &(HttpConfig);

	return;
}

int HTTPSUpdate(int argc, char **argv)
{
	InitHTTPSOTAServer();
	xTaskCreate(&OTATask, "OTATask", 1024 * 8, (void *)&HttpsOtaConfig, 5, NULL);
	return 0;
}

void Register_Update(void) 
{
    const esp_console_cmd_t cmd = {
        .command = "update",
        .help = "update",
        .hint = NULL,
        .func = &HTTPSUpdate,
		.argtable = NULL
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}

void HTTPSUpdateSW(void)
{
	InitHTTPSOTAServer();
	xTaskCreatePinnedToCore(&OTATask, "OTATask", 1024 * 8, (void *)&HttpsOtaConfig, 5, NULL, 0);
}


