#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <time.h>


#include "co2.h"
#include "serial.h"


#define CM1106_MAX_CMD_LENGTH    100
	#define CM1106_CMD_READ_CO2_CONCENTRATION   0x01
	#define CM1106_CMD_ABC_PARAMETER            0x10
	#define CM1106_CMD_CALIBRATE_CON_VALUE      0x03
	#define CM1106_CMD_READ_SERIAL_NUMBER       0x1F
	#define CM1106_CMD_READ_SW_VERSION          0x1E
#define CM1106_START_BYTE    0x11
#define CM1106_ACK_START_BYTE    0x16

typedef enum 
{
	STATE_UNKNOWN = 0,
	STATE_INIT,
	STATE_NORMAL,
	STATE_CALIBRATION_ZERO_POINT,
	STATE_CALIBRATION_SPAN_POINT,
	STATE_MAX
}sensor_state_e;

static sensor_state_e sensor_state = STATE_UNKNOWN;

static unsigned char getCheckSum(unsigned char *buf, unsigned long size) 
{
	unsigned int i;
	unsigned char checksum = 0;

	for(i=0; i<size; i++)
	{
		checksum = checksum + buf[i];
	}

	checksum = 0x100 - checksum;
	return checksum;
}

void Cm1106Task(void* arg)
{
	unsigned char buf[1024];
	unsigned char cmd[CM1106_MAX_CMD_LENGTH];
	int rc = 0, packet_valid = 0, co2event = 0;

//	unsigned char cnt = 0;

	DrvCo2DevT *pCo2Dev = (DrvCo2DevT *)arg;
	if(pCo2Dev == NULL)	
	{
		return;
	}

//	while(1) 
//	{
		memset((void *)cmd, 0, sizeof(cmd));
		memset((void *)buf, 0, 1024);
		
		cmd[0] = CM1106_START_BYTE;
		cmd[1] = 0x0; //length

		switch(sensor_state) 
		{
			case STATE_CALIBRATION_SPAN_POINT:
			case STATE_CALIBRATION_ZERO_POINT:
				printf("enter co2 calibration  \r\n");
				cmd[2] = CM1106_CMD_CALIBRATE_CON_VALUE; cmd[1] += 1;
				cmd[3] = 0x01; cmd[1] += 1;
				cmd[4] = 0x90; cmd[1] += 1;
				cmd[5] = getCheckSum(cmd, 5);
				
				rc = SerialWrite(CO2, cmd,  cmd[1] + 3);
				if (rc<0) 
				{
					return;
				}

//				sleep(120);
				sensor_state = STATE_NORMAL;
				return;
				
			case STATE_INIT:
				cmd[2] = CM1106_CMD_ABC_PARAMETER; cmd[1] += 1;
				cmd[3] = 0x64; cmd[1] += 1; //Reserved
				cmd[4] = 0x02; cmd[1] += 1; //ABC Close
				cmd[5] = 0x07; cmd[1] += 1;
				cmd[6] = 0x01; cmd[1] += 1;
				cmd[7] = 0x90; cmd[1] += 1;
				cmd[8] = 0x64; cmd[1] += 1;
				cmd[9] = getCheckSum(cmd, 9);
				rc = SerialWrite(CO2, cmd,  cmd[1] + 3);
				if (rc<0) 
				{
					return;
				}

				sensor_state = STATE_NORMAL;
				return;

			case STATE_NORMAL:
				cmd[2] = CM1106_CMD_READ_CO2_CONCENTRATION; cmd[1] += 1;
				cmd[3] = getCheckSum(cmd, 3);
				
				rc = SerialWrite(CO2, cmd, cmd[1] + 3);
				if ( rc < 0 ) 
				{
					return;
				}

				break;

			default:
				return;
		}

		rc = SerialRead(CO2, buf, 8);
		if (rc<0) 
		{
			return;
		}

		printf("### CO2 1 !!! (%x: %x: %x: %x: %x: %x: %x: %x) \r\n", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);

		packet_valid = (buf[0] == CM1106_ACK_START_BYTE && buf[2] == CM1106_CMD_READ_CO2_CONCENTRATION &&
				buf[7] == getCheckSum(buf, buf[1] + 2));

		if (packet_valid)
		{
			co2event = buf[3] * 256 + buf[4];
		}
		else 
		{
			co2event = 0;
		}

		printf("### CO2 2 !!! (%d)(%x:%x)(%d) \r\n", packet_valid, buf[3], buf[4], co2event);

		if (pCo2Dev->fnCo2Callback && packet_valid) 
		{
			pCo2Dev->fnCo2Callback(co2event);
		}

		printf("%d: co2(%x)(%ld)(%d) \r\n", __LINE__, co2event, pCo2Dev->ScanPeriod, co2event);

//cont:
//		usleep(pCo2Dev->ScanPeriod*1000);
//		cnt = 0;
//		continue;
//
//	}
}



void InitCo2 (DrvCo2DevT *pCo2dev) 
{
	sensor_state = STATE_INIT;
//
//	xTaskCreatePinnedToCore(Cm1106Task, "CO2 Task", 4096, (void *)pCo2dev, 5, NULL, 1);
//
//	return;
}

