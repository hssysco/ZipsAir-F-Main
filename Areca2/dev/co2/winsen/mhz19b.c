#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <time.h>
//#include <sys/time.h>
//#include <unistd.h>
//
//#include "freertos/FreeRTOS.h"
//#include "freertos/task.h"

#include "co2.h"
#include "serial.h"


#define MHZ19B_CMD_LENGTH    9
#define MHZ19B_CMD_READ_CO2_CONCENTRATION    0x86
#define MHZ19B_CMD_ZERO_POINT_CALIBRATION    0x87
#define MHZ19B_CMD_SPAN_POINT_CALIBRATION    0x88
#define MHZ19B_CMD_AUTOMATIC_CALIBRATION    0x79
	
#define MHZ19B_START_BYTE    0xFF
#define MHZ19B_SENSOR_NUM    1

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
	
	for(i=1; i<(size-1); i++) 
	{
		checksum += buf[i];
	}

	checksum = 0xff - checksum;
	checksum += 1;
	return checksum;
}

void Mhz19bTask(void* arg)
{
	unsigned char buf[1024];
	unsigned char cmd[MHZ19B_CMD_LENGTH];
	int rc = 0, packet_valid = 0, co2event = 0;

	DrvCo2DevT *pCo2Dev = (DrvCo2DevT *)arg;
	if(pCo2Dev == NULL)	
	{
		return;
	}

	while(1) 
	{
		memset((void *)cmd, 0, sizeof(cmd));
		memset((void *)buf, 0, 1024);
		
		cmd[0] = MHZ19B_START_BYTE;
		cmd[1] = MHZ19B_SENSOR_NUM; 


		switch(sensor_state) 
		{
			case STATE_CALIBRATION_ZERO_POINT:
				printf("enter co2 calibration zero point mode \r\n");
				cmd[2] = (unsigned char)MHZ19B_CMD_ZERO_POINT_CALIBRATION;
				cmd[8] = getCheckSum(cmd, sizeof(cmd));
				rc = SerialWrite(CO2, cmd, MHZ19B_CMD_LENGTH);
				if (rc<0) {
					goto cont;
				}

				sleep(1200);

				sensor_state = STATE_NORMAL;
				goto cont;
				
			case STATE_CALIBRATION_SPAN_POINT:
				printf("enter co2 calibration span point mode \r\n");
				cmd[2] = (unsigned char)MHZ19B_CMD_SPAN_POINT_CALIBRATION;
				cmd[8] = getCheckSum(cmd, sizeof(cmd));
				rc = SerialWrite(CO2, cmd, MHZ19B_CMD_LENGTH);
				if ( rc<0 ) {
					goto cont;
				}

				sensor_state = STATE_NORMAL;
				goto cont;
				
			case STATE_INIT:
#if 0
				cmd[2] = (unsigned char)MHZ19B_CMD_AUTOMATIC_CALIBRATION;
				cmd[3] = 0x00;
				cmd[8] = getCheckSum(cmd, sizeof(cmd));
				ftdi_write_data(ftdi, (const unsigned char*)cmd, sizeof(cmd));
				state_init_time++;
				if (state_init_time > 5) {
					sensor_state = STATE_NORMAL;
				}
#else
				sensor_state = STATE_NORMAL;
#endif
				goto cont;

			case STATE_NORMAL:
				cmd[2] = (unsigned char)MHZ19B_CMD_READ_CO2_CONCENTRATION;
				cmd[3] = 0x00;
				cmd[8] = getCheckSum(cmd, sizeof(cmd));
				rc = SerialWrite(CO2, cmd, MHZ19B_CMD_LENGTH);
				if ( rc < 0 ) {
					goto cont;
				}

				break;

			default:
				goto cont;
		}

		rc = SerialRead(CO2, buf, 9);
		if (rc<0) goto cont;

		packet_valid = (buf[0] == 0xff && buf[1] == 0x86 && getCheckSum(buf, sizeof(cmd)) == buf[8]);
		if (packet_valid) 
		{
			co2event = buf[2] * 256 + buf[3];
		}
		else 
		{
//			printf("%d: checksum mismatched(expected:%x, real:%x) \r\n", __LINE__, getCheckSum(buf, sizeof(cmd)), buf[8]);
			co2event = 0;
		}

		if (pCo2Dev->fnCo2Callback && packet_valid) 
		{
			pCo2Dev->fnCo2Callback(co2event);
		}

//		printf("%d: co2(%x)(%ld)(%d) \r\n", __LINE__, co2event, pCo2Dev->ScanPeriod, co2event);

cont:
		usleep(pCo2Dev->ScanPeriod*1000);
		
		continue;

	}

	return;
}

void InitCo2 (DrvCo2DevT *pCo2dev) 
{
	sensor_state = STATE_INIT;

	xTaskCreatePinnedToCore(Mhz19bTask, "CO2 Task", 4096, (void *)pCo2dev, 5, NULL, 1);

	return;
}

