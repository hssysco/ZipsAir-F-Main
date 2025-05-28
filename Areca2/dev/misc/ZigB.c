#include <stdio.h>
#include <stddef.h>
#include <string.h>

#include "app.h"


//#define PACKET_FAU_HEADER		0x7E
//#define PACKET_FAU_TAIL 		0x7F
//#define PACKET_TIMEOUT			500
//
//#define ABOV_SERIAL_LENGTH		19//11
//#define ABOV_VERSION_LENGTH		10//5
//
//#define PACKET_ID_INDEX			1
//#define ITEM_NUM_INDEX			3
//#define ITEM_START_INDEX		4
//#define ABOV_NUMBER_OF_ITEMS	16
//
//#define MAX_PACKET_SIZE			64
//
//#define ITEM_LEN_NUM_ITEM 1
//#define ITEM_LEN_SERIAL1 12
//#define ITEM_LEN_VERSION 18
//#define ITEM_LEN_FAN_STATE 6
//#define ITEM_LEN_RPM 4
//#define ITEM_LEN_FILTER 7
//#define ITEM_LEN_VSP1 8
//#define ITEM_LEN_SENSOR 4
//#define ITEM_NUM_SENSOR 9
//#define ITEM_LEN_RESV_SET 1
//#define ITEM_LEN_RESV_TIMER 4
//#define ITEM_LEN_ERROR 3
//
//#define MOTOR_ERR				0x01
//
//#define SYNC_WIRED_ROUND_OFF_VAL 250

