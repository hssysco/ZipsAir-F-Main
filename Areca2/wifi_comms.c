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


#include <nvs_flash.h>
#include <lwip/err.h>
#include <lwip/sockets.h>
#include <lwip/sys.h>
#include <lwip/netdb.h>
#include <lwip/dns.h>
#include <lwip/dns.h>
#include <esp_wifi.h>
#include <time.h>
#include <mbedtls/aes.h>
#include <cJSON.h>

#include "app.h"


/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT		BIT0
#define WIFI_FAIL_BIT			BIT1

#define UDP_PORT_NUM	61001
#define TCPIP_PORT_NUM	61002

#define UDP_DATA_SIZE	1024
#define IV_LEN			16
#define	ENC_HEADER_LEN	4

#define HEADER_SIZE 2 * sizeof(unsigned short)

#define JSON_PKT_PREFIX 0xFF
#define JSON_PKT_CALIBRATION ((JSON_PKT_PREFIX << 8) | 0)
#define JSON_PKT_PAIRING ((JSON_PKT_PREFIX << 8) | 1)
#define JSON_PKT_UPGRADE ((JSON_PKT_PREFIX << 8) | 2)
#define JSON_PKT_LISTEN ((JSON_PKT_PREFIX << 8) | 3)

unsigned char ReceiveBuffer[UDP_DATA_SIZE + 1] = {0,};
unsigned char SendBuffer[UDP_DATA_SIZE + 1] = {0,};

unsigned char DReceiveBuffer[UDP_DATA_SIZE + 1] = {0,};
unsigned char ESendBuffer[UDP_DATA_SIZE + 1] = {0,};

mbedtls_aes_context AESCtx;
unsigned char AESKey[IV_LEN] = {0x65, 0x31, 0xFE, 0xA2, 0xBB, 0x69, 0x58, 0x6D, 0x89, 0x2A, 0x87, 0xD5, 0x01, 0x27, 0xC4, 0xD2};

bool paringMatched = false;
int udpSocket = -1;
int tcpSocket = -1;
unsigned int tcpServerIP = 0;
int Connected = -1;

struct sockaddr_in SendAddr;
static EventGroupHandle_t s_wifi_event_group = NULL;

static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
	wifi_scan_config_t scan_config = { 0 };
	WifiInfoT *pWifiInfo = NULL;
	GetWifiInfo(&pWifiInfo);	

	if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) 
	{
		esp_wifi_connect();
	} 
	else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED) 
	{
		if(pWifiInfo)
		{
			scan_config.ssid = pWifiInfo->SSID;	
			esp_wifi_scan_start(&scan_config, true);
		}
		
	} 
	else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) 
	{
		if(pWifiInfo)
		{
			pWifiInfo->Status = 0;
		}

		esp_wifi_connect();
	} 
	else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) 
	{
		ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;

		if(pWifiInfo)
		{
			pWifiInfo->Status = 1;

			pWifiInfo->FullIp = (event->ip_info.ip.addr);
			pWifiInfo->Ip = (event->ip_info.ip.addr);
			
			pWifiInfo->Ip &= 0xff000000;
			pWifiInfo->Ip >>= 24;
		}

		
		xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
	}

 }
								
static void scan_done_handler(void* arg, esp_event_base_t event_base,
                              int32_t event_id, void* event_data)
{
	uint16_t sta_number = 0;
	wifi_ap_record_t *ap_list_buffer;

	int ret = 0;
	WifiInfoT *pWifiInfo = NULL;

	esp_wifi_scan_get_ap_num(&sta_number);
	GetWifiInfo(&pWifiInfo);	

	if(sta_number == 1)
	{
		ap_list_buffer = malloc(sta_number * sizeof(wifi_ap_record_t));
		if (ap_list_buffer == NULL) 
		{
			return;
		}

		if (esp_wifi_scan_get_ap_records(&sta_number,(wifi_ap_record_t *)ap_list_buffer) == ESP_OK) 
		{
			if(pWifiInfo)
			{
				ret = strcmp((char *)pWifiInfo->SSID, (char *)(ap_list_buffer[0].ssid));
				if(ret == 0)
				{
					pWifiInfo->Rssi = ap_list_buffer[0].rssi;
				}
			}
		}

		free(ap_list_buffer);
	}
}


int InitWifiStation(unsigned char *pSsid, unsigned char *pPassword) 
{
	wifi_config_t wifi_config;
	int dataLen = 0;
	esp_err_t ret = 0;
	int result  = 1;

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	WifiInfoT *pWifiInfo = NULL;
	GetWifiInfo(&pWifiInfo);

	if((pSsid == NULL) || (pPassword == NULL)|| (pWifiInfo == NULL)) 
	{
		return result;
	}

    s_wifi_event_group = xEventGroupCreate();
	
	ret = esp_netif_init();
	if(ret)
	{
		goto DONE;
	}
	
	ret = esp_event_loop_create_default();
	if(ret)
	{
		goto DONE;
	}

	esp_netif_create_default_wifi_sta();

	ret = esp_wifi_init(&cfg);
	if(ret)
	{
		goto DONE;
	}

	ret = esp_wifi_set_storage(WIFI_STORAGE_RAM);
	if(ret)
	{
		goto DONE;
	}

	ret = esp_wifi_set_mode(WIFI_MODE_STA);
	if(ret)
	{
		goto DONE;
	}

	ret = esp_wifi_get_config(ESP_IF_WIFI_STA, &wifi_config);
	if(ret)
	{
		goto DONE;
	}

	dataLen = strlen((const char*)pSsid);
	if(dataLen > 0)
	{
		strncpy((char *)(wifi_config.sta.ssid), (char *)pSsid, dataLen);
		strncpy((char *)(pWifiInfo->SSID), (char *)pSsid, dataLen);
	}

	dataLen = strlen((const char*)pPassword);
	if(dataLen > 0)
	{
		strncpy((char *)(wifi_config.sta.password), (char *)pPassword, dataLen);
		strncpy((char *)(pWifiInfo->PWD), (char *)pPassword, dataLen);
	}

	ret = esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
	if(ret)
	{
		goto DONE;
	}


	ret = esp_event_handler_register(WIFI_EVENT,
										WIFI_EVENT_SCAN_DONE,
                                        &scan_done_handler,
                                        NULL);
	
	if(ret)
	{
		goto DONE;
	}

	  ret = esp_event_handler_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL);
	
	if(ret)
	{
		goto DONE;
	}

	ret = esp_event_handler_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL);
										
	if(ret)
	{
		goto DONE;
	}


	ret = esp_wifi_start();
	if(ret == 0)
	{
		return 0;
	}

