#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

//#include "esp_wifi.h"
//#include "esp_system.h"
//#include "nvs_flash.h"
//#include "esp_event.h"
//
//#include "freertos/FreeRTOS.h"
//#include "freertos/task.h"
//#include "freertos/semphr.h"
//#include "freertos/queue.h"
//#include "freertos/event_groups.h"
//
//#include "lwip/sockets.h"
//#include "lwip/dns.h"
//#include "lwip/netdb.h"
//
//#include "esp_log.h"
//#include "mqtt_client.h"
//#include "cJSON.h"

#include "app.h"

#define	MQTT_URL	"192.168.50.2"
#define	MQTT_PORT	8883

//static const char *TAG = "MQTTS_SAMPLE";

esp_mqtt_client_config_t sMqttCfg;
MqttInfoT				*pMqttInf = NULL;
void MessageCallback(esp_mqtt_event_handle_t event);


static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    // your_context_t *context = event->context;
    switch (event->event_id) 
	{
        case MQTT_EVENT_CONNECTED:
			pMqttInf->Connected = 1;
//            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
			printf("### MQTT_EVENT_CONNECTED ++++ !!!\r\n");
			msg_id = esp_mqtt_client_subscribe(client, pMqttInf->Topic, 0);
//            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
			printf("### MQTT_EVENT_CONNECTED ----(%d)!!!\r\n", msg_id);

            break;

        case MQTT_EVENT_DISCONNECTED:
//            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
			pMqttInf->Connected = 0;
			printf("### MQTT_EVENT_DISCONNECTED ++++ !!!\r\n");
            break;

        case MQTT_EVENT_SUBSCRIBED:
//            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
			printf("### MQTT_EVENT_SUBSCRIBED pMqttInf->Connected (%d) ++++ !!!\r\n", pMqttInf->Connected);
			pMqttInf->Connected = 2;

//            msg_id = esp_mqtt_client_publish(client, pMqttInf->Topic, "data", 0, 0, 0);
//            ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
			printf("### MQTT_EVENT_SUBSCRIBED pMqttInf->Connected (%d)----!!!\r\n", pMqttInf->Connected);
            break;
			
        case MQTT_EVENT_UNSUBSCRIBED:
//            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
			printf("### MQTT_EVENT_UNSUBSCRIBED msg_id=%d ++++ !!!\r\n", event->msg_id);
            break;
		
        case MQTT_EVENT_PUBLISHED:
//            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
			printf("### MQTT_EVENT_PUBLISHED msg_id=%d ++++ !!!\r\n", event->msg_id);
            break;
		
        case MQTT_EVENT_DATA:
//            ESP_LOGI(TAG, "MQTT_EVENT_DATA");

			printf("### MQTT_EVENT_DATA msg_id=%d ++++ !!!\r\n", event->msg_id);
		
//            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
//            printf("DATA=%.*s\r\n", event->data_len, event->data);
			  printf("TOPIC len (%d) \r\n", event->topic_len);
			  printf("DATA len (%d) \r\n", event->data_len);
			/* message receive from server*/
			  MessageCallback(event);
            break;
			
        case MQTT_EVENT_ERROR:
//            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
			printf("### MQTT_EVENT_ERROR ++++ !!!\r\n");
            break;
		
        default:
//            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
			printf("### Other event id:%d ++++ !!!\r\n", event->event_id);
            break;
    }
    return ESP_OK;
}

