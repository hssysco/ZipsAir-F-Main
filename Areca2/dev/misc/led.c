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

//typedef struct AboveTxInfo
//{
////	unsigned char	Power;
//	unsigned char	FanLevel;
//	unsigned int	Mode;
//
//	unsigned char	FltTmrRst;
//	unsigned short	FltTmr;
//	unsigned short	FltTmrLmt;
//
//	unsigned char	VSP[MAX_FAU_LVL];
//	int				VSPOffset;
//	int				Led;
//
//	int				RPMSet;
//	int				RPM[MAX_FAU_LVL];
//	
//}AboveTxInfoT;
//
Led_t Color = LED_DUST_RED;
Led_sts LED_sts = LED_PwrOn;
uint16_t LedTimer = 0;
uint16_t LightSts = 0;
////Led_t NewColor = 0;
//extern AboveTxInfoT *pAboveTxData;


void InitLed(void) 
{
    timer_oc_parameter_struct timer_ocintpara;
    timer_parameter_struct timer_initpara;

    rcu_periph_clock_enable(RCU_GPIOC);
    rcu_periph_clock_enable(RCU_AF);
    rcu_periph_clock_enable(RCU_TIMER7);
    gpio_init(GPIOC, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_7);
    gpio_init(GPIOC, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_8);
    gpio_init(GPIOC, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);
////    gpio_pin_remap_config(GPIO_TIMER2_FULL_REMAP, ENABLE);    


    timer_deinit(TIMER7);

//    gpio_init(GPIOC, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_7);
//    gpio_init(GPIOC, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_8);
//    gpio_init(GPIOC, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);

    gpio_bit_set(GPIOC, GPIO_PIN_7);
    gpio_bit_reset(GPIOC, GPIO_PIN_8);
    gpio_bit_reset(GPIOC, GPIO_PIN_9);


    /* TIMER7 configuration */
    timer_initpara.prescaler         = 119;
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.period            = 255;
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
    timer_initpara.repetitioncounter = 0;
    timer_init(TIMER7, &timer_initpara);

    /* CH1,CH2 and CH3 configuration in PWM mode */
    timer_ocintpara.outputstate  = TIMER_CCX_ENABLE;
    timer_ocintpara.outputnstate = TIMER_CCXN_DISABLE;
    timer_ocintpara.ocpolarity   = TIMER_OC_POLARITY_HIGH;
    timer_ocintpara.ocnpolarity  = TIMER_OCN_POLARITY_HIGH;
    timer_ocintpara.ocidlestate  = TIMER_OC_IDLE_STATE_LOW;
    timer_ocintpara.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW;

    timer_channel_output_config(TIMER7, TIMER_CH_1, &timer_ocintpara);
    timer_channel_output_config(TIMER7, TIMER_CH_2, &timer_ocintpara);
    timer_channel_output_config(TIMER7, TIMER_CH_3, &timer_ocintpara);

    timer_channel_output_pulse_value_config(TIMER7,TIMER_CH_1,100);
    timer_channel_output_mode_config(TIMER7,TIMER_CH_1,TIMER_OC_MODE_PWM0);
    timer_channel_output_shadow_config(TIMER7,TIMER_CH_1,TIMER_OC_SHADOW_DISABLE);

    timer_channel_output_pulse_value_config(TIMER7,TIMER_CH_2,100);
    timer_channel_output_mode_config(TIMER7,TIMER_CH_2,TIMER_OC_MODE_PWM0);
    timer_channel_output_shadow_config(TIMER7,TIMER_CH_2,TIMER_OC_SHADOW_DISABLE);

    timer_channel_output_pulse_value_config(TIMER7,TIMER_CH_3,100);
    timer_channel_output_mode_config(TIMER7,TIMER_CH_3,TIMER_OC_MODE_PWM0);
    timer_channel_output_shadow_config(TIMER7,TIMER_CH_3,TIMER_OC_SHADOW_DISABLE);

    /* auto-reload preload enable */
    timer_auto_reload_shadow_enable(TIMER7);
    timer_enable(TIMER7);
    timer_primary_output_config(TIMER7,ENABLE);
    /* auto-reload preload enable */
    timer_auto_reload_shadow_enable(TIMER7);
    timer_enable(TIMER7);

}


Led_t GetLedColor(Led_t led) 
{
    Led_t result = LED_DUST_RED;
    
    return result; 
}