DONE:
	if(s_wifi_event_group)
	{
		vEventGroupDelete(s_wifi_event_group);
	}

	return result;
}

void InitSecurity(void)
{
	srand((unsigned)time(NULL));
	mbedtls_aes_init(&AESCtx);
	return;
}

int EncryptData(unsigned char *pSrcData, unsigned int DataLen, unsigned char *pDstData)
{
	int rc = 0, i = 0;
	int Encrypted16Size = 0, EncryptedSize = 0;
	int random = 0;
	unsigned char AES_IV[16] = {0,};
	
	if((pSrcData == NULL) || (pDstData == NULL) || (DataLen == 0))
	{
		return -1;
	}

	Encrypted16Size = (DataLen/16);
	EncryptedSize = (Encrypted16Size+1)*16;
	
	for(i = DataLen; i < EncryptedSize; i++)
	{
		pSrcData[i] = EncryptedSize - DataLen;
	}
	
	pDstData[0] = EncryptedSize>>24;
	pDstData[1] = EncryptedSize>>16;
	pDstData[2] = EncryptedSize>>8;
	pDstData[3] = EncryptedSize;

	for(i = 0; i < IV_LEN; i++) 
	{
		random = (rand() % 256);
		AES_IV[i] = random;
		pDstData[ENC_HEADER_LEN + i] = AES_IV[i];
	}

	pDstData = (pDstData + ENC_HEADER_LEN + IV_LEN);

	mbedtls_aes_setkey_enc( &AESCtx, (const unsigned char*) &AESKey[0], 128 );

	rc = mbedtls_aes_crypt_cbc(&AESCtx, MBEDTLS_AES_ENCRYPT, EncryptedSize, AES_IV, pSrcData, pDstData);

	return rc;
	
}

int DecryptData(unsigned char *pSrcData, unsigned char *pDstData)
{
	int rc = -1;

	int EncryptedSize = 0;
	
	unsigned char aes_iv[IV_LEN] = {0,};

	if((pSrcData == NULL) || (pDstData == NULL))
	{
		return -1;
	}

	EncryptedSize = *pSrcData;
	EncryptedSize <<= 8;
	pSrcData++;

	EncryptedSize |= *pSrcData;
	EncryptedSize <<= 8;
	pSrcData++;

	EncryptedSize |= *pSrcData;
	EncryptedSize <<= 8;
	pSrcData++;

	EncryptedSize |= *pSrcData;
	pSrcData++;

	if((IV_LEN <= EncryptedSize) && (EncryptedSize <= UDP_DATA_SIZE - ( ENC_HEADER_LEN + IV_LEN )))
	{
		memcpy(aes_iv, pSrcData, IV_LEN);

		pSrcData = (pSrcData + IV_LEN);
			
		mbedtls_aes_setkey_dec( &AESCtx, (const unsigned char*) AESKey, 128 );
		rc = mbedtls_aes_crypt_cbc(&AESCtx, MBEDTLS_AES_DECRYPT, EncryptedSize , aes_iv, pSrcData, pDstData);
	}

	return rc;
	
}

int GetIntValuefromJson(cJSON *pJsonData, const char *pString)
{
	int result = -1;

	if((pJsonData == NULL) || (pString == NULL))
	{
		return result;
	}

	if(cJSON_HasObjectItem(pJsonData,pString))
	{
		result = cJSON_GetObjectItem(pJsonData,pString)->valueint; 
	}

	return result;
}

int GetStrValuefromJson(cJSON *pJsonData, const char *pString, char **ppData)
{
	int result = -1;

	if((pJsonData == NULL) || (pString == NULL))
	{
		return result;
	}

	if(cJSON_HasObjectItem(pJsonData,pString))
	{
		*ppData = cJSON_GetObjectItem(pJsonData,pString)->valuestring; 
		result = 0;
	}

	return result;
}

