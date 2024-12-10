
#ifndef __LED_H__
#define __LED_H__

typedef enum {
	LED_DUST_BLUE,		
	LED_DUST_GREEN,		
	LED_DUST_DGREEN,		
	LED_DUST_RED,		
	LED_POWER,		
	LED_FAN,		
	LED_SLEEP,		
	LED_LOCK,		
	LED_FILTER_ERR,		
	LED_AI,
	LED_FAN_LOW,
	LED_FAN_MID,
	LED_FAN_HIGH,
	LED_FAN_TURBO,	
	LED_STATUS_MAX
}Led_t;

typedef enum {
	LED_OFF = 0x0,
	LED_ON,
}LedCmd_t;

void InitLed(void);
void Register_CommandLed (void);

void OnLed ( uint8_t led  );
void OffLed ( uint8_t led  );
void OffAllLed (void);
void OnAllLed (void);

#endif /* __LED_H__ */

