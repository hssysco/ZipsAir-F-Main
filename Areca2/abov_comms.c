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


AboveTxInfoT *pAboveTxData = NULL;
AboveRxInfoT *pAboveRxData = NULL;

uint8_t AboveRxstart = 0;
uint8_t AboveRxend = 0;
uint8_t AboveRxData[100];
uint8_t AboveRxCnt = 0;
extern uint16_t     Uart_RxTime;
extern void delay_1ms(uint32_t count);

    
extern void delay_1ms(uint32_t count);


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

void Abov_Tx() 
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
	Packet[idx++] = pAboveTxData->FanLevel;		/* fau level */
	Packet[idx++] = pAboveTxData->Mode;		/* fau level */


    NumofItems++;
	Packet[idx++] = ITEM_RPM;           /* item type: RPM */
	Packet[idx++] = FAU_LEN_RPM;        /* Item length*/
 	Packet[idx++] = (uint8_t)(pAboveTxData->RPMSet>>8);               /* PPS High*/
	Packet[idx++] = (uint8_t)(pAboveTxData->RPMSet&0xff);               /* PPS Low*/
   
    NumofItems++;
	Packet[idx++] = ITEM_TIMER;           /* item type: Timer */
	Packet[idx++] = FAU_LEN_TIMER;        /* Item length*/
 	Packet[idx++] = 0;               /* PPS High*/
	Packet[idx++] = 0;               /* PPS Low*/
    

    /* item type: Filter */
    NumofItems++;
	Packet[idx++] = ITEM_FILTER; 
	Packet[idx++] = FAU_LEN_FILTER; /* item length */
	Packet[idx++] = pAboveTxData->FltTmrRst; /* filter time reset */
	pAboveTxData->FltTmrRst = 0;    
	Packet[idx++] = (uint8_t)(pAboveTxData->FltTmr>>8); /* filter time used H */
	Packet[idx++] = (uint8_t)(pAboveTxData->FltTmr&0xff); /* filter time used L */
	Packet[idx++] = (pAboveTxData->FltTmrLmt>>8)&0xFF; /* filter life time H */
	Packet[idx++] = (pAboveTxData->FltTmrLmt)&0xFF; /* filter life time L */
	
	if (NumofItems > 2) 
	{
        NumofItems++;
		Packet[idx++] = ITEM_VSP;           /* item type: VSP */
		Packet[idx++] = FAU_LEN_VSP;        /* item length */
		Packet[idx++] = pAboveTxData->VSP[0];    /* vsp1 */
		Packet[idx++] = pAboveTxData->VSP[1];    /* vsp2 */
		Packet[idx++] = pAboveTxData->VSP[2];    /* vsp3 */
		Packet[idx++] = pAboveTxData->VSP[3];    /* vsp4 */
		Packet[idx++] = pAboveTxData->VSP[4];    /* vsp5 */
        Packet[idx++] = pAboveTxData->VSPOffset;     /* VSPOffset */
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

    gpio_bit_set(GPIOA, GPIO_PIN_1);
    delay_1ms(2);
    SerialWrite(1, &Packet[0], idx);
    delay_1ms(2);
    gpio_bit_reset(GPIOA, GPIO_PIN_1);
}

int Abov_Rx() {
    int ret = 0;
    uint8_t NumofItems = 0, revLen = 0,  i = 0;
	uint8_t *pItem = NULL;


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
		NumofItems = AboveRxData[3];
		pItem = &(AboveRxData[4]);

		for (i = 0; i<NumofItems; i++) 
		{
			switch(pItem[0]) 
			{
            case ITEM_SW_VERSION: /* item type: version */
				if(pAboveRxData->VerH != pItem[2]) 
					{
					pAboveRxData->VerH = pItem[2];
				}
				if(pAboveRxData->VerL != pItem[3]) 
					{
					pAboveRxData->VerL = pItem[3];
				}
				break;

			case ITEM_FAN_STATE:
                if(pItem[2] > 0)
                    pAboveRxData->Power = 1;
                else 
                    pAboveRxData->Power = 0;
                    
                pAboveRxData->FanLevel = pItem[2];
                pAboveRxData->ErvLevel = 0;

                if(pItem[3] == 0) 
                {
                    pAboveRxData->Mode = OP_MODE_OFF;
                }					
                else 
                { 
                    pAboveRxData->Mode = OP_MODE_AUTO;
                }
				break;
				
			case ITEM_RPM:
				pAboveRxData->PPS  = ((pItem[2]<<8) | pItem[3]); /* RPM of motor */
				break;
			
			case ITEM_FILTER:
				pAboveRxData->FltTmrRst = pItem[2]; /* filter time reset */
				pAboveRxData->FltTmr = ((pItem[3]<<8) | pItem[4]); /* filter time used */
				pAboveRxData->FltTmrLmt = ((pItem[5]<<8) | pItem[6]); /* filter time used */
				break;
				
			case ITEM_VSP:
				pAboveRxData->VSP[0] = pItem[2];
				pAboveRxData->VSP[1] = pItem[3];
				pAboveRxData->VSP[2] = pItem[4];
				pAboveRxData->VSP[3] = pItem[5];
				pAboveRxData->VSP[4] = pItem[6];
				pAboveRxData->VSPOffset = pItem[7];
				break;
				
			case ITEM_ERROR:
				if(pItem[2] & MOTOR_ERR)
				{
					pAboveRxData->Err |= MOTOR_ERR;
				}
				else
				{
					pAboveRxData->Err &= ~(MOTOR_ERR);
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