unsigned char *ReplyListen(unsigned int *pDataLen)
{
	int StrLen = 0;
	cJSON *pListen = NULL;
	cJSON *pReq = NULL;
	cJSON *pItem = NULL;
	char *pPrintedData = NULL;
	char SWVer[64] = {0,};
	unsigned short *header = NULL;
	unsigned char *pRet = NULL;

	unsigned int rpmSet = 0;

	PersistDataInfoT	*pPersistDataInfo = NULL;
	CommInfoT *pCommInfo = NULL;
	SystemInfoT *pSystemInfo = NULL;
	AboveRxInfoT *pRxData = NULL;
	AboveTxInfoT *pTxData = NULL;

	GetAbovTxInfo(&pTxData);

	GetAbovRxInfo(&pRxData);
	GetPersistDataInfo(&pPersistDataInfo);
	GetCommInfo(&pCommInfo);
	GetSystemInfo(&pSystemInfo);

	if((pDataLen == NULL) || (pPersistDataInfo == NULL) || (pRxData == NULL) || (pCommInfo == NULL))
	{
		return NULL;
	}

	pListen = cJSON_CreateObject();
	if(pListen)
	{
		cJSON_AddItemToObject(pListen, "listen", pReq = cJSON_CreateObject());
		if(pReq)
		{
			cJSON_AddItemToObject(pReq, "rq", pItem = cJSON_CreateObject());
			if(pItem)
			{
				cJSON_AddNumberToObject(pItem,"id", pPersistDataInfo->DeviceId);
				cJSON_AddStringToObject(pItem,"esr", pPersistDataInfo->Serial);
				cJSON_AddStringToObject(pItem,"efw", pSystemInfo->Version);
				cJSON_AddStringToObject(pItem,"asr", pRxData->Serial);
				if(pRxData->Ver >= 0)
				{
					snprintf(SWVer, sizeof(SWVer), "%.1f", (float)pRxData->Ver/10);
				}
					
				cJSON_AddStringToObject(pItem,"afw", SWVer);

				cJSON_AddNumberToObject(pItem,"fpwr", pRxData->Power);

				if(pTxData->FanLevel == pRxData->FanLevel)
				{
					cJSON_AddNumberToObject(pItem,"fl", pRxData->FanLevel);					
				}
				else
				{
					cJSON_AddNumberToObject(pItem,"fl", pTxData->FanLevel);					
				}
					
				if(pTxData->Mode == pRxData->Mode)
				{
					cJSON_AddNumberToObject(pItem,"fm", pRxData->Mode);
				}
				else
				{
					cJSON_AddNumberToObject(pItem,"fm", pTxData->Mode);
				}


				if(pTxData->RPMSet)
				{
					if((pPersistDataInfo->rpm1 == pTxData->RPM[0]) 
						&& (pPersistDataInfo->rpm2 == pTxData->RPM[1]) 
						&& (pPersistDataInfo->rpm3 == pTxData->RPM[2]) 
						&& (pPersistDataInfo->rpm4 == pTxData->RPM[3]) 
						&& (pPersistDataInfo->rpm5 == pTxData->RPM[4]))
					{
						rpmSet = 0;
						pCommInfo->Sync += 5;
					}
					else
					{
						rpmSet = 1;
					}
				}

				cJSON_AddNumberToObject(pItem,"fst", pCommInfo->Sync);
				cJSON_AddNumberToObject(pItem,"fpps", pRxData->PPS);
				cJSON_AddNumberToObject(pItem,"frt", 0);				/* not used.. temporary */
				cJSON_AddNumberToObject(pItem,"ft", pRxData->FltTmr);
				cJSON_AddNumberToObject(pItem,"ftl", pRxData->FltTmrLmt);
				cJSON_AddNumberToObject(pItem,"fdp", 0);				/* not used .. temporary */
				cJSON_AddNumberToObject(pItem,"fdpl", 0);				/* not used .. temporary */
				cJSON_AddNumberToObject(pItem,"ftr", pRxData->FltTmrRst);
				cJSON_AddNumberToObject(pItem,"fef", pRxData->Err);

				cJSON_AddNumberToObject(pItem,"fnl", 6);
				cJSON_AddNumberToObject(pItem,"fvspu", 1);

				cJSON_AddNumberToObject(pItem,"fvsp1", pRxData->VSP[0]);
				cJSON_AddNumberToObject(pItem,"fvsp2", pRxData->VSP[1]);
				cJSON_AddNumberToObject(pItem,"fvsp3", pRxData->VSP[2]);
				cJSON_AddNumberToObject(pItem,"fvsp4", pRxData->VSP[3]);
				cJSON_AddNumberToObject(pItem,"fvsp5", pRxData->VSP[4]);

				cJSON_AddNumberToObject(pItem,"frpm1", pPersistDataInfo->rpm1);
				cJSON_AddNumberToObject(pItem,"frpm2", pPersistDataInfo->rpm2);
				cJSON_AddNumberToObject(pItem,"frpm3", pPersistDataInfo->rpm3);
				cJSON_AddNumberToObject(pItem,"frpm4", pPersistDataInfo->rpm4);
				cJSON_AddNumberToObject(pItem,"frpm5", pPersistDataInfo->rpm5);

				cJSON_AddNumberToObject(pItem,"frpms", rpmSet);
				cJSON_AddNumberToObject(pItem,"fvspo", pRxData->VSPOffset);
				cJSON_AddNumberToObject(pItem,"fvsps", pCommInfo->VSPSet);
				
				cJSON_AddNumberToObject(pItem,"fresvs",pSystemInfo->ResvTimeSet);
				cJSON_AddNumberToObject(pItem,"fresv",pSystemInfo->ResvTimer);
			}
		}

		pPrintedData = cJSON_Print(pListen);
		if(pPrintedData)
		{
			StrLen = strlen(pPrintedData);
			if(StrLen)
			{
				pRet = (unsigned char *)malloc(StrLen + HEADER_SIZE);
				header = (unsigned short *)pRet;
				header[0] = htons(StrLen);
				header[1] = htons(JSON_PKT_LISTEN);
				memcpy(pRet + HEADER_SIZE, pPrintedData, StrLen);
				*pDataLen = StrLen + HEADER_SIZE;
				pRet = pRet;
			}
			else
			{
				pRet = NULL;
			}

			free(pPrintedData);
		}		

		cJSON_Delete(pListen);
		
	}

	return pRet;
}

