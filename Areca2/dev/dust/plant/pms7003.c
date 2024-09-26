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

#include "serial.h"
#include "dust.h"

#define MAX_HOST_PROTOCOL_LENGTH    7
#define MAX_CLIENT_PROTOCOL_LENGTH    32


void Pms7003Task(void* arg) 
{
	int i = 0, rc = 0;
	unsigned char buf[MAX_CLIENT_PROTOCOL_LENGTH];
	unsigned short checksum;
	unsigned short calcurated_checksum;

	unsigned long pm_1_0;
	unsigned long pm_2_5;
	unsigned long pm_10_0;

	DustEventT param;
	

	DrvDustDevT *pDustDev = (DrvDustDevT *)arg;
	if(pDustDev == NULL)
	{
		return;
	}

	checksum = 0;
	buf[0] = 0x42;
	buf[1] = 0x4d;
	buf[2] = 0xe1;
	buf[3] = 0x00;
	buf[4] = 0x00;
	for(i=0;i<MAX_HOST_PROTOCOL_LENGTH-2;i++) 
	{
		checksum += buf[i];
	}
	buf[5] = checksum>>8;
	buf[6] = checksum & 0x00ff;

	rc = SerialWrite(DUST, buf, MAX_HOST_PROTOCOL_LENGTH);

	while(1) 
	{
		checksum = 0;
		buf[0] = 0x42;
		buf[1] = 0x4d;
		buf[2] = 0xe2;
		buf[3] = 0x00;
		buf[4] = 0x00;
		for(i=0;i<MAX_HOST_PROTOCOL_LENGTH-2;i++) 
		{
			checksum += buf[i];
		}
		buf[5] = checksum>>8;
		buf[6] = checksum & 0x00ff;
		rc = SerialWrite(DUST, buf, MAX_HOST_PROTOCOL_LENGTH);
	
		memset(buf, 0, sizeof(buf));
		rc = SerialRead(DUST, buf, MAX_CLIENT_PROTOCOL_LENGTH);
		if (rc<=0) goto cont;

		checksum = buf[MAX_CLIENT_PROTOCOL_LENGTH-2]<<8 | buf[MAX_CLIENT_PROTOCOL_LENGTH-1];
		for(calcurated_checksum=0,i=0; i<MAX_CLIENT_PROTOCOL_LENGTH-2; i++) 
		{
			calcurated_checksum += buf[i];
		}

		if (checksum!=calcurated_checksum) 
		{
			goto cont;
		}
			
		pm_1_0 = (buf[4] << 8) + buf[5];
		pm_2_5 = (buf[6] << 8) + buf[7];
		pm_10_0 = (buf[8] << 8) + buf[9];

		/* Adjustment */
		switch(pDustDev->mAdjMode) 
		{
			case ADJ_MODE_RAW:
				break;
			case ADJ_MODE_GRIMM:
				param.pm_1_0 = (pm_1_0<10) ? pm_1_0 : (unsigned long)(((float)pm_1_0 * 1.7498) -9.1964);
				param.pm_2_5 = (pm_2_5<10) ? pm_2_5 : (unsigned long)(((float)pm_2_5 * 1.9436) -7.3548);
				param.pm_10_0 = (pm_10_0<32) ? pm_10_0 : (unsigned long)(((float)pm_10_0 * 3.37) -47.9);
				break;
			default:
				break;
		}

		if (pDustDev->fnDustCallback) 
		{
			pDustDev->fnDustCallback(&param);
		}

cont:
//		printf("## Dust %d: ScanPeriod(%ld)\n", __LINE__, pDustDev->ScanPeriod);
		usleep(pDustDev->ScanPeriod*1000);
		continue;
	}

	return;

}

void InitDust (DrvDustDevT *pDustDev) 
{

	if(pDustDev == NULL) 
	{
		return;
	}

	xTaskCreatePinnedToCore(Pms7003Task, "Dust Task", 4096, (void *)pDustDev, 5, NULL, 1);

	return;
}


