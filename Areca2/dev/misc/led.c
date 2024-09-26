#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "esp_console.h"
//#include "driver/uart.h"
//#include "driver/gpio.h"

#include "led.h"
#include "serial.h"
#include "gd32f30x.h"

#define RS485_MSG_NUM	19
#define HEADER			0x6A
#define LEDNUM			0x0d
#define ETX				0xA6 

#define BLUE_ON			0x04
#define GREEN_ON		0x02
#define RED_ON			0x01

static struct {
    struct arg_int *led_command;
    struct arg_int *led_num;
    struct arg_end *end;
}cmd_args;

typedef struct ConfigLed {
	unsigned int	ledNum;
	LedCmd_t		cmd;	/* on, off */
}tConfigLed;

tConfigLed sConfigLed[LED_STATUS_MAX];


//void SetLedGpio (gpio_num_t GpioNum ) 
//{
//
//    gpio_pad_select_gpio(GpioNum);
//    /* Set the GPIO as a push/pull output */
//    gpio_set_direction(GpioNum, GPIO_MODE_OUTPUT);
//	gpio_set_level(GpioNum, 1);
//}

void InitLed(void) 
{
	unsigned int i = 0;

	/* LED config */
	for(i = 0; i < LED_STATUS_MAX; i++) 
	{
		memset(&sConfigLed[i],0,sizeof(tConfigLed));

		sConfigLed[i].ledNum = (i - LED_DUST_BLUE);
	}
}

int OnLedRs485(unsigned int LedNum, unsigned char bgr ) 
{
	unsigned short crc16value = 0;
	unsigned char cmdbuff[RS485_MSG_NUM] = {0,};
	unsigned int dataLen = 0;	
	Led_t led = 0;
	unsigned char k = 0, index = 0;

	led = LedNum;

	memset(cmdbuff,0,RS485_MSG_NUM);

	cmdbuff[0] = HEADER;
	cmdbuff[1] = 0;

	if((led >= LED_DUST_BLUE)&&(led < LED_POWER)) 
	{
		cmdbuff[2] = 3;
		cmdbuff[3] = 0;
		cmdbuff[3] |= bgr;
		cmdbuff[4] = (1 << 4);
		cmdbuff[4] |= bgr;
		cmdbuff[5] = (2 << 4);
		cmdbuff[5] |= bgr;
		crc16value = CRC16Checksum(cmdbuff, 6);
		cmdbuff[7] = ((crc16value >> 8) & 0xff);
		cmdbuff[6] = (crc16value & 0xff);;
		cmdbuff[8] = ETX;
		dataLen = 9;
	}
	else if((led >= LED_POWER)&&(led < LED_FAN_LOW)) 
	{
		LedNum -= 1;
		cmdbuff[2] = 1;
		cmdbuff[3] = ((LedNum & 0xff) << 4);
		cmdbuff[3] |= bgr;
		crc16value = CRC16Checksum(cmdbuff,4);
		
		cmdbuff[5] = ((crc16value >> 8) & 0xff);
		cmdbuff[4] = (crc16value & 0xff);;
		cmdbuff[6] = ETX;
		dataLen = 7;

	}
	else if((led >= LED_FAN_LOW)&&(led < LED_STATUS_MAX)) 
	{
		cmdbuff[2] = 4;

		index = 9;
		for(k = 0; k < 4; k++) 
		{
			cmdbuff[3 + k] = ((index & 0xff) << 4);
			index++;
		}

		switch(led) 
		{
			case LED_FAN_LOW:
				cmdbuff[3] |= 0x01;
			break;
		
			case LED_FAN_MID:
				cmdbuff[3] |= 0x01;
				cmdbuff[4] |= 0x01;
			break;
		
			case LED_FAN_HIGH:
				cmdbuff[3] |= 0x01;
				cmdbuff[4] |= 0x01;
				cmdbuff[5] |= 0x01;
			break;
		
			case LED_FAN_TURBO:
				cmdbuff[3] |= 0x01;
				cmdbuff[4] |= 0x01;
				cmdbuff[5] |= 0x01;
				cmdbuff[6] |= 0x01;
			break;
		
			default:
			break;
		}

		crc16value = CRC16Checksum(cmdbuff, 7);

		cmdbuff[8] = ((crc16value >> 8) & 0xff);
		cmdbuff[7] = (crc16value & 0xff);;
		cmdbuff[9] = ETX;
		dataLen = 10;
	}

    gpio_bit_set(GPIOB, GPIO_PIN_4);
	SerialWrite(3, cmdbuff, dataLen);
    gpio_bit_reset(GPIOB, GPIO_PIN_4);


//	vTaskDelay(5);

	return 0;
}