//const char *fake_serial = "XXXXXXXXXX";
//
//enum Item
//{
//	ITEM_SERIAL1 = 1,
//	ITEM_SERIAL2 = 2,
//	ITEM_SW_VERSION = 3,
//	ITEM_FAN_STATE = 4,
//	ITEM_RPM = 5,
//	ITEM_TIMER = 6, // not used
//	ITEM_FILTER = 7,
//	ITEM_VSP1 = 8,
//	ITEM_VSP2 = 9, // not used
//	ITEM_VSP3 = 10, // not used
//	ITEM_VSP4 = 11, // not used
//	ITEM_DIFF_PRESSURE = 12, // not used
//	ITEM_LED_STATUS = 20,
//	ITEM_SENSOR_TEMP,
//	ITEM_SENSOR_PM1_0,
//	ITEM_SENSOR_PM2_5,
//	ITEM_SENSOR_PM10_0,
//	ITEM_SENSOR_CO2,
//	ITEM_SENSOR_HUMIDITY,
//	ITEM_SENSOR_SMELL,
//	ITEM_SENSOR_GAS,
//	ITEM_SENSOR_PRESSURE,
//	ITEM_RESV_SET,
//	ITEM_RESV_TIMER,
//	ITEM_ERROR = 99,
//};
//
//PersistDataInfoT *p_PersistDataInfo = NULL;
//SystemInfoT *p_SystemInfo = NULL;
//CommInfoT *p_CommInfo = NULL;
//SensorInfoT *p_SensorInfo = NULL;
//    
//AboveTxInfoT *pthermoTxData = NULL;
//AboveRxInfoT *pthermoRxData = NULL;
//
//extern FlagStatus    UART1_TX_Sts;
//extern FlagStatus    UART1_RX_Sts;
//extern uint16_t     Uart_RxTime;
//extern AboveTxInfoT *pAboveTxData;
//extern AboveRxInfoT *pAboveRxData;
//
//
//uint8_t ThermoRxstart = 0;
//uint8_t ThermoRxend = 0;
//uint8_t ThermoRxData[100];
//uint8_t ThermoRxCnt = 0;
//
//extern void delay_1ms(uint32_t count);
//
//
//static int ReceiveData(unsigned char *pData, unsigned char *pDataLen)
//{
//    unsigned char packet_size = 0, read_data = 0;
//	unsigned short checksum = 0, checksumC = 0;
//    packet_size = pData[2];
//    packet_size += 6;
//    read_data = pData[2];
//    read_data += 3;
//    
//	if((pData == NULL) || (pDataLen == NULL)) 
//	{
//		return -1;
//	}
//
//    packet_size = *pDataLen;
//    read_data = packet_size -3;
//
//        
//	checksum = CRC16Checksum(pData, read_data);
//	checksumC = pData[packet_size-2];
//	checksumC <<= 8;
//	checksumC = checksumC + pData[packet_size-3];
//
//    if(checksum != checksumC) { 
//        *pDataLen = 0;
//        return -1;
//    }
//
//	*pDataLen = ThermoRxCnt;
//
//	return 0;
//
//}
//
////static char skipStaticData = false;
//char StrBuf[MAX_SERIAL_STR_LEN];
//
//
//
//void Thermostat_Tx(uint8_t revPktId)
//{
//	int idx = 0;
//	uint8_t NumofItems = 0, packetLen = 0;
//
//	uint16_t chksum = 0;
////	static uint8_t needToSync = false;
//	uint8_t Packet[MAX_PACKET_SIZE];
//
//
//
//	GetPersistDataInfo(&p_PersistDataInfo);
//	GetSystemInfo(&p_SystemInfo);
//	GetCommInfo(&p_CommInfo);
//	GetSensorInfo (&p_SensorInfo);
//
//
////	if(p_CommInfo->SyncWired > SYNC_WIRED_ROUND_OFF_VAL)
////	{
////		p_CommInfo->SyncWired = 0;
////		needToSync = true;
////	}
//
//	idx = 0;
//	memset(Packet, 0, sizeof(Packet));
//	memset(StrBuf, 0, MAX_SERIAL_STR_LEN);
//
//
//    Packet[idx++] = PACKET_FAU_HEADER;
//    Packet[idx++] = revPktId;
//    Packet[idx++] = packetLen;
//    
//    Packet[idx++] = NumofItems; /* number of items */
//    
//    NumofItems++;
//    Packet[idx++] = ITEM_SW_VERSION;
//    Packet[idx++] = 0x02;
//    Packet[idx++] = 'a';
//    Packet[idx++] = 0x0D;
//    
//    NumofItems++;
//    Packet[idx++] = ITEM_FAN_STATE;
//    Packet[idx++] = 0x4;
//    Packet[idx++] = pAboveRxData->Power;
//    Packet[idx++] = pAboveRxData->FanLevel;
//    Packet[idx++] = pAboveRxData->ErvLevel;
//    Packet[idx++] = pAboveRxData->Mode;
//
//    NumofItems++;
//    Packet[idx++] = ITEM_RPM;
//    Packet[idx++] = 0x2;
//    Packet[idx++] = pAboveRxData->PPS >> 8;
//    Packet[idx++] = pAboveRxData->PPS & 0xFF;
//    
//    NumofItems++;    
//    Packet[idx++] = ITEM_FILTER;
//    Packet[idx++] = 0x5;
//    Packet[idx++] = pAboveRxData->FltTmrRst;
//    Packet[idx++] = pAboveRxData->FltTmr >> 8;
//    Packet[idx++] = pAboveRxData->FltTmr & 0xFF;
//    Packet[idx++] = pAboveRxData->FltTmrLmt >> 8;
//    Packet[idx++] = pAboveRxData->FltTmrLmt & 0xFF;
//    
//    NumofItems++;
//    Packet[idx++] = ITEM_VSP1;
//    Packet[idx++] = 0x6;
//    Packet[idx++] = pAboveRxData->VSP[0];
//    Packet[idx++] = pAboveRxData->VSP[1];
//    Packet[idx++] = pAboveRxData->VSP[2];
//    Packet[idx++] = pAboveRxData->VSP[3];
//    Packet[idx++] = pAboveRxData->VSP[4];
//    Packet[idx++] = pAboveRxData->VSPOffset;
//    
//    NumofItems++;
//    Packet[idx++] = ITEM_RESV_SET;
//    Packet[idx++] = 1;
//    Packet[idx++] = p_SystemInfo->ResvTimeSet;
//    
////    NumofItems++;
////    Packet[idx++] = ITEM_RESV_TIMER;
////    Packet[idx++] = 4;
////    Packet[idx++] = (p_SystemInfo->ResvTimer >> 24) & 0xFF;
////    Packet[idx++] = (p_SystemInfo->ResvTimer >> 16) & 0xFF;
////    Packet[idx++] = (p_SystemInfo->ResvTimer >> 8) & 0xFF;
////    Packet[idx++] = p_SystemInfo->ResvTimer & 0xFF;
//    
//    NumofItems++;
//    Packet[idx++] = ITEM_ERROR;
//    Packet[idx++] = 0x1;
//    Packet[idx++] = pAboveRxData->Err;
//    
//    packetLen = idx;
//    Packet[2] = packetLen;
//    Packet[3] = NumofItems; /* number of items */
//   
//    chksum = CRC16Checksum(Packet, idx);
//    Packet[idx++] = chksum&0xFF;
//    Packet[idx++] = (chksum>>8)&0xFF;
//    Packet[idx++] = PACKET_FAU_TAIL;
//
//    
//
//    gpio_bit_set(GPIOB, GPIO_PIN_4);
//    delay_1ms(2);
//    SerialWrite(3, &Packet[0], idx);
//    delay_1ms(2);
//    gpio_bit_reset(GPIOB, GPIO_PIN_4);
//}
//
//void Thermostat_Rx() {
//    int ret = 0;
//	uint8_t NumofItems = 0, revLen = 0, revPktId = 0, i;
//	uint8_t *pItem = NULL;
//	uint16_t value = 0;
//   
//    if(ThermoRxend == 0) {
////        ThermoRxend = 0;
////        ThermoRxstart = 0;
////        ThermoRxCnt = 0;
////        memset(ThermoRxData, 0, sizeof(ThermoRxData));
//        return;
//    }
//    
////    Uart_RxTime = 0;
////    ThermoRxend = 0;
////
////    
////    ThermoRxend = 0;
////    ThermoRxstart = 0;
//    memcpy(pthermoRxData->Serial, ThermoRxData, ThermoRxCnt);
//    
//    revLen = ThermoRxCnt;
//	ret = ReceiveData(ThermoRxData, &revLen);
//	if(ret == 0 && revLen > 0)
//	{
////		disconnect_cnt = 0;
//		p_SystemInfo->ConnType |= DEV_CONNECTION_RS485;
//
//		revPktId = ThermoRxData[1];
////		if(needToSync)
////		{
////			if(revPktId > 2)
////			{
////				return -1;
////			}
////			needToSync = false;
////		}
////
////		if(revPktId == 0)
////		{
////			revPktId = 2;
////			p_CommInfo->SyncWired = 1;
//		}
//
//		NumofItems = ThermoRxData[3];
//		pItem = &(ThermoRxData[4]);
//
//		for(i=0; i<NumofItems; i++)
//		{
//			switch(pItem[0])
//			{
//				case ITEM_SERIAL1:
//					for(int j=0; j<pItem[1]; j++)
//					{
//						StrBuf[j] = pItem[2 + j];
//					}
//
////					if(strncmp(StrBuf, p_PersistDataInfo->Serial, MAX_SERIAL_STR_LEN) == 0)
////					{
////						skipStaticData = true;
////					}
//					break;
//				case ITEM_FAN_STATE:
////					if(skipStaticData && revPktId > p_CommInfo->SyncWired)
////					{
////						if(pAboveRxData->Err & OPENED_COVER_ERR)
////						{
////							if(pAboveTxData->Mode != OP_MODE_OFF)
////								FocedOffMode();
////						}
////						else
////						{
////							pAboveTxData->Power = pItem[2];
//							pAboveTxData->FanLevel = pItem[3];
//							pAboveTxData->Mode = pItem[5];
////						}
////					}
//					break;
//				case ITEM_RPM:
//                    pAboveTxData->RPMSet = pItem[2]<<8;   
//                    pAboveTxData->RPMSet |= pItem[3]; 
//					break;
//                    
//				case ITEM_FILTER:
////					if(revPktId > p_CommInfo->SyncWired)
////					{
//						pAboveTxData->FltTmrRst = pItem[2];
//						value = (pItem[3] << 8 | pItem[4]);
//                        pAboveTxData->FltTmr = value;
//                        value = (pItem[5] << 8 | pItem[6]);
////						if(value && p_PersistDataInfo->fltdifprslmt != value && value <= MAX_FILTER_USE_TIME)
////						{
//							pAboveTxData->FltTmrLmt = value;
////							p_PersistDataInfo->fltdifprslmt = pAboveTxData->FltTmrLmt;
////							SetValueU32(PROP_NAME_SYS_FAU_FLTPRSLMT, p_PersistDataInfo->fltdifprslmt);
////						}
////					}
//					break;
//				case ITEM_VSP1:
//					pAboveTxData->VSP[0] = pItem[2];
//					pAboveTxData->VSP[1] = pItem[3];
//					pAboveTxData->VSP[2] = pItem[4];
//					pAboveTxData->VSP[3] = pItem[5];
//					pAboveTxData->VSP[4] = pItem[6];
//					pAboveTxData->VSPOffset = pItem[7];
//					break;
////				case ITEM_LED_STATUS:
////					pAboveTxData->Led = pItem[2] << 8 | pItem[3];
////					break;
////				case ITEM_SENSOR_TEMP:
////					p_SensorInfo->temp = (pItem[2] << 8 | pItem[3]) / 10;
////					break;
////				case ITEM_SENSOR_PM1_0:
////					if(pItem[2] & 0x80) p_SensorInfo->selected_pm = PM_1_0;
////					p_SensorInfo->pm1_0 = ((pItem[2] & 0x7F) << 8) | pItem[3];
////					break;
////				case ITEM_SENSOR_PM2_5:
////					if(pItem[2] & 0x80) p_SensorInfo->selected_pm = PM_2_5;
////					p_SensorInfo->pm2_5 = ((pItem[2] & 0x7F) << 8) | pItem[3];
////					break;
////				case ITEM_SENSOR_PM10_0:
////					if(pItem[2] & 0x80) p_SensorInfo->selected_pm = PM_10_0;
////					p_SensorInfo->pm10_0 = ((pItem[2] & 0x7F) << 8) | pItem[3];
////					break;
////				case ITEM_SENSOR_CO2:
////					p_SensorInfo->co2 = pItem[2] << 8 | pItem[3];
////					break;
////				case ITEM_SENSOR_HUMIDITY:
////					p_SensorInfo->humidity = pItem[2] << 8 | pItem[3];
////					break;
////				case ITEM_SENSOR_SMELL:
////					p_SensorInfo->smell_iaq = pItem[2] << 8 | pItem[3];
////					break;
////				case ITEM_SENSOR_GAS:
////					p_SensorInfo->gas = (pItem[2] << 8 | pItem[3]) / 10;
////					break;
//				case ITEM_DIFF_PRESSURE:
//					p_SensorInfo->pressure = pItem[2];
//                    p_SensorInfo->pressure_limit = pItem[3];                    
//					break;
//				case ITEM_TIMER:
////					if(revPktId > p_CommInfo->SyncWired)
////					{
////						value = pItem[2] << 24 | pItem[3] << 16 | pItem[4] << 8 | pItem[5];
//                        value = pItem[2] << 8 | pItem[3];
//                        p_SystemInfo->ResvTimer = value;
////						if(value > 0)
////						{
////							if(p_SystemInfo->ResvStatus)
////							{
////								p_SystemInfo->ResvTimer = value;
////								//printf("resv uptadte, timer : %d\n", p_SystemInfo->ResvTimer);
////							}
////							else
////							{
////								p_SystemInfo->ResvTimer = value;
////								p_SystemInfo->ResvTime = (p_SystemInfo->ResvTimer/3600) + 1;
////								EnableFauResvTime(p_SystemInfo->ResvTime);
////								p_SystemInfo->ResvTimer = value;
////								p_SystemInfo->ResvStatus = 1;
////								//printf("resv on, timer : %d\n", p_SystemInfo->ResvTimer);
////							}
////						}
////						else
////						{
////							if(p_SystemInfo->ResvStatus)
////							{
////								DisableFauResvTime(p_SystemInfo->ResvTimerInstance);
////								p_SystemInfo->ResvTime = 0;
////								p_SystemInfo->ResvTimer = 0;
////								p_SystemInfo->ResvTimerInstance = -1;
////								p_SystemInfo->ResvStatus = 0;
////								//printf("resv off\n");
////							}
////						}
//						/* Only last sync item should update current sync for traverse upper item */
//						p_CommInfo->SyncWired = revPktId;
////					}
//					break;
//                case ITEM_ERROR:
// 					break;
//                   
//			}
//			pItem = pItem + (pItem[1]+2);
//		}
////	}
////	else
////	{
////		disconnect_cnt++;
////		if(disconnect_cnt >= DEV_DISCONNECT_CNT)
////		{
////			p_SystemInfo->ConnType &= ~DEV_CONNECTION_RS485;
////			disconnect_cnt = 0;
////		}
////		skipStaticData = false;
////	}
//    
//    ThermoRxend = 0;
//    ThermoRxstart = 0;
//    ThermoRxCnt = 0;
//    memset(ThermoRxData, 0, sizeof(ThermoRxData));
//
//    Thermostat_Tx(revPktId);
//}
//
/////------------------------------------------------------
////void ThermostatCommTask()
////{
////    CommandToThermostat();
////}
//
//void InitThermostatComms(void)
//{
//	GetAbovTxInfo(&pthermoTxData);
//	GetAbovRxInfo(&pthermoRxData);
//}