unsigned char *ReplyPairing(bool Matched , unsigned int *pDataLen)
{
	int StrLen = 0;
	unsigned char *pRet = NULL;
	cJSON *pPairing = NULL;
	cJSON *pItem = NULL;
	char *pPrintedData = NULL;
	unsigned short *header = NULL;

	PersistDataInfoT	*pPersistDataInfo = NULL;
	WifiInfoT *pWifiInfo = NULL;
	SystemInfoT *pSystemInfo = NULL;

	GetPersistDataInfo(&pPersistDataInfo);
	GetWifiInfo(&pWifiInfo);
	GetSystemInfo(&pSystemInfo);

	if((pDataLen == NULL) || (pPersistDataInfo == NULL)|| (pWifiInfo == NULL))
	{
		return NULL;
	}

	pPairing = cJSON_CreateObject();
	if(pPairing)
	{
		cJSON_AddItemToObject(pPairing, "pairing", pItem = cJSON_CreateObject());
		if(pItem)
		{
			if(Matched == true)
			{
				cJSON_AddNumberToObject(pItem,"type", 0);
				cJSON_AddNumberToObject(pItem,"subtype", 0);
				cJSON_AddNumberToObject(pItem,"ip", pWifiInfo->Ip);
				cJSON_AddStringToObject(pItem,"serial", pPersistDataInfo->Serial);
				cJSON_AddStringToObject(pItem,"fw", pSystemInfo->Version);
				cJSON_AddNumberToObject(pItem,"rssi", pWifiInfo->Rssi);
				cJSON_AddNumberToObject(pItem,"fug", 0);
				cJSON_AddNumberToObject(pItem,"done", 1);

			}
			else
			{
				cJSON_AddStringToObject(pItem,"serial", pPersistDataInfo->Serial);
				cJSON_AddStringToObject(pItem,"fw",  pSystemInfo->Version);
				cJSON_AddNumberToObject(pItem,"type", 0);
				cJSON_AddNumberToObject(pItem,"subtype", 0);
				cJSON_AddNumberToObject(pItem,"ip", pWifiInfo->Ip);
				cJSON_AddNumberToObject(pItem,"id", pPersistDataInfo->DeviceId);
				cJSON_AddNumberToObject(pItem,"rssi", pWifiInfo->Rssi);
			}
		}

		pPrintedData = cJSON_Print(pPairing);
		if(pPrintedData)
		{
			StrLen = strlen(pPrintedData);
			if(StrLen)
			{
				pRet = (unsigned char *)malloc(StrLen + HEADER_SIZE);
				header = (unsigned short *)pRet;
				header[0] = htons(StrLen);
				header[1] = htons(JSON_PKT_PAIRING);
				memcpy(pRet + HEADER_SIZE, pPrintedData, StrLen);
				*pDataLen = StrLen + HEADER_SIZE;
				pRet = pRet;
			}
			else
			{
				pRet = NULL;
			}

			cJSON_free(pPrintedData);
		}		

		cJSON_Delete(pPairing);
	}

	return pRet;
}