int OffLedRs485(unsigned int led)
{
	unsigned short crc16value = 0;
	unsigned char cmdbuff[RS485_MSG_NUM] = {0,};
	unsigned int dataLen = 0;	
	unsigned char k = 0, index = 0;

	memset(cmdbuff,0,RS485_MSG_NUM);
	cmdbuff[0] = HEADER;
	cmdbuff[1] = 0;

	if((led >= LED_DUST_BLUE)&&(led < LED_POWER)) 
	{
		cmdbuff[2] = 3;
		cmdbuff[3] = 0;
		cmdbuff[3] &= 0xf0;
		cmdbuff[4] = (1 << 4);
		cmdbuff[4] &= 0xf0;
		cmdbuff[5] = (2 << 4);
		cmdbuff[5] &= 0xf0;
		crc16value = CRC16Checksum(cmdbuff, 6);
		cmdbuff[7] = ((crc16value >> 8) & 0xff);
		cmdbuff[6] = (crc16value & 0xff);;
		cmdbuff[8] = ETX;
		dataLen = 9;
	}
	else if((led >= LED_POWER)&&(led < LED_FAN_LOW))
	{
		cmdbuff[2] = 1;
		cmdbuff[3] = (((led-1) & 0xff) << 4);
		cmdbuff[3] &= 0xf0;
		crc16value = CRC16Checksum(cmdbuff,4);
		cmdbuff[5] = ((crc16value >> 8) & 0xff);
		cmdbuff[4] = (crc16value & 0xff);;
		cmdbuff[6] = ETX;
		dataLen = 7;
	}
	else if((led >= LED_FAN_LOW)&&(led < LED_STATUS_MAX))
	{
		cmdbuff[2] = 4;

		index = 9;
		for(k = 0; k < 4; k++) 
		{
			cmdbuff[3 + k] = ((index & 0xff) << 4);
			index++;
		}

		cmdbuff[3] &= 0xf0;
		cmdbuff[4] &= 0xf0;
		cmdbuff[5] &= 0xf0;
		cmdbuff[6] &= 0xf0;

		crc16value = CRC16Checksum(cmdbuff, 7);

		cmdbuff[8] = ((crc16value >> 8) & 0xff);
		cmdbuff[7] = (crc16value & 0xff);;
		cmdbuff[9] = ETX;
		dataLen = 10;
	}

    gpio_bit_set(GPIOB, GPIO_PIN_4);
	SerialWrite(3, cmdbuff, dataLen);
    gpio_bit_reset(GPIOB, GPIO_PIN_4);

//	vTaskDelay(5);

	return 0;
}

unsigned char GetLedColor(Led_t led) 
{
	unsigned char value = 0;
	if(led == LED_DUST_BLUE)
	{
		value = BLUE_ON;
	}
	else if(led == LED_DUST_GREEN) 
	{
		value = GREEN_ON;
	}
	else if(led == LED_DUST_DGREEN) 
	{
		value = GREEN_ON | RED_ON;
	}
	else if(led == LED_DUST_RED) 
	{
		value = RED_ON;
	}
	else 
	{
		value = LED_ON;
	}

	return value;
}

int OnLed ( Led_t led  ) 
{

	unsigned char rgbValue = 0;
	unsigned char i = 0, index = 0;
	
	if(led >= LED_STATUS_MAX) 
	{
		return -1;
	}

	if(sConfigLed[led].cmd == LED_ON) 
	{
		return 1;
	}

	rgbValue = GetLedColor(led);
	OnLedRs485(sConfigLed[led].ledNum, rgbValue);

	if((led >= LED_POWER) && (led <= LED_AI)) 
	{
		sConfigLed[led].cmd = LED_ON;
	}
	else if((led >= LED_DUST_BLUE) && (led <= LED_DUST_RED)) 
	{
		index = LED_DUST_BLUE;
		sConfigLed[led].cmd = LED_ON;
		for(i = 0; i < 4; i++) 
		{
			if(index != led) 
			{
				sConfigLed[led].cmd = LED_OFF;
			}
			index++;
		}
	}
	else if((led >= LED_FAN_LOW) && (led <= LED_FAN_TURBO)) 
	{
		index = LED_FAN_LOW;
		sConfigLed[led].cmd = LED_ON;
		for(i = 0; i < 4; i++) 
		{
			if(index != led) 
			{
				sConfigLed[index].cmd = LED_OFF;
			}
			index++;
		}
	}

	return 0;
	
}

