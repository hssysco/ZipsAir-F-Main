#include <stdio.h>
#include <stddef.h>
#include <string.h>

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
#define FAU_LEN_PPS				6
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


AboveTxInfoT *pZigbeeTxData = NULL;
AboveRxInfoT *pZigbeeRxData = NULL;

uint8_t ZigbeeRxstart = 0;
uint8_t ZigbeeRxend = 0;
uint8_t ZigbeeRxData[100];
uint8_t ZigbeeRxCnt = 0;

extern uint16_t     Uart_RxTime;
extern SystemInfoT  SystemInfo;

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

	*pDataLen = ZigbeeRxCnt;

	return 0;
}

void Zigbee_CodiTx() 
{
    uint8_t Packet[FAU_RX_PACKET_SIZE];

	int idx = 0, readsize = 0;
    uint8_t revPktId = 0;
	uint8_t PktLen = 0, NumofItems = 0;
	uint8_t PktId;
	
	uint16_t chksum = 0;

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
	Packet[idx++] = ITEM_FAN_STATE;         /* item type: Fan State */
	Packet[idx++] = FAU_LEN_FAN_STATE;      /* item length */
//	Packet[idx++] = 3;		/* fau level */
	Packet[idx++] = pZigbeeTxData->FanLevel;		/* fau level */
	Packet[idx++] = pZigbeeTxData->Mode;		/* fau level */


    NumofItems++;
	Packet[idx++] = ITEM_RPM;           /* item type: RPM */
	Packet[idx++] = FAU_LEN_RPM;        /* Item length*/
 	Packet[idx++] = (uint8_t)(pZigbeeTxData->RPMSet>>8);               /* PPS High*/
	Packet[idx++] = (uint8_t)(pZigbeeTxData->RPMSet&0xff);               /* PPS Low*/
   
    NumofItems++;
	Packet[idx++] = ITEM_TIMER;           /* item type: Timer */
	Packet[idx++] = FAU_LEN_TIMER;        /* Item length*/
 	Packet[idx++] = 0;               /* PPS High*/
	Packet[idx++] = 0;               /* PPS Low*/
    

    /* item type: Filter */
    NumofItems++;
	Packet[idx++] = ITEM_FILTER; 
	Packet[idx++] = FAU_LEN_FILTER; /* item length */
	Packet[idx++] = pZigbeeTxData->FltTmrRst; /* filter time reset */
	pZigbeeTxData->FltTmrRst = 0;    
	Packet[idx++] = (uint8_t)(pZigbeeTxData->FltTmr>>8); /* filter time used H */
	Packet[idx++] = (uint8_t)(pZigbeeTxData->FltTmr&0xff); /* filter time used L */
	Packet[idx++] = (pZigbeeTxData->FltTmrLmt>>8)&0xFF; /* filter life time H */
	Packet[idx++] = (pZigbeeTxData->FltTmrLmt)&0xFF; /* filter life time L */
	
	if (NumofItems > 2) 
	{
        NumofItems++;
		Packet[idx++] = ITEM_VSP;           /* item type: VSP */
		Packet[idx++] = FAU_LEN_VSP;        /* item length */
		Packet[idx++] = pZigbeeTxData->VSP[0];    /* vsp1 */
		Packet[idx++] = pZigbeeTxData->VSP[1];    /* vsp2 */
		Packet[idx++] = pZigbeeTxData->VSP[2];    /* vsp3 */
		Packet[idx++] = pZigbeeTxData->VSP[3];    /* vsp4 */
		Packet[idx++] = pZigbeeTxData->VSP[4];    /* vsp5 */
        Packet[idx++] = pZigbeeTxData->VSPOffset;     /* VSPOffset */
	}
    
    
    NumofItems++;
 	Packet[idx++] = ITEM_PPS; 
	Packet[idx++] = FAU_LEN_PPS;    /* item length */
 	Packet[idx++] = 0x5f;              /* bit0-Motor, bit1-Door, 2-Filter */
    Packet[idx++] = 0x64;              /* bit0-Motor, bit1-Door, 2-Filter */
    Packet[idx++] = 0x7c;              /* bit0-Motor, bit1-Door, 2-Filter */
    Packet[idx++] = 0xAE;              /* bit0-Motor, bit1-Door, 2-Filter */
    Packet[idx++] = 0xc8;              /* bit0-Motor, bit1-Door, 2-Filter */
    Packet[idx++] = 0x00;              /* bit0-Motor, bit1-Door, 2-Filter */
	
    NumofItems++;
 	Packet[idx++] = ITEM_ERROR; 
	Packet[idx++] = FAU_LEN_ERR;    /* item length */
 	Packet[idx++] = 0;              /* bit0-Motor, bit1-Door, 2-Filter */
          
	readsize = idx;
    Packet[2] = idx - 3;    //  packet length
    Packet[3] = NumofItems;    //  packet length
    
	chksum = CRC16Checksum(Packet, readsize);
	Packet[idx++] = chksum&0xFF;
	Packet[idx++] = (chksum>>8)&0xFF;
	Packet[idx++] = PACKET_FAU_TAIL;

    SerialWrite(0, &Packet[0], idx);
}