void SendStat(void) {
	int rc;

    cJSON *pub = NULL;
    cJSON *item = NULL;
    cJSON *cmdtype = NULL;
    cJSON *cmdcopy = NULL;

	char *data = NULL;
	int ErrFlag;

	SensorInfoT *pSensorInfo = NULL; 
	SystemInfoT *pSystemInfo = NULL;
	PersistDataInfoT *pPersistDataInfo = NULL;

	AboveTxInfoT *pTxData = NULL;
	AboveRxInfoT *pRxData = NULL;

	GetAbovTxInfo(&pTxData);
	GetAbovRxInfo(&pRxData);	

	printf("MQTT: SendStat 1 !!!!  !!\r\n");

	GetSensorInfo(&pSensorInfo);
	printf("MQTT: SendStat 2 (%p) !!!!  !!\r\n", pSensorInfo);
	
	GetSystemInfo(&pSystemInfo);
	printf("MQTT: SendStat 3 (%p) !!!!  !!\r\n", pSystemInfo);

	GetPersistDataInfo(&pPersistDataInfo);
	printf("MQTT: SendStat 4 (%p) !!!!	!!\r\n", pPersistDataInfo);

	pub = cJSON_CreateObject();
	cmdtype = cJSON_CreateString("STAT");
	cmdcopy = cJSON_CreateNumber(0);

	item = cJSON_CreateObject();
    cJSON_AddNumberToObject(item, "no", pPersistDataInfo->DeviceId);
    cJSON_AddNumberToObject(item, "ver", pSystemInfo->SWVer);
    cJSON_AddNumberToObject(item, "faupower",pTxData->Power);
    cJSON_AddNumberToObject(item, "temp", pSensorInfo->temp);
    cJSON_AddNumberToObject(item, "co2", pSensorInfo->co2);
    cJSON_AddNumberToObject(item, "pm10_0", pSensorInfo->pm10_0);
    cJSON_AddNumberToObject(item, "pm2_5", pSensorInfo->pm2_5);
    cJSON_AddNumberToObject(item, "pm1_0", pSensorInfo->pm1_0);
    cJSON_AddNumberToObject(item, "smell", pSensorInfo->smell_level);
    cJSON_AddNumberToObject(item, "humidity", pSensorInfo->humidity);
    cJSON_AddNumberToObject(item, "fmode", pTxData->Mode);
    cJSON_AddNumberToObject(item, "ffan", pTxData->FanLevel);
	ErrFlag = pRxData->Err & 0x03;
	ErrFlag = (pRxData->FltTmr >= pRxData->FltTmrLmt) ? ErrFlag|0x04 : ErrFlag;

	if(ErrFlag&0x04)
		cJSON_AddStringToObject(item, "ffilter", "CHANGE_NEEDED");
	else
		cJSON_AddStringToObject(item, "ffilter", "OK");

    cJSON_AddNumberToObject(item, "ferror", ErrFlag);

	cJSON_AddItemToObject(pub, "cmdType", cmdtype);
	cJSON_AddItemToObject(pub, "cmdCopy", cmdcopy);
	cJSON_AddItemToObject(pub, "context", item);

	data = cJSON_Print(pub);
	printf("MQTT: SendStat 5 (%p) !!!!	!!\r\n", data);
	
	if (data && pMqttInf->mqttHdl) {
		rc = esp_mqtt_client_publish(( esp_mqtt_client_handle_t )pMqttInf->mqttHdl, pMqttInf->Topic, data, strlen(data), 0, 0);	
		printf("MQTT: Send STATUS context:: esp_mqtt_client_publish (%d) !!\r\n", rc);
	}

	if (data)
	{
		free(data);
		data = NULL;
	}	

	printf("MQTT: SendStat 6 (%p) !!!!	!!\r\n", data);

	cJSON_Delete(pub);

	printf("MQTT: SendStat END !!!!	!!\r\n");

	return;
}

void MessageCallback(esp_mqtt_event_handle_t event) {

	int match = 0;

	cJSON *root;
//	cJSON *obj;

	const char *str;
	double datavalue = 255;
//	unsigned int index;


	match = strncmp(pMqttInf->Topic,event->topic, event->topic_len);
	printf("### MessageCallback: (%d) \r\n", match);
	
	if (!match) {

		printf("### Mosquitto: Matched MessageCallback \r\n");
		printf("### Mosquitto: Len: %d \r\n", event->data_len);


		root = cJSON_Parse((const char*)event->data);
		if (root) {
			str = cJSON_GetStringValue(cJSON_GetObjectItem(root, "cmdType"));
			if (!str) {
				goto json_done;
			}

			if (!strncmp(str, "GET", 3)) {
				printf("Mosquitto: GET !!!! \r\n");

			}
			else if (!strncmp(str, "SET", 3)) {
				printf("Mosquitto: SET !!!! \r\n");

			}
			else if (!strncmp(str, "STAT", 4) || !strncmp(str, "RET", 3)) {
				printf("Mosquitto: Ignore %s command type \r\n", str);
				datavalue = cJSON_GetIntValue(cJSON_GetObjectItem(root, "cmdCopy"));
				printf("Mosquitto: What (%lf) \r\n", datavalue);		
			
			}
			else if (str) {
				printf("Mosquitto: Unknown Command Type(%s) \r\n", str);
				goto json_done;
			}

json_done:
			cJSON_Delete(root);			
		}
	}
}



void MqttTask(void* arg)
{

	while(1)
	{
		if(pMqttInf->Connected != 2) {
	        vTaskDelay(2000 / portTICK_PERIOD_MS);
			continue;
		}

		SendStat();

		sleep(1);
	}

	return;
}

void InitMqtt( MqttInfoT *pMqttInfo)
{
	esp_err_t err = 0;
	esp_mqtt_client_handle_t client = NULL;


	if(pMqttInfo == NULL) {
		printf("### InitMqtt :: return !!! \r\n");
		return;
	}

	pMqttInf = pMqttInfo;

	printf("### InitMqtt :: URI(%s), port(%d), clientID(%s), username(%s), password(%s) \r\n", pMqttInf->Addr, pMqttInf->Port, pMqttInf->ClientId, pMqttInf->Username, pMqttInf->Password  );

	sMqttCfg.host = pMqttInf->Addr;
	sMqttCfg.port = pMqttInf->Port;
	sMqttCfg.client_id = pMqttInf->ClientId;
	sMqttCfg.username = pMqttInf->Username;
	sMqttCfg.password = pMqttInf->Password;
	sMqttCfg.event_handle = mqtt_event_handler;

	client = esp_mqtt_client_init(&sMqttCfg);
	err = esp_mqtt_client_start(client);
	pMqttInf->mqttHdl = (void *)(client);
	printf("### InitMqtt :: esp_mqtt_client_start (%d) !!!\r\n", err);	

    xTaskCreate(MqttTask, "MQTT Task", 4096, NULL, 5, NULL);

	return;
}

