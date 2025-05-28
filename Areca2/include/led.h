
#ifndef __LED_H__
#define __LED_H__

typedef enum {
	LED_DUST_RED,		
	LED_DUST_GREEN,		
	LED_DUST_BLUE,		
	LED_DUST_ORANGE,		
	LED_GBLUE,		
	LED_RBLUE,		
	LED_PINK,		
	LED_OFF,		
}Led_t;

typedef enum {
	LED_PwrOn,		
	LED_Level_0,		
	LED_Level_1,		
	LED_Level_2,		
	LED_Level_3,		
	LED_Level_4,		
	LED_Level_5,		
	LED_Level_AUTO,		
	LED_Err_Door,		
	LED_Err_Moto,	
    LED_Err_Filter,
}Led_sts;
//typedef enum {
//	LED_OFF = 0x0,
//	LED_ON,
//}LedCmd_t;

void InitLed(void);
void Register_CommandLed (void);



#endif /* __LED_H__ */

