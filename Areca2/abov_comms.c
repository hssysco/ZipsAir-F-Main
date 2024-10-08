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
//#define FAU_TX_PACKET_LENGTH	22 //52
#define FAU_TX_PACKET_LENGTH	34 //52
#define FAU_RX_PACKET_SIZE		64
#define FAU_LEN_VER				2
#define FAU_LEN_SR				10
//#define FAU_LEN_FAN_STATE		4
#define FAU_LEN_FAN_STATE		2
#define FAU_LEN_RPM				2
#define FAU_LEN_DIFF_PRESSURE	2
#define FAU_LEN_TIMER			2
#define FAU_LEN_FILTER			5
#define FAU_LEN_VSP			    6
#define FAU_LEN_ERR				1
#define FAU_LEN_TETSMOD			2

#define MOTOR_ERR				0x01

enum FAUItem 
{
//	ITEM_SERIAL1 = 1,
//	ITEM_SERIAL2 = 2, // not used
	ITEM_SW_VERSION = 3,
	ITEM_FAN_STATE = 4,
	ITEM_RPM = 5,
	ITEM_TIMER = 6, // not used
	ITEM_FILTER = 7,
	ITEM_VSP = 8,
	ITEM_PPS = 9, // not used
//	ITEM_VSP3 = 10, // not used
//	ITEM_VSP4 = 11, // not used
//	ITEM_DIFF_PRESSURE = 12, // not used
	ITEM_ERROR = 99,
	ITEM_TEST_MODE = 200, // not used
};


AboveTxInfoT *pAboveTxData = NULL;
AboveRxInfoT *pAboveRxData = NULL;

uint8_t AboveRxstart = 0;
uint8_t AboveRxend = 0;
uint8_t AboveRxData[100];
uint8_t AboveRxCnt = 0;
extern uint16_t     Uart_RxTime;

    
extern void delay_1ms(uint32_t count);
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

    packet_size = *pDataLen;
    read_data = packet_size -3;

        
	checksum = CRC16Checksum(pData, read_data);
	checksumC = pData[packet_size-2];
	checksumC <<= 8;
	checksumC = checksumC + pData[packet_size-3];

    if(checksum != checksumC) { 
        *pDataLen = 0;
        return -1;
    }

	*pDataLen = AboveRxCnt;

	return 0;
}