void Set_LedColor(Led_t led) 
{
    if(Color == led) return;
    Color = led;
    
    switch(Color) {
        case LED_DUST_RED:
            timer_channel_output_pulse_value_config(TIMER7,TIMER_CH_1,100);
            timer_channel_output_pulse_value_config(TIMER7,TIMER_CH_2,0);
            timer_channel_output_pulse_value_config(TIMER7,TIMER_CH_3,0);
            break;
        case LED_DUST_GREEN:
            timer_channel_output_pulse_value_config(TIMER7,TIMER_CH_1,0);
            timer_channel_output_pulse_value_config(TIMER7,TIMER_CH_2,100);
            timer_channel_output_pulse_value_config(TIMER7,TIMER_CH_3,0);
            break;
        
        case LED_DUST_BLUE:
            timer_channel_output_pulse_value_config(TIMER7,TIMER_CH_1,0);
            timer_channel_output_pulse_value_config(TIMER7,TIMER_CH_2,0);
            timer_channel_output_pulse_value_config(TIMER7,TIMER_CH_3,100);
            break;
        
        case LED_DUST_ORANGE:
            timer_channel_output_pulse_value_config(TIMER7,TIMER_CH_1,100);
            timer_channel_output_pulse_value_config(TIMER7,TIMER_CH_2,100);
            timer_channel_output_pulse_value_config(TIMER7,TIMER_CH_3,0);
            break;
        
        case LED_GBLUE:
            timer_channel_output_pulse_value_config(TIMER7,TIMER_CH_1,0);
            timer_channel_output_pulse_value_config(TIMER7,TIMER_CH_2,100);
            timer_channel_output_pulse_value_config(TIMER7,TIMER_CH_3,100);
            break;
        case LED_RBLUE:
            timer_channel_output_pulse_value_config(TIMER7,TIMER_CH_1,100);
            timer_channel_output_pulse_value_config(TIMER7,TIMER_CH_2,0);
            timer_channel_output_pulse_value_config(TIMER7,TIMER_CH_3,100);
            break;
        case LED_PINK:
            timer_channel_output_pulse_value_config(TIMER7,TIMER_CH_1,100);
            timer_channel_output_pulse_value_config(TIMER7,TIMER_CH_2,100);
            timer_channel_output_pulse_value_config(TIMER7,TIMER_CH_3,100);
            break;       
        case LED_OFF:
            timer_channel_output_pulse_value_config(TIMER7,TIMER_CH_1,0);
            timer_channel_output_pulse_value_config(TIMER7,TIMER_CH_2,0);
            timer_channel_output_pulse_value_config(TIMER7,TIMER_CH_3,0);
            break;   
            
    }
}

void LED_Ctrl(void) {
//    LightSts = 
        
    switch(LED_sts) {
        case LED_PwrOn:
            if(LightSts == 1)
                Set_LedColor(LED_DUST_RED);
            else 
                Set_LedColor(LED_OFF);
            break;
            
        case LED_Level_0:	
            Set_LedColor(LED_DUST_RED);
            break;
            
        case LED_Level_1:	
            Set_LedColor(LED_DUST_ORANGE);
            break;
            
        case LED_Level_2:		
            Set_LedColor(LED_DUST_BLUE);
            break;
            
        case LED_Level_3:
            Set_LedColor(LED_DUST_GREEN);
            break;
            
        case LED_Level_4:	
            Set_LedColor(LED_GBLUE);
            break;
            
        case LED_Level_5:	
            Set_LedColor(LED_RBLUE);
            break;
            
        case LED_Level_AUTO:		
            Set_LedColor(LED_PINK);
            break;
            
        case LED_Err_Door:		
            if(LedTimer < 200)
                Set_LedColor(LED_DUST_RED);
            else if(LedTimer < 600)
                Set_LedColor(LED_OFF);
            else  LedTimer = 0;            
            break;
            
        case LED_Err_Moto:	
            if(LedTimer < 200)
                Set_LedColor(LED_DUST_RED);
            else if(LedTimer < 2000)
                Set_LedColor(LED_OFF);
            else  LedTimer = 0;
            break;
            
        case LED_Err_Filter:
            if(LedTimer < 1000)
                Set_LedColor(LED_DUST_RED);
            else if(LedTimer < 2000)
                Set_LedColor(LED_OFF);
            else  LedTimer = 0;
            break;
    }
}



