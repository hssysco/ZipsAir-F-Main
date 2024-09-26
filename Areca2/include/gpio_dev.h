
#ifndef __GPIO_H__
#define __GPIO_H__

#define OPENED_COVER_ERR	0x02

typedef enum GpioEvent{
	COVER_OPEN = 1,
	COVER_CLOSED,
	BUTTON_1,
	EVENT_MAX
}GpioEventT;

typedef struct DrvGpioDev
{
	void (*fnGpioCallback)(GpioEventT);

}DrvGpioDevT;

void InitCheckGpio(void);
//void InitCheckGpio(DrvGpioDevT *pGpioDev);


typedef enum 
{
    KEY_REV0 = 0U,
    KEY_REV1 = 1U,
    KEY_REV2 = 2U,
    KEY_REV3 = 3U,
    KEY_DOOR = 4U,    
    KEY_BUT1 = 5U,    
}key_typedef_enum;

typedef enum 
{
    KEY_MODE_GPIO = 0U,
    KEY_MODE_EXTI = 1U
}keymode_typedef_enum;

#define KEYn                        6U

/* Haware Option */
#define REV0_PIN                    GPIO_PIN_4
#define REV0_GPIO_PORT              GPIOA
#define REV0_GPIO_CLK               RCU_GPIOA
#define REV0_EXTI_LINE              EXTI_4
#define REV0_EXTI_PORT_SOURCE       GPIO_PORT_SOURCE_GPIOA
#define REV0_EXTI_PIN_SOURCE        GPIO_PIN_SOURCE_4
#define REV0_EXTI_IRQn              EXTI4_IRQn  

#define REV1_PIN                    GPIO_PIN_5
#define REV1_GPIO_PORT              GPIOA
#define REV1_GPIO_CLK               RCU_GPIOA
#define REV1_EXTI_LINE              EXTI_5
#define REV1_EXTI_PORT_SOURCE       GPIO_PORT_SOURCE_GPIOA
#define REV1_EXTI_PIN_SOURCE        GPIO_PIN_SOURCE_5
#define REV1_EXTI_IRQn              EXTI5_9_IRQn 

#define REV2_PIN                    GPIO_PIN_6
#define REV2_GPIO_PORT              GPIOA
#define REV2_GPIO_CLK               RCU_GPIOA
#define REV2_EXTI_LINE              EXTI_6
#define REV2_EXTI_PORT_SOURCE       GPIO_PORT_SOURCE_GPIOA
#define REV2_EXTI_PIN_SOURCE        GPIO_PIN_SOURCE_6
#define REV2_EXTI_IRQn              EXTI5_9_IRQn 

#define REV3_PIN                    GPIO_PIN_7
#define REV3_GPIO_PORT              GPIOA
#define REV3_GPIO_CLK               RCU_GPIOA
#define REV3_EXTI_LINE              EXTI_7
#define REV3_EXTI_PORT_SOURCE       GPIO_PORT_SOURCE_GPIOA
#define REV3_EXTI_PIN_SOURCE        GPIO_PIN_SOURCE_7
#define REV3_EXTI_IRQn              EXTI5_9_IRQn 


/* Door Switch */
#define DOOR_PIN                    GPIO_PIN_6
#define DOOR_GPIO_PORT              GPIOB
#define DOOR_GPIO_CLK               RCU_GPIOB
#define DOOR_EXTI_LINE              EXTI_6
#define DOOR_EXTI_PORT_SOURCE       GPIO_PORT_SOURCE_GPIOB
#define DOOR_EXTI_PIN_SOURCE        GPIO_PIN_SOURCE_6
#define DOOR_EXTI_IRQn              EXTI5_9_IRQn 

/* BUT1 Switch */
#define BUT1_PIN                    GPIO_PIN_0
#define BUT1_GPIO_PORT              GPIOB
#define BUT1_GPIO_CLK               RCU_GPIOB
#define BUT1_EXTI_LINE              EXTI_0
#define BUT1_EXTI_PORT_SOURCE       GPIO_PORT_SOURCE_GPIOB
#define BUT1_EXTI_PIN_SOURCE        GPIO_PIN_SOURCE_0
#define BUT1_EXTI_IRQn              EXTI0_IRQn 
#endif /* __DOOR_H__ */