int ParseListen( cJSON *pListen )
{
	int rc = 0;
	cJSON *pBcast = NULL, *pReq = NULL;

	int Bcast = 0;
	int Id =0, Value = 0;
	int fm = 0, fl = 0, fst = 0;

	char StrBuf[8] = {0,};
	unsigned int i = 0, index = 0;

	PersistDataInfoT	*pPersistDataInfo = NULL;
	CommInfoT *pCommInfo = NULL;
	AboveTxInfoT *pTxData = NULL;
	AboveRxInfoT *pRxData = NULL;
	SystemInfoT *pSystemInfo = NULL;
	SensorInfoT *pSensorInfo = NULL;

	GetAbovTxInfo(&pTxData);
	GetAbovRxInfo(&pRxData);

	GetCommInfo (&pCommInfo);
	GetSystemInfo (&pSystemInfo);
	GetSensorInfo (&pSensorInfo);

	GetPersistDataInfo(&pPersistDataInfo);

	if((pListen == NULL) || (pCommInfo == NULL) || (pPersistDataInfo == NULL) || (pTxData == NULL) || (pRxData == NULL))
	{
		return -1;
	}

	pBcast = cJSON_GetObjectItem(pListen, "bcast");
	if(pBcast)
	{
		Bcast = cJSON_GetIntValue(pBcast);
	}

	if(Bcast)
	{
		pReq = cJSON_GetObjectItem(pListen, "rq");
		if(pReq)
		{
			Id = GetIntValuefromJson(pReq,"id");
			if(Id == pPersistDataInfo->DeviceId)
			{
				fst = GetIntValuefromJson(pReq,"fst");
				if(fst == -1)
				{
					rc = -2;
				}

				if(fst == 0)
				{
					pCommInfo->Sync = 2;
				}
				else if(fst == 1)
				{
					pCommInfo->Sync = 1;
				}
				else if(fst > pCommInfo->Sync)
				{
					/* Apply resv info from controller if my sync is 0(reboot) */
					Value  = GetIntValuefromJson(pReq,"fresv");
					if(pCommInfo->Sync == 0 && Value > 0)
					{
						pSystemInfo->ResvTimer = Value;
						pSystemInfo->ResvTime = (pSystemInfo->ResvTimer/3600) + 1;
						EnableFauResvTime(pSystemInfo->ResvTime);
						pSystemInfo->ResvTimer = Value;
						pSystemInfo->ResvStatus = 1;
						printf("init resv on, timer = %d\n", Value);
					}
					else
					{
						if(Value > 0)
						{
							if(pSystemInfo->ResvStatus)
							{
								pSystemInfo->ResvTimer = Value;
								printf("resv uptadte, timer : %d \r\n", pSystemInfo->ResvTimer);
							}
							else
							{
								pSystemInfo->ResvTimer = Value;
								pSystemInfo->ResvTime = (pSystemInfo->ResvTimer/3600) + 1;
								EnableFauResvTime(pSystemInfo->ResvTime);
								pSystemInfo->ResvTimer = Value;
								pSystemInfo->ResvStatus = 1;
								printf("resv on, timer : %d \r\n", pSystemInfo->ResvTimer);
							}
						}
						else
						{
							if(pSystemInfo->ResvStatus)
							{
								DisableFauResvTime(pSystemInfo->ResvTimerInstance);
								pSystemInfo->ResvTime = 0;
								pSystemInfo->ResvTimer = 0;
								pSystemInfo->ResvTimerInstance = -1;
								pSystemInfo->ResvStatus = 0;
								printf("resv off\n");
							}
						}
					}
					pCommInfo->Sync = fst;
				}

				if(fst >= pCommInfo->Sync)
				{
					Value  = GetIntValuefromJson(pReq,"fpwr");
					if(Value != -1)
					{
						if(pTxData->Power != Value)
						{
							pTxData->Power = Value;							
						}
					}

					fm  = GetIntValuefromJson(pReq,"fm");
					fl  = GetIntValuefromJson(pReq,"fl");
					if((fm != -1)&&(fl != -1))
					{
						if(pRxData->Err & OPENED_COVER_ERR)
						{
							if(fm != 0)
							{
								FocedOffMode();
							}
						}
						else
						{
							if(pTxData->Mode != fm)
							{
								pTxData->Mode = fm;
							}

							if(pTxData->FanLevel != fl)
							{
								pTxData->FanLevel = fl;
							}
						}
					}
					
					Value  = GetIntValuefromJson(pReq,"ft");
					if(Value != -1)
					{
						if(pTxData->FltTmr != Value)
						{
							pTxData->FltTmr = Value;
						}
					}

					Value  = GetIntValuefromJson(pReq,"ftl");
					if(Value != -1 && Value <= MAX_FILTER_USE_TIME)
					{
						if(Value && pPersistDataInfo->fltdifprslmt != Value)
						{
							pTxData->FltTmrLmt = Value;
							pPersistDataInfo->fltdifprslmt = pTxData->FltTmrLmt;
							SetValueU32(PROP_NAME_SYS_FAU_FLTPRSLMT, pPersistDataInfo->fltdifprslmt);
						}
					}

					Value  = GetIntValuefromJson(pReq,"fdp");
					Value  = GetIntValuefromJson(pReq,"fdpl");
	
					Value  = GetIntValuefromJson(pReq,"ftr");
					if(Value != -1)
					{
						if(pTxData->FltTmrRst != Value)
						{
							pTxData->FltTmrRst = Value;
						}
					}

					Value  = GetIntValuefromJson(pReq,"fnl");

					Value  = GetIntValuefromJson(pReq,"fvsps");
					if(Value != -1)
					{
						if(pCommInfo->VSPSet != Value)
						{
							pCommInfo->VSPSet = Value;
						}

					}

					Value  = GetIntValuefromJson(pReq,"frpms");
					if(Value != -1)
					{
						if(pTxData->RPMSet != Value)
						{
							printf("### frpms (%d) !!! \r\n", pTxData->RPMSet);
							pTxData->RPMSet = Value;
						}
					}
					
/*
					Value  = GetIntValuefromJson(pReq,"fvspo");
					if(Value != -1)
					{
						if(pTxData->VSPOffset != Value)
						{
							pTxData->VSPOffset = Value;
						}
					}
*/
					Value  = GetIntValuefromJson(pReq,"led");
					if(Value != -1)
					{
						if(pTxData->Led != Value)
						{
							pTxData->Led = Value;
						}
					}

					Value  = GetIntValuefromJson(pReq,"spm");
					if(Value != -1)
					{
						pSensorInfo->selected_pm = Value;
					}

					Value  = GetIntValuefromJson(pReq,"pm1_0");
					if(Value != -1)
					{
						pSensorInfo->pm1_0 = Value;
					}

					Value  = GetIntValuefromJson(pReq,"pm2_5");
					if(Value != -1)
					{
						pSensorInfo->pm2_5 = Value;
					}

					Value  = GetIntValuefromJson(pReq,"pm10_0");
					if(Value != -1)
					{
						pSensorInfo->pm10_0 = Value;
					}

					Value  = GetIntValuefromJson(pReq,"co");
					if(Value != -1)
					{
						pSensorInfo->co2 = Value;
					}

					Value  = GetIntValuefromJson(pReq,"fresvs");
					if(Value != -1)
					{
						pSystemInfo->ResvTimeSet = Value;
					}

					/* Echo from controller, reset fresvs of sub controller */
					if(pSystemInfo->ResvTimeSet)
					{
						printf("ResvTimeSet (%d) \n", Value);
						Value  = GetIntValuefromJson(pReq,"fresv");
						if(Value != -1)
						{
							printf("ResvTimer (%d) \n", Value);
							pSystemInfo->ResvTimer = Value;
						}
						pSystemInfo->ResvTime = (pSystemInfo->ResvTimer/3600) + 1;
						pSystemInfo->ResvTimeSet = 0;
					}

#if 0
					if(pCommInfo->VSPSet)
					{
						index = 0;
						for(i = 1; i < 6; i++)
						{
							snprintf(StrBuf, sizeof(StrBuf),"fvsp%d",i);
							Value = GetIntValuefromJson(pReq, StrBuf);
							if(Value != -1)
							{
								index = i;
								index -= i;

								if(pTxData->VSP[index] != Value)
								{
									pTxData->VSP[index] = Value;
								}
							}
						}
					}
#endif
					if(pTxData->RPMSet)
					{
						index = 0;
						for(i = 1; i < 6; i++)
						{
							snprintf(StrBuf, sizeof(StrBuf),"frpm%d",i);
							Value = GetIntValuefromJson(pReq, StrBuf);
							if(Value != -1)
							{
								index = i;
								index -= 1;
								if((pTxData->RPM[index] != Value) && (Value != 0))
								{
									SetRPM(i, Value);
									pTxData->RPM[index] = Value;
									pTxData->VSP[index] = PPS2VSP(Value);
									printf("### frpm (%d:%d:%d) !!! \r\n", index, Value , pTxData->VSP[index]);
									
								}						
							}
						}
					}

				}

			}
			else
			{
				rc = -2;
			}

		}

	}
	
	return rc;
}