uint8_t revPktId = 0;
int CommandToAbov( AboveTxInfoT *pTxData,  AboveRxInfoT *pRxData ) 
{

	int ret = 0, idx = 0, readsize = 0;
	uint8_t Packet[FAU_RX_PACKET_SIZE];
	uint8_t PktLen = 0, NumofItems = 0, revLen = 0,  i = 0;
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


    PktId = (revPktId++);
    PktLen = 0;
    NumofItems = 0;

	Packet[idx++] = PACKET_FAU_HEADER;
	Packet[idx++] = PktId;
	Packet[idx++] = PktLen;
	Packet[idx++] = NumofItems;
    
    NumofItems++;
    Packet[idx++] = 0x03;                   // Item Version
    Packet[idx++] = 0x02;                   // Item length
    Packet[idx++] = 0x0D;                    // 'a' : Main SW, 't' : test version
    Packet[idx++] = 0x0D;                   // Version 1.3

    
    NumofItems++;
	Packet[idx++] = ITEM_FAN_STATE;         /* item type: Fan State */
	Packet[idx++] = FAU_LEN_FAN_STATE;      /* item length */
	Packet[idx++] = 5;		/* fau level */
//	Packet[idx++] = pTxData->FanLevel;		/* fau level */

	if(pTxData->Mode == OP_MODE_AUTO)
	{
		Packet[idx++] = 1;
	}
//	else if(pTxData->Mode == OP_MODE_NORMAL)
//	{
//		Packet[idx++] = 4;
//	}		
	else 
	{
		Packet[idx++] = 0;
	}
    
    NumofItems++;
	Packet[idx++] = ITEM_RPM;           /* item type: RPM */
	Packet[idx++] = FAU_LEN_RPM;        /* Item length*/
 	Packet[idx++] = 0x02;               /* PPS High*/
	Packet[idx++] = 0xAD;               /* PPS Low*/
   
//  NumofItems++;
//	Packet[idx++] = ITEM_TIMER;         /* item type: TIMER */
//	Packet[idx++] = FAU_LEN_TIMER;      /* item type: TIMER */
// 	Packet[idx++] = 0x00;               /* TIMER High*/
//	Packet[idx++] = 0x01;               /* TIMER Low*/

    /* item type: Filter */
    NumofItems++;
	Packet[idx++] = ITEM_FILTER; 
	Packet[idx++] = FAU_LEN_FILTER; /* item length */
	Packet[idx++] = pTxData->FltTmrRst; /* filter time reset */
	pTxData->FltTmrRst = 0;    
	Packet[idx++] = 0; /* filter time used H */
	Packet[idx++] = 0; /* filter time used L */
	Packet[idx++] = (pTxData->FltTmrLmt>>8)&0xFF; /* filter life time H */
	Packet[idx++] = (pTxData->FltTmrLmt)&0xFF; /* filter life time L */
	
	if (NumofItems > 2) 
	{
        NumofItems++;
		Packet[idx++] = ITEM_VSP;           /* item type: VSP */
		Packet[idx++] = FAU_LEN_VSP;        /* item length */
//		Packet[idx++] = pTxData->VSP[0];    /* vsp1 */
//		Packet[idx++] = pTxData->VSP[1];    /* vsp2 */
//		Packet[idx++] = pTxData->VSP[2];    /* vsp3 */
//		Packet[idx++] = pTxData->VSP[3];    /* vsp4 */
//		Packet[idx++] = pTxData->VSP[4];    /* vsp5 */

		Packet[idx++] = 0x2d; /* vsp2 */
		Packet[idx++] = 0x2e; /* vsp3 */
		Packet[idx++] = 0x30; /* vsp4 */
		Packet[idx++] = 0x36; /* vsp5 */
		Packet[idx++] = 0x3e; /* vsp5 */
		Packet[idx++] = 0; // offset
	}
	
    NumofItems++;
 	Packet[idx++] = ITEM_ERROR; 
	Packet[idx++] = FAU_LEN_ERR;    /* item length */
 	Packet[idx++] = 0;              /* bit0-Motor, bit1-Door, 2-Filter */
  
// 	Packet[idx++] = ITEM_TEST_MODE; 
// 	Packet[idx++] = FAU_LEN_TETSMOD; 
//  	Packet[idx++] = 0; 
// 	Packet[idx++] = 0; 
          
	readsize = idx;
    Packet[2] = idx - 3;    //  packet length
    Packet[3] = NumofItems;    //  packet length
    
	chksum = CRC16Checksum(Packet, readsize);
	Packet[idx++] = chksum&0xFF;
	Packet[idx++] = (chksum>>8)&0xFF;
	Packet[idx++] = PACKET_FAU_TAIL;

    gpio_bit_set(GPIOA, GPIO_PIN_1);
	SendData(1, Packet, idx); // motor
    gpio_bit_reset(GPIOA, GPIO_PIN_1);

//	SendData(AVOVE, Packet, idx);
//	SendData(0, Packet, idx); // Zigbee
//	SendData(1, Packet, idx); // motor
//	SendData(2, Packet, idx); // sensor
//	SendData(3, Packet, idx); // Thermo
    
	memset(AboveRxData, 0, sizeof(AboveRxData));
    
    AboveRxend = 0;
    AboveRxstart = 0;
    AboveRxCnt = 0;
    Uart_RxTime = 0;
    while(AboveRxend == 0) {
        if(Uart_RxTime > 200) {
            return -1;
        }
    }
    AboveRxend = 0;
    AboveRxstart = 0;
    memcpy(pAboveRxData->Serial, AboveRxData, AboveRxCnt);
    
	NumofItems = 0;
    revLen = AboveRxCnt;
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
//			case ITEM_SERIAL1:
//				if(pItem[1] == FAU_LEN_SR) 
//				{
//					//ABOV data processing
//					memset(pRxData->Serial, '\0', ABOV_SERIAL_LENGTH);
//					strcpy(pRxData->Serial, "9876543210");
//					for(index = 0; index < FAU_LEN_SR; index++) 
//					{
//						pRxData->Serial[index] = pItem[2+index];
//					}
//				}
//				break;
//				
			case ITEM_SW_VERSION: /* item type: version */
				if(pRxData->VerH != pItem[2]) 
					{
					pRxData->VerH = pItem[2];
				}
				if(pRxData->VerL != pItem[3]) 
					{
					pRxData->VerL = pItem[3];
				}
				break;

			case ITEM_FAN_STATE:
				if (revPktId > PktId) 
				{
					pRxData->FanLevel = pItem[2];

					if(pItem[3] == 0) 
					{
						pRxData->Mode = OP_MODE_OFF;
					}					
					else 
					{ 
						pRxData->Mode = OP_MODE_AUTO;
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
				
			case ITEM_VSP:
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