void Zigbee_RouterTx() 
{
    uint8_t Packet[FAU_RX_PACKET_SIZE];

	int idx = 0, readsize = 0;
    uint8_t revPktId = 0;
	uint8_t PktLen = 0, NumofItems = 0;
	uint8_t PktId;
	
	uint16_t chksum = 0;

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
	Packet[idx++] = ITEM_FAN_STATE;         /* item type: Fan State */
	Packet[idx++] = FAU_LEN_FAN_STATE;      /* item length */
//	Packet[idx++] = 3;		/* fau level */
	Packet[idx++] = pZigbeeTxData->FanLevel;		/* fau level */
	Packet[idx++] = pZigbeeTxData->Mode;		/* fau level */


    NumofItems++;
	Packet[idx++] = ITEM_RPM;           /* item type: RPM */
	Packet[idx++] = FAU_LEN_RPM;        /* Item length*/
 	Packet[idx++] = (uint8_t)(pZigbeeTxData->RPMSet>>8);               /* PPS High*/
	Packet[idx++] = (uint8_t)(pZigbeeTxData->RPMSet&0xff);               /* PPS Low*/
   
    NumofItems++;
	Packet[idx++] = ITEM_TIMER;           /* item type: Timer */
	Packet[idx++] = FAU_LEN_TIMER;        /* Item length*/
 	Packet[idx++] = 0;               /* PPS High*/
	Packet[idx++] = 0;               /* PPS Low*/
    

    /* item type: Filter */
    NumofItems++;
	Packet[idx++] = ITEM_FILTER; 
	Packet[idx++] = FAU_LEN_FILTER; /* item length */
	Packet[idx++] = pZigbeeTxData->FltTmrRst; /* filter time reset */
	pZigbeeTxData->FltTmrRst = 0;    
	Packet[idx++] = (uint8_t)(pZigbeeTxData->FltTmr>>8); /* filter time used H */
	Packet[idx++] = (uint8_t)(pZigbeeTxData->FltTmr&0xff); /* filter time used L */
	Packet[idx++] = (pZigbeeTxData->FltTmrLmt>>8)&0xFF; /* filter life time H */
	Packet[idx++] = (pZigbeeTxData->FltTmrLmt)&0xFF; /* filter life time L */
	
	if (NumofItems > 2) 
	{
        NumofItems++;
		Packet[idx++] = ITEM_VSP;           /* item type: VSP */
		Packet[idx++] = FAU_LEN_VSP;        /* item length */
		Packet[idx++] = pZigbeeTxData->VSP[0];    /* vsp1 */
		Packet[idx++] = pZigbeeTxData->VSP[1];    /* vsp2 */
		Packet[idx++] = pZigbeeTxData->VSP[2];    /* vsp3 */
		Packet[idx++] = pZigbeeTxData->VSP[3];    /* vsp4 */
		Packet[idx++] = pZigbeeTxData->VSP[4];    /* vsp5 */
        Packet[idx++] = pZigbeeTxData->VSPOffset;     /* VSPOffset */
	}
    
    
    NumofItems++;
 	Packet[idx++] = ITEM_PPS; 
	Packet[idx++] = FAU_LEN_PPS;    /* item length */
 	Packet[idx++] = 0x5f;              /* bit0-Motor, bit1-Door, 2-Filter */
    Packet[idx++] = 0x64;              /* bit0-Motor, bit1-Door, 2-Filter */
    Packet[idx++] = 0x7c;              /* bit0-Motor, bit1-Door, 2-Filter */
    Packet[idx++] = 0xAE;              /* bit0-Motor, bit1-Door, 2-Filter */
    Packet[idx++] = 0xc8;              /* bit0-Motor, bit1-Door, 2-Filter */
    Packet[idx++] = 0x00;              /* bit0-Motor, bit1-Door, 2-Filter */
	
    NumofItems++;
 	Packet[idx++] = ITEM_ERROR; 
	Packet[idx++] = FAU_LEN_ERR;    /* item length */
 	Packet[idx++] = 0;              /* bit0-Motor, bit1-Door, 2-Filter */
          
	readsize = idx;
    Packet[2] = idx - 3;    //  packet length
    Packet[3] = NumofItems;    //  packet length
    
	chksum = CRC16Checksum(Packet, readsize);
	Packet[idx++] = chksum&0xFF;
	Packet[idx++] = (chksum>>8)&0xFF;
	Packet[idx++] = PACKET_FAU_TAIL;

    SerialWrite(0, &Packet[0], idx);
}