int ParsePairing( cJSON *pPairing, bool *pMatched )
{
	int Ret = 0;
	int paring_size = 0, i = 0;
	unsigned char deviceId = 0;
	
	cJSON *pItem  = NULL;

	PersistDataInfoT	*pPersistDataInfo = NULL;
	WifiInfoT *pWifiInfo = NULL;

	char *pSerial = NULL;
	char *pSSID = NULL;
	char *pPWD = NULL;
	unsigned int IP = 0;
	unsigned int netId = 0;
	int gDeviceId = -1;
	int UpgradeOn = 0;
	
	GetPersistDataInfo(&pPersistDataInfo);
	GetWifiInfo(&pWifiInfo);

	if((pPairing == NULL) || (pMatched == NULL) || (pPersistDataInfo == NULL) || (pWifiInfo == NULL))
	{
		return -1;
	}

	paring_size = cJSON_GetArraySize(pPairing);
	for(i = 0; i < paring_size; i++)
	{
		pItem = cJSON_GetArrayItem(pPairing,i);
		if(pItem)
		{
			GetStrValuefromJson(pItem, "serial", &pSerial);
			IP  = GetIntValuefromJson(pItem,"ip");
			if(IP == pWifiInfo->Ip) 
			{
				if(pSerial != NULL)
				{
					if(strcmp(pSerial, pPersistDataInfo->Serial) == 0)
					{
						*pMatched = true;
						
						GetStrValuefromJson(pItem, "ssid", &pSSID);
						if(pSSID != NULL)
						{
							Ret = SetValueStr(PROP_NAME_SYS_SSID, pSSID);
						}

						GetStrValuefromJson(pItem, "pwd", &pPWD);
						if(pPWD != NULL)
						{
							Ret = SetValueStr(PROP_NAME_SYS_PWD, pPWD);
						}

						netId = GetIntValuefromJson(pItem, "netid");
						if(netId > 0)
						{
							Ret = SetValueU32(PROP_NAME_SYS_NETID, netId);
						}

						gDeviceId = GetIntValuefromJson(pItem,"id");
						if(gDeviceId != -1)
						{
							deviceId = gDeviceId;	
							Ret = SetValueU8(PROP_NAME_SYS_ROOMID, deviceId);
						}
						
						UpgradeOn = GetIntValuefromJson(pItem,"fug");
						if(UpgradeOn != -1)
						{
							Ret = SetValueU8(PROP_NAME_APP_UPGRADE, UpgradeOn);
						}						
					}
				}
			}							
		}
	}

	return Ret;
}

int ProcessListenData( unsigned char *pData )
{
	int result = 0;
	unsigned char *pRet;
	
	cJSON *pRecv = NULL, *pListen = NULL;
	int dataLen = 0;

	unsigned int jsonLen = 0;

	PersistDataInfoT	*pPersistDataInfo = NULL;
	GetPersistDataInfo(&pPersistDataInfo);

	if((pData == NULL) || (pPersistDataInfo == NULL))
	{
		return -1;
	}

	pRecv = cJSON_Parse((char *)pData);
	if(pRecv)
	{
		if(cJSON_HasObjectItem(pRecv, "listen"))
		{
			pListen = cJSON_GetObjectItem(pRecv, "listen");
			if(pListen)
			{
				result = ParseListen(pListen);
				if(result == 0)
				{
					jsonLen = 0;
					pRet = ReplyListen( &jsonLen );
					if(pRet)
					{
						dataLen = write(tcpSocket, pRet, jsonLen);
						if(dataLen != jsonLen)
						{
							result = -2;
						}

						free(pRet);
					}
				}
			}
		}
	}
	else
	{
		const char *error_ptr = cJSON_GetErrorPtr();
		if(error_ptr != NULL)
		{
			printf("Json error before %s\n", error_ptr);
		}
	}

	if(pRecv)
	{
		cJSON_Delete(pRecv);
	}

	return result;
}

