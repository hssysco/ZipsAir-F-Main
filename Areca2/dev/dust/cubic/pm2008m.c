#include "gd32f30x.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <time.h>
//#include <sys/time.h>
//#include <unistd.h>

//#include "freertos/FreeRTOS.h"
//#include "freertos/task.h"
//#include "driver/gpio.h"

#include "serial.h"
#include "dust.h"

#define MAX_PACKET_LEN    	56


void Pm2008Task(void* arg) 
{
	int idx = 0, rc = 0;
	unsigned char buf[MAX_PACKET_LEN];
	unsigned char checksum;
	unsigned char calcurated_checksum;

	unsigned char *df = NULL;

	unsigned long pm_1_0;
	unsigned long pm_2_5;
	unsigned long pm_10_0;

	DustEventT param;
	
	char StrBuf[256];
	int Fd = -1;

	DrvDustDevT *pDustDev = (DrvDustDevT *)arg;
	if(pDustDev == NULL)
	{
		return;
	}

//	while(1) 
//	{
		idx = checksum = 0;
		buf[idx++] = 0x11;
		buf[idx++] = 0x02;
		buf[idx++] = 0x0B;
		buf[idx++] = 0x07;
		buf[idx++] = 0xDB;
		rc = SerialWrite(2, buf, idx);
//		rc = SerialWrite(DUST, buf, idx);

		memset(buf, 0, sizeof(buf));
//		rc = SerialRead(2, buf, MAX_PACKET_LEN);
//		rc = SerialRead(DUST, buf, MAX_PACKET_LEN);
		if (rc <= 0) 
		{
			goto cont;
		}

		checksum = buf[MAX_PACKET_LEN-1];
		for(calcurated_checksum=0,idx=0; idx<MAX_PACKET_LEN-1; idx++)
		{
			calcurated_checksum += buf[idx];
		}
		calcurated_checksum = 256 - calcurated_checksum;

		if (checksum != calcurated_checksum)
		{
			goto cont;
		}

		df = &buf[3];
		if(df)
		{
			/* Adjustment */
			switch(pDustDev->mAdjMode) 
			{
				case ADJ_MODE_TSI:
					param.pm_1_0 = (df[12]<<24) | (df[13]<<16) | (df[14]<<8) | df[15];
					param.pm_2_5 = (df[16]<<24) | (df[17]<<16) | (df[18]<<8) | df[19];
					param.pm_10_0 = (df[20]<<24) | (df[21]<<16) | (df[22]<<8) | df[23];
					printf("Pm2008Task: 2 (%ld: %ld: %ld) \r\n", param.pm_1_0, param.pm_2_5, param.pm_10_0);
					break;
				case ADJ_MODE_GRIMM:
				default:
					param.pm_1_0 = (df[0]<<24) | (df[1]<<16) | (df[2]<<8) | df[3];
					param.pm_2_5 = (df[4]<<24) | (df[5]<<16) | (df[6]<<8) | df[7];
					param.pm_10_0 = (df[8]<<24) | (df[9]<<16) | (df[10]<<8) | df[11];
					printf("Pm2008Task: 3 (%ld: %ld: %ld) \r\n", param.pm_1_0, param.pm_2_5, param.pm_10_0);
					break;
			}
		}

		if (pDustDev->fnDustCallback) 
		{
			pDustDev->fnDustCallback(&param);
		}

cont:
		printf("## Dust %d: ScanPeriod(%ld)\n", __LINE__, pDustDev->ScanPeriod);
//		usleep(pDustDev->ScanPeriod*1000);
//		continue;
//	}

//	return;

}

void InitDust (DrvDustDevT *pDustDev) 
{
	if(pDustDev == NULL) 
	{
		return;
	}
    rcu_periph_clock_enable(RCU_AF);
    rcu_periph_clock_enable(SEN_GPIO_PORT);
//    gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_4);
//    gpio_bit_reset(GPIOB, GPIO_PIN_4);
//    gpio_pin_remap_config(GPIO_SWJ_NONJTRST_REMAP, ENABLE);    

    rcu_periph_clock_enable(SEN_GPIO_CLK);
    /* configure led GPIO port */ 
    gpio_init(SEN_GPIO_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ,SEN_PIN);

    gpio_bit_set(SEN_GPIO_PORT, SEN_PIN);
    gpio_pin_remap_config(GPIO_SWJ_SWDPENABLE_REMAP, ENABLE); 
//    GPIO_BOP(SEN_GPIO_PORT) = SEN_PIN;
}


