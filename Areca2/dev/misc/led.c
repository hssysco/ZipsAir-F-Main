#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>


#include "led.h"
#include "gd32f30x.h"

#define RS485_MSG_NUM	19
#define HEADER			0x6A
#define LEDNUM			0x0d
#define ETX				0xA6 

#define BLUE_ON			0x04
#define GREEN_ON		0x02
#define RED_ON			0x01



void InitLed(void) 
{
}


uint8_t GetLedColor(uint8_t led) 
{
    uint8_t result = 0;
    
    return result; 
}

void OnLed (uint8_t led) 
{
	
}

void OffLed (uint8_t led ) 
{
}


void OnAllLed (void) 
{

}

void OffAllLed (void) 
{
	
}