int ParseUDPPacket( unsigned char *pData, unsigned int netId, bool *pMatched )
{
	int result = -1;
	int Bcast = 0, NetId = 0;

	cJSON *pRecv = NULL, *pPairing = NULL;

	pRecv = cJSON_Parse((char *)pData);
	if(pRecv)
	{
		if(cJSON_HasObjectItem(pRecv, "bcast"))
		{
			Bcast = cJSON_GetObjectItem(pRecv, "bcast")->valueint;
			if(Bcast)
			{
				if(cJSON_HasObjectItem(pRecv, "netid"))
				{
					NetId = cJSON_GetObjectItem(pRecv, "netid")->valueint;
				}

				if(cJSON_HasObjectItem(pRecv, "ip") && NetId == netId)
				{
					tcpServerIP = (unsigned int)cJSON_GetObjectItem(pRecv, "ip")->valuedouble;
					result = 0;
				}
				else
				{
					if(tcpServerIP == 0)
					{
						printf("%s\n", cJSON_Print(pRecv));
					}
				}

				if(cJSON_HasObjectItem(pRecv, "pairing"))
				{
					pPairing = cJSON_GetObjectItem(pRecv, "pairing");
					if(pPairing)
					{
						result = ParsePairing( pPairing, pMatched );						
					}
				}				
			}
		}
		else
		{
			result = 0;
		}
	}

	cJSON_Delete(pRecv);

	return result;
}

void UDPBroadcastTask(void *pvParameters)
{
	int RecvLen = 0, result = 0, rc = 0;
	int BroadcastEn = 1;
	socklen_t adr_sz;
	struct sockaddr_in sockAddr;

	int flags = MSG_NOSIGNAL;

	struct timeval tv;
	fd_set read_fds;
	
	int readsize = 0, remainsize = 0;
	unsigned char *pucData = NULL, *pDstData = NULL;

	PersistDataInfoT	*pPersistDataInfo = NULL;

	WifiInfoT	*pWifiInfo = NULL;
	GetPersistDataInfo(&pPersistDataInfo);
	GetWifiInfo(&pWifiInfo);

	if(pPersistDataInfo->BcastEncryption)
	{
		InitSecurity();
	}

retry_conn:
	while(1)
	{
		if(pWifiInfo->Status == 1)
		{
			break;
		}
		sleep(3);
	}

	udpSocket = socket(PF_INET, SOCK_DGRAM, 0);

	if ( udpSocket < 0 )
	{
		printf("socket call failed !!\r\n");
		return;
	}

	if( setsockopt(udpSocket, SOL_SOCKET, SO_BROADCAST, &BroadcastEn, sizeof(BroadcastEn)) == -1)
	{
		printf("Set SO_BROADCAST opt to udpSocket failed %s\n", strerror(errno));
	}

	memset(&sockAddr, 0, sizeof(struct sockaddr_in));
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	sockAddr.sin_port = htons(UDP_PORT_NUM);

	if (bind(udpSocket, (struct sockaddr *)&sockAddr, sizeof(struct sockaddr_in)) == -1)
	{
		printf("Bind to Port Number %d failed\n",UDP_PORT_NUM);
		close(udpSocket);
		return;
	}

	tv.tv_sec = 6;
	tv.tv_usec = 0;

	while(1)
	{
		FD_ZERO(&read_fds);
		FD_SET(udpSocket,&read_fds);

		do
		{
			rc = select(udpSocket + 1, &read_fds, (fd_set*)0, (fd_set*)0, &tv);
		} while (rc == -1 && errno == EINTR);

		if (FD_ISSET(udpSocket, &read_fds))
		{
			adr_sz	= sizeof( SendAddr);
			memset(&SendAddr, 0, sizeof(struct sockaddr_in));

			memset(ReceiveBuffer, 0, UDP_DATA_SIZE);
			remainsize = UDP_DATA_SIZE;
			readsize = 0;
			pucData = (unsigned char*)ReceiveBuffer;
			

			while(remainsize)
			{
				RecvLen = recvfrom(udpSocket, pucData, remainsize, flags, (struct sockaddr*)&SendAddr, &adr_sz);
				if (RecvLen < 0)
				{
					if (errno==EBADF || errno==EFAULT || errno==EINVAL || errno==EIO || errno==ENOSPC || errno==EPIPE)
					{
						goto done;
					}

					break;
				}
				else if (RecvLen==0)
				{
					if (readsize==0)
					{
						goto done;
					}

					break;
				}

				remainsize -= RecvLen;
				readsize += RecvLen;
				pucData += RecvLen;
			}

			if(pPersistDataInfo->BcastEncryption)
			{
				memset(DReceiveBuffer, 0, sizeof(UDP_DATA_SIZE + 1));
				DecryptData(ReceiveBuffer, DReceiveBuffer);
				pDstData = (unsigned char*)DReceiveBuffer;			
			}
			else
			{
				pDstData = (unsigned char*)ReceiveBuffer;
			}

			result =  ParseUDPPacket(pDstData, pPersistDataInfo->NetId, &paringMatched );
			if(result < 0)
			{
				printf("ParseUDPPacket failed\n");
				goto done;
			}			
		}
	}

	done:
		if (udpSocket>=0) close(udpSocket);
		udpSocket = -1;
		if (tcpSocket>=0) close(tcpSocket);
		tcpSocket = -1; 
		tcpServerIP = 0;
		sleep(1);
		goto retry_conn;

	return;
}

