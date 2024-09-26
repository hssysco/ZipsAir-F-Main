#include <stdio.h>
#include <stddef.h>
#include <string.h>
//#include <freertos/FreeRTOS.h>
//#include <freertos/task.h>
//#include <freertos/event_groups.h>
//#include <freertos/queue.h>
//#include <freertos/semphr.h>
//#include <esp_system.h>
//#include <esp_event.h>
//#include <esp_log.h>
//#include <esp_sleep.h>
//#include <esp_err.h>

#include "app.h"


#define PACKET_FAU_HEADER		0x7E
#define PACKET_FAU_TAIL 		0x7F
#define PACKET_TIMEOUT			500

#define ABOV_SERIAL_LENGTH		19//11
#define ABOV_VERSION_LENGTH		10//5

#define PACKET_ID_INDEX			1
#define ITEM_NUM_INDEX			3
#define ITEM_START_INDEX		4
#define MIN_RX_NUM				6

#define FAU_TX_PACKET_SIZE		28 //58
#define FAU_TX_PACKET_LENGTH	22 //52
#define FAU_RX_PACKET_SIZE		64
#define FAU_LEN_VER				2
#define FAU_LEN_SR				10
#define FAU_LEN_FAN_STATE		4
#define FAU_LEN_RPM				2
#define FAU_LEN_DIFF_PRESSURE	2
#define FAU_LEN_TIMER			2
#define FAU_LEN_FILTER			5
#define FAU_LEN_VSP1			6
#define FAU_LEN_ERR				1

#define MOTOR_ERR				0x01

enum FAUItem 
{
	ITEM_SERIAL1 = 1,
	ITEM_SERIAL2 = 2, // not used
	ITEM_SW_VERSION = 3,
	ITEM_FAN_STATE = 4,
	ITEM_RPM = 5,
	ITEM_TIMER = 6, // not used
	ITEM_FILTER = 7,
	ITEM_VSP1 = 8,
	ITEM_VSP2 = 9, // not used
	ITEM_VSP3 = 10, // not used
	ITEM_VSP4 = 11, // not used
	ITEM_DIFF_PRESSURE = 12, // not used
	ITEM_ERROR = 99,
};


AboveTxInfoT *pAboveTxData = NULL;
AboveRxInfoT *pAboveRxData = NULL;

uint8_t AboveRxstart = 0;
uint8_t AboveRxend = 0;
uint8_t AboveRxData[100];
uint8_t AboveRxCnt = 0;
    
//static void SendData(unsigned char *pData, unsigned int DataLen)
//{
//	SerialWrite(AVOVE, pData, DataLen);
//	return;
//}
static void SendData(int channel, unsigned char *pData, unsigned int DataLen)
{
	SerialWrite(channel, pData, DataLen);
	return;
}

static int ReceiveData(unsigned char *pData, unsigned char *pDataLen)
{
	unsigned char packet_size = 0, read_data = 0;
	unsigned short checksum = 0, checksumC = 0;
    
	if((pData == NULL) || (pDataLen == NULL)) 
	{
		return -1;
	}

    while(AboveRxend == 0);
    packet_size = pData[2];
    packet_size += 6;
    read_data = pData[2];
    read_data += 3;

        
	checksum = CRC16Checksum(pData, read_data);
	checksumC = pData[packet_size-2];
	checksumC <<= 8;
	checksumC = checksumC + pData[packet_size-3];

    if(checksum != checksumC) { 
        return -1;
    }

	*pDataLen = AboveRxCnt;

	return 0;
}