int Zigbee_CodiRx() {
    int ret = 0;
    uint8_t NumofItems = 0, revLen = 0,  i = 0;
	uint8_t *pItem = NULL;


	memset(ZigbeeRxData, 0, sizeof(ZigbeeRxData));
    
    ZigbeeRxend = 0;
    ZigbeeRxstart = 0;
    ZigbeeRxCnt = 0;
    Uart_RxTime = 0;
    while(ZigbeeRxend == 0) {
        if(Uart_RxTime > 200) {
            return -1;
        }
    }
    ZigbeeRxend = 0;
    ZigbeeRxstart = 0;
    memcpy(pZigbeeRxData->Serial, ZigbeeRxData, ZigbeeRxCnt);
    
	NumofItems = 0;
    revLen = ZigbeeRxCnt;
	ret = ReceiveData(ZigbeeRxData, &revLen);
	if((ret == 0)&&(revLen > 0)) 
	{
		NumofItems = ZigbeeRxData[3];
		pItem = &(ZigbeeRxData[4]);

		for (i = 0; i<NumofItems; i++) 
		{
			switch(pItem[0]) 
			{
            case ITEM_SW_VERSION: /* item type: version */
				if(pZigbeeRxData->VerH != pItem[2]) 
					{
					pZigbeeRxData->VerH = pItem[2];
				}
				if(pZigbeeRxData->VerL != pItem[3]) 
					{
					pZigbeeRxData->VerL = pItem[3];
				}
				break;

			case ITEM_FAN_STATE:
                if(pItem[2] > 0)
                    pZigbeeRxData->Power = 1;
                else 
                    pZigbeeRxData->Power = 0;
                    
                pZigbeeRxData->FanLevel = pItem[2];
                pZigbeeRxData->ErvLevel = 0;

                if(pItem[3] == 0) 
                {
                    pZigbeeRxData->Mode = OP_MODE_OFF;
                }					
                else 
                { 
                    pZigbeeRxData->Mode = OP_MODE_AUTO;
                }
				break;
				
			case ITEM_RPM:
				pZigbeeRxData->PPS  = ((pItem[2]<<8) | pItem[3]); /* RPM of motor */
				break;
			
			case ITEM_FILTER:
				pZigbeeRxData->FltTmrRst = pItem[2]; /* filter time reset */
				pZigbeeRxData->FltTmr = ((pItem[3]<<8) | pItem[4]); /* filter time used */
				pZigbeeRxData->FltTmrLmt = ((pItem[5]<<8) | pItem[6]); /* filter time used */
				break;
				
			case ITEM_VSP:
				pZigbeeRxData->VSP[0] = pItem[2];
				pZigbeeRxData->VSP[1] = pItem[3];
				pZigbeeRxData->VSP[2] = pItem[4];
				pZigbeeRxData->VSP[3] = pItem[5];
				pZigbeeRxData->VSP[4] = pItem[6];
				pZigbeeRxData->VSPOffset = pItem[7];
				break;
				
			case ITEM_ERROR:
				if(pItem[2] & MOTOR_ERR)
				{
					pZigbeeRxData->Err |= MOTOR_ERR;
				}
				else
				{
					pZigbeeRxData->Err &= ~(MOTOR_ERR);
				}
				break;
			default:
				break;
			}
		
			pItem = pItem + (pItem[1]+2);
		}	
	}
	
	return ret;
}