int TCPClientInit(int *pSocketFd)
{
	int SocketFd = -1;
	int ret = -1;
	char StrBuf[128] = {0,};
	char tmp[4] = {0,};

	struct sockaddr_in ServerAddr;
	struct timeval timeout;
	fd_set writefds;

	if(pSocketFd == NULL)
	{
		return -1;
	}
		
	printf("#### TCPClientInit 1 (%lld),(%x)!!! \r\n", (long long int)tcpServerIP, tcpServerIP);
	
	tmp[0] = (tcpServerIP&0xff);
	tmp[1] = (tcpServerIP >> 8)&0xff;
	tmp[2] = (tcpServerIP >> 16)&0xff;
	tmp[3] = (tcpServerIP >> 24)&0xff;

	snprintf(StrBuf, sizeof(StrBuf), "%d.%d.%d.%d", tmp[0], tmp[1], tmp[2], tmp[3]);

	printf("#### TCPClientInit 2 (%s)!!! \r\n", StrBuf);
	
	SocketFd = socket(AF_INET, SOCK_STREAM, 0);
	if(SocketFd > 0)
	{
		ServerAddr.sin_family = AF_INET;
		ServerAddr.sin_addr.s_addr = inet_addr(StrBuf);
		ServerAddr.sin_port = htons(TCPIP_PORT_NUM);


		ret = connect(SocketFd, (struct sockaddr *)&ServerAddr, sizeof(ServerAddr));
		printf("#### TCPClientInit Connect (%d) !!! \r\n", ret);
		if(ret == 0)
		{
			FD_ZERO(&writefds);
			FD_SET(SocketFd, &writefds);
			timeout.tv_sec = 1;
			timeout.tv_usec = 0;
			
			ret = select(SocketFd+1, NULL, &writefds, NULL, &timeout);
			printf("#### TCPClientInit select (%d) !!! \r\n", ret);
			if(ret <= 0)
			{
				close(SocketFd);
			}
		}
	}
	
	*pSocketFd = SocketFd;

	return ret;
	
}

///------------------------------------------------------
void TCPClientTask(void *pvParameters)
{
	int oret = 0, rc = 0, offset = 0;
	unsigned int jsonLen = 0, remain_size = 0;
	unsigned short datasize, type;
	unsigned short *buf;
	unsigned char *pRet, *pRecvBuf;
	static int disconnect_cnt = 0;

	SystemInfoT *pSystemInfo = NULL;

	struct timeval tv;
	fd_set read_fds;

	tv.tv_sec = 6;
	tv.tv_usec = 0;

	GetSystemInfo(&pSystemInfo);

	while(1)
	{
		if(tcpServerIP == 0)
		{
			sleep(3);
			continue;
		}

		if(tcpSocket == -1)
		{
			rc = TCPClientInit(&tcpSocket);
			if(rc < 0)
			{
				close(tcpSocket);
				tcpSocket = -1; 
				sleep(3);
				continue;
			}
		}

		jsonLen = 0;
		pRet = ReplyPairing( paringMatched, &jsonLen );
		if(pRet)
		{
			oret = write(tcpSocket, pRet, jsonLen);
			if(oret < 0)
			{
				printf("ReplyPairing write fail %s\n", strerror(errno));
				close(tcpSocket);
				tcpSocket = -1; 
				tcpServerIP = 0;
				free(pRet);
				continue;
			}
			
			if(paringMatched)
			{
				vTaskDelay(100); /* 1 sec */
				esp_wifi_disconnect();
				vTaskDelay(400); /* 4 sec */
				esp_restart();
			}

			free(pRet);
		}

		FD_ZERO(&read_fds);
		FD_SET(tcpSocket,&read_fds);
		do
		{
			rc = select(tcpSocket + 1, &read_fds, (fd_set*)0, (fd_set*)0, &tv);
		} while (rc == -1 && errno == EINTR);

		if (FD_ISSET(tcpSocket, &read_fds))
		{
			pRecvBuf = (unsigned char *)malloc(HEADER_SIZE);
			oret = read(tcpSocket, pRecvBuf, HEADER_SIZE);
			if(oret == HEADER_SIZE)
			{
				buf = (unsigned short *)pRecvBuf;
				datasize = ntohs(buf[0]);
				type = ntohs(buf[1]);
				free(pRecvBuf);

				if(type < JSON_PKT_CALIBRATION || type > JSON_PKT_LISTEN)
				{
					printf("Invalid json packet type %d\n", type);
					close(tcpSocket);
					tcpSocket = -1;
					tcpServerIP = 0;
					continue;
				}

				remain_size = datasize;
				offset = 0;

				pRecvBuf = (unsigned char *)malloc(datasize);
				while(remain_size)
				{
					oret = read(tcpSocket, pRecvBuf + offset, remain_size);
					if(oret <= 0)
					{
						printf("Failed to get remain packet %d\n", remain_size);
						close(tcpSocket);
						tcpSocket = -1;
						tcpServerIP = 0;
						break;
					}

					remain_size -= oret;
					offset += oret;
				}

				if(remain_size == 0)
				{
					disconnect_cnt = 0;
					pSystemInfo->ConnType |= DEV_CONNECTION_WIFI;
					ProcessListenData(pRecvBuf);
				}
				free(pRecvBuf);
			}
		}
		else
		{
			disconnect_cnt++;
			if(disconnect_cnt >= DEV_DISCONNECT_CNT)
			{
				printf("WIFI connection has lost\n");
				close(tcpSocket);
				tcpSocket = -1;
				tcpServerIP = 0;
				disconnect_cnt = 0;
				pSystemInfo->ConnType &= ~DEV_CONNECTION_WIFI;
			}
		}
	}
}

void InitWIFIComms(void)
{
	xTaskCreatePinnedToCore(UDPBroadcastTask, "UDP Broadcast Task", 4096, NULL, 5, NULL, 0);
	xTaskCreatePinnedToCore(TCPClientTask, "TCP Broadcast Task", 4096, NULL, 5, NULL, 0);
}