int OffLed ( Led_t led ) 
{
	unsigned char i = 0, index = 0;
	
	if(led >= LED_STATUS_MAX) 
	{
		return -1;
	}

	if((led >= LED_POWER) && (led <= LED_AI)) 
	{
		if(sConfigLed[led].cmd == LED_OFF) 
		{
			return 0;
		}
	}

	OffLedRs485(sConfigLed[led].ledNum);

	if((led >= LED_POWER) && (led <= LED_AI)) 
	{
		sConfigLed[led].cmd = LED_OFF;
	}
	else if((led >= LED_DUST_BLUE) && (led <= LED_DUST_RED)) 
	{
		index = LED_DUST_BLUE;
		for(i = 0; i < 4; i++) 
		{
			sConfigLed[index].cmd = LED_OFF;
			index++;
		}
	}
	else if((led >= LED_FAN_LOW) && (led <= LED_FAN_TURBO)) 
	{
		index = LED_FAN_LOW;
		for(i = 0; i < 4; i++) 
		{
			sConfigLed[index].cmd = LED_OFF;
			index++;
		}
	}

	return 0;
}


int OnAllLed (void) 
{
	unsigned int i = 0;
	unsigned char cmdbuff[RS485_MSG_NUM] = {0x6a,0x00,0x0d,0x07,0x17,0x27,0x31,0x42,0x54,0x64,0x72,0x81,0x91,0xa2,0xb4,0xc4,0x07,0xa4,0xa6};

    gpio_bit_set(GPIOB, GPIO_PIN_4);
	SerialWrite(3, cmdbuff ,RS485_MSG_NUM);
    gpio_bit_reset(GPIOB, GPIO_PIN_4);

	for(i = LED_DUST_BLUE; i < LED_STATUS_MAX; i++) 
	{
		sConfigLed[i].cmd = LED_ON;
	}

	return 0;	
}

int OffAllLed (void) 
{
	unsigned int i = 0;
	unsigned char cmdbuff[RS485_MSG_NUM] = {0x6a,0x00,0x0d,0x00,0x10,0x20,0x30,0x40,0x50,0x60,0x70,0x80,0x90,0xa0,0xb0,0xc0,0x70,0xd7,0xa6};

    gpio_bit_set(GPIOB, GPIO_PIN_4);
	SerialWrite(3, cmdbuff ,RS485_MSG_NUM);
    gpio_bit_reset(GPIOB, GPIO_PIN_4);

	for(i = LED_DUST_BLUE; i < LED_STATUS_MAX; i++) 
	{
		sConfigLed[i].cmd = LED_OFF;
	}

	return 0;	
}

static int CommandLed(int argc, char **argv) 
{
	int result = 0;

    int nerrors = arg_parse(argc, argv, (void **) &cmd_args);
    if (nerrors != 0) 
	{
        arg_print_errors(stderr, cmd_args.end, argv[0]);
        return 1;
    }

//	if(cmd_args.led_command->ival[0] == 0) 
//	{
//		result  = OffLed(cmd_args.led_num->ival[0]);
//	}
//	else if(cmd_args.led_command->ival[0] == 1) 
//	{
//		result  = OnLed(cmd_args.led_num->ival[0]);
//	}
//	else if(cmd_args.led_command->ival[0] == 2) 
//	{
//		OnAllLed();
//	}
//	else if(cmd_args.led_command->ival[0] == 3) 
//	{
//		OffAllLed();
//	}

    return result;
}

void Register_CommandLed (void) 
{
//	cmd_args.led_command = arg_int0(NULL, "com", "<0|1|2|3>", "led's com( 0: off, 1: on , 2: on all, 3: off all )");
//	cmd_args.led_num = arg_int1(NULL, "num", "< 0 ~ 23 >", "led's number");
//    cmd_args.end = arg_end(2);
	
    const esp_console_cmd_t cmd_led = 
	{
        .command = "led",
        .help = "led command led_number",
        .hint = NULL,
        .func = &CommandLed,
	    .argtable = &cmd_args
        
    };
//    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd_led) );
//     esp_console_cmd_register(&cmd_led);
}