int Zigbee_RouterRx() {
    int ret = 0;
    uint8_t NumofItems = 0, revLen = 0,  i = 0;
	uint8_t *pItem = NULL;


	memset(ZigbeeRxData, 0, sizeof(ZigbeeRxData));
    
    ZigbeeRxend = 0;
    ZigbeeRxstart = 0;
    ZigbeeRxCnt = 0;
    Uart_RxTime = 0;
    while(ZigbeeRxend == 0) {
        if(Uart_RxTime > 200) {
            return -1;
        }
    }
    ZigbeeRxend = 0;
    ZigbeeRxstart = 0;
    memcpy(pZigbeeRxData->Serial, ZigbeeRxData, ZigbeeRxCnt);
    
	NumofItems = 0;
    revLen = ZigbeeRxCnt;
	ret = ReceiveData(ZigbeeRxData, &revLen);
	if((ret == 0)&&(revLen > 0)) 
	{
		NumofItems = ZigbeeRxData[3];
		pItem = &(ZigbeeRxData[4]);

		for (i = 0; i<NumofItems; i++) 
		{
			switch(pItem[0]) 
			{
            case ITEM_SW_VERSION: /* item type: version */
				if(pZigbeeRxData->VerH != pItem[2]) 
					{
					pZigbeeRxData->VerH = pItem[2];
				}
				if(pZigbeeRxData->VerL != pItem[3]) 
					{
					pZigbeeRxData->VerL = pItem[3];
				}
				break;

			case ITEM_FAN_STATE:
                if(pItem[2] > 0)
                    pZigbeeRxData->Power = 1;
                else 
                    pZigbeeRxData->Power = 0;
                    
                pZigbeeRxData->FanLevel = pItem[2];
                pZigbeeRxData->ErvLevel = 0;

                if(pItem[3] == 0) 
                {
                    pZigbeeRxData->Mode = OP_MODE_OFF;
                }					
                else 
                { 
                    pZigbeeRxData->Mode = OP_MODE_AUTO;
                }
				break;
				
			case ITEM_RPM:
				pZigbeeRxData->PPS  = ((pItem[2]<<8) | pItem[3]); /* RPM of motor */
				break;
			
			case ITEM_FILTER:
				pZigbeeRxData->FltTmrRst = pItem[2]; /* filter time reset */
				pZigbeeRxData->FltTmr = ((pItem[3]<<8) | pItem[4]); /* filter time used */
				pZigbeeRxData->FltTmrLmt = ((pItem[5]<<8) | pItem[6]); /* filter time used */
				break;
				
			case ITEM_VSP:
				pZigbeeRxData->VSP[0] = pItem[2];
				pZigbeeRxData->VSP[1] = pItem[3];
				pZigbeeRxData->VSP[2] = pItem[4];
				pZigbeeRxData->VSP[3] = pItem[5];
				pZigbeeRxData->VSP[4] = pItem[6];
				pZigbeeRxData->VSPOffset = pItem[7];
				break;
				
			case ITEM_ERROR:
				if(pItem[2] & MOTOR_ERR)
				{
					pZigbeeRxData->Err |= MOTOR_ERR;
				}
				else
				{
					pZigbeeRxData->Err &= ~(MOTOR_ERR);
				}
				break;
			default:
				break;
			}
		
			pItem = pItem + (pItem[1]+2);
		}	
	}
	
	return ret;
}




void InitZigbeeComms(void) 
{

    pZigbeeTxData = NULL;
    pZigbeeRxData = NULL;
	GetAbovTxInfo(&pZigbeeTxData);
	GetAbovRxInfo(&pZigbeeRxData);
    
    if((pZigbeeTxData == NULL) || (pZigbeeRxData == NULL))
	{
		return;
	}
}