int CommandToAbov( AboveTxInfoT *pTxData,  AboveRxInfoT *pRxData ) 
{

	int ret = 0, idx = 0, readsize = 0;
	unsigned char Packet[FAU_RX_PACKET_SIZE];
	unsigned char PktLen = 0, NumofItems = 0, revLen = 0, revPktId = 0, i = 0, index = 0;
	static unsigned char PktId;
	
	unsigned short chksum = 0;
	unsigned char *pItem = NULL;

	if((pTxData == NULL)||(pRxData == NULL))
	{
		return -1;
	}

	idx = 0;
	readsize = 0;
	memset(Packet, 0, sizeof(Packet));

	Packet[idx++] = PACKET_FAU_HEADER;

	if(pRxData->Ver > 0) 
	{
		PktLen = FAU_TX_PACKET_LENGTH;
		NumofItems = 3;
	}
	else 
	{
		PktId = 0;
		PktLen = (FAU_TX_PACKET_LENGTH - 8);
		NumofItems = 2;
	}

	Packet[idx++] = PktId;
	Packet[idx++] = PktLen;
	Packet[idx++] = NumofItems;
	Packet[idx++] = ITEM_FAN_STATE; /* item type: Fan State */
	Packet[idx++] = FAU_LEN_FAN_STATE; /* item length */
	Packet[idx++] = pTxData->Power;			/* fau power */
	Packet[idx++] = pTxData->FanLevel;		/* fau level */
	Packet[idx++] = 0; /* erv level: do not consider erv level. just send 0 for this position */

	if(pTxData->Mode == OP_MODE_AUTO)
	{
		Packet[idx++] = 5;
	}
//	else if(pTxData->Mode == OP_MODE_NORMAL)
//	{
//		Packet[idx++] = 4;
//	}		
	else 
	{
		Packet[idx++] = 0;
	}

	Packet[idx++] = ITEM_FILTER; /* item type: Filter */
	Packet[idx++] = FAU_LEN_FILTER; /* item length */
	Packet[idx++] = pTxData->FltTmrRst; /* filter time reset */
	pTxData->FltTmrRst = 0;
	Packet[idx++] = 0; /* filter time used H */
	Packet[idx++] = 0; /* filter time used L */
	Packet[idx++] = (pTxData->FltTmrLmt>>8)&0xFF; /* filter life time H */
	Packet[idx++] = (pTxData->FltTmrLmt)&0xFF; /* filter life time L */
	
	if (NumofItems > 2) 
	{
		Packet[idx++] = ITEM_VSP1; /* item type: VSP */
		Packet[idx++] = FAU_LEN_VSP1; /* item length */
		Packet[idx++] = pTxData->VSP[0]; /* vsp1 */
		Packet[idx++] = pTxData->VSP[1]; /* vsp2 */
		Packet[idx++] = pTxData->VSP[2]; /* vsp3 */
		Packet[idx++] = pTxData->VSP[3]; /* vsp4 */
		Packet[idx++] = pTxData->VSP[4]; /* vsp5 */
		Packet[idx++] = pTxData->VSPOffset;
	}
	
	readsize = idx;
	chksum = CRC16Checksum(Packet, readsize);
	Packet[idx++] = chksum&0xFF;
	Packet[idx++] = (chksum>>8)&0xFF;
	Packet[idx++] = PACKET_FAU_TAIL;

    gpio_bit_set(GPIOA, GPIO_PIN_1);
//	SendData(AVOVE, Packet, idx);
//	SendData(0, Packet, idx); // Zigbee
//	SendData(1, Packet, idx); // motor
//	SendData(2, Packet, idx); // sensor
//	SendData(3, Packet, idx); // above
	SendData(1, Packet, idx); // motor
    gpio_bit_reset(GPIOA, GPIO_PIN_1);
    
    AboveRxCnt = 0;
	memset(AboveRxData, 0, sizeof(AboveRxData));
    
    AboveRxend = 0;
    while(AboveRxend == 0);
    AboveRxend = 0;
    AboveRxstart = 0;
    memcpy(pAboveRxData->Serial, AboveRxData, AboveRxCnt);
    
	NumofItems = 0;
	ret = ReceiveData(AboveRxData, &revLen);
	if((ret == 0)&&(revLen > 0)) 
	{
		revPktId = AboveRxData[1];
		PktLen = AboveRxData[2];
		NumofItems = AboveRxData[3];
		pItem = &(AboveRxData[4]);

		for (i = 0; i<NumofItems; i++) 
		{
			switch(pItem[0]) 
			{
			case ITEM_SERIAL1:
				if(pItem[1] == FAU_LEN_SR) 
				{
					//ABOV data processing
					memset(pRxData->Serial, '\0', ABOV_SERIAL_LENGTH);
					strcpy(pRxData->Serial, "9876543210");
					for(index = 0; index < FAU_LEN_SR; index++) 
					{
						pRxData->Serial[index] = pItem[2+index];
					}
				}
				break;
				
			case ITEM_SW_VERSION: /* item type: version */
				if(pRxData->Ver != pItem[3]) 
					{
					pRxData->Ver = pItem[3];
				}
				break;

			case ITEM_FAN_STATE:
				if (revPktId > PktId) 
				{
					pRxData->Power = pItem[2];
					pRxData->FanLevel = pItem[3];

//					if(pItem[5] == 4) 
//					{
//						pRxData->Mode = OP_MODE_NORMAL;
//					}
					if(pItem[5] == 5) 
					{ 
						pRxData->Mode = OP_MODE_AUTO;
					}
					else if(pItem[5] == 0) 
					{
						pRxData->Mode = OP_MODE_OFF;
					}					

				}
				break;
				
			case ITEM_RPM:
				pRxData->PPS  = ((pItem[2]<<8) | pItem[3]); /* RPM of motor */
				break;
			
			case ITEM_FILTER:
				pRxData->FltTmrRst = pItem[2]; /* filter time reset */
				pRxData->FltTmr = ((pItem[3]<<8) | pItem[4]); /* filter time used */
				pRxData->FltTmrLmt = ((pItem[5]<<8) | pItem[6]); /* filter time used */
				break;
				
			case ITEM_VSP1:
				pRxData->VSP[0] = pItem[2];
				pRxData->VSP[1] = pItem[3];
				pRxData->VSP[2] = pItem[4];
				pRxData->VSP[3] = pItem[5];
				pRxData->VSP[4] = pItem[6];
				pRxData->VSPOffset = pItem[7];
				break;
				
			case ITEM_ERROR:
				if(pItem[2] & MOTOR_ERR)
				{
					pRxData->Err |= MOTOR_ERR;
				}
				else
				{
					pRxData->Err &= ~(MOTOR_ERR);
				}
				break;
			default:
				break;
			}
		
			pItem = pItem + (pItem[1]+2);
		}

		PktId = revPktId;
		
	}
	
	return ret;
}

///------------------------------------------------------
void AbovCommTask() 
{
    CommandToAbov(pAboveTxData, pAboveRxData);
}

void InitAbovComms(void) 
{

    pAboveTxData = NULL;
    pAboveRxData = NULL;
	GetAbovTxInfo(&pAboveTxData);
	GetAbovRxInfo(&pAboveRxData);
    
    if((pAboveTxData == NULL) || (pAboveRxData == NULL))
	{
		return;
	}
}


