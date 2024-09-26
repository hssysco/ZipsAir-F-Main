#include "gd32f30x.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//#include "freertos/FreeRTOS.h"
//#include "freertos/task.h"
//#include "freertos/queue.h"
//#include "driver/gpio.h"
#include "gpio_dev.h"

//#define GPIO_DOOR_PIN		34
//#define GPIO_BUTTON_1		36
//#define GPIO_INPUT_PIN_SEL  ((1ULL<<GPIO_DOOR_PIN) | (1ULL<<GPIO_BUTTON_1))
//#define ESP_INTR_FLAG_DEFAULT 0

//static xQueueHandle GPIO_EVT_QUEUE = NULL;
//
//static void IRAM_ATTR gpio_isr_handler(void* arg)
//{
//	uint32_t gpio_num = (uint32_t) arg;
//
//	xQueueSendFromISR(GPIO_EVT_QUEUE, &gpio_num, NULL);
//}

static uint32_t KEY_PORT[KEYn] = {REV0_GPIO_PORT, REV1_GPIO_PORT,
                                  REV2_GPIO_PORT, REV3_GPIO_PORT,
                                  DOOR_GPIO_PORT, BUT1_GPIO_PORT};

static uint32_t KEY_PIN[KEYn] = {REV0_PIN,REV1_PIN,
                                 REV2_PIN,REV3_PIN, 
                                 DOOR_PIN, BUT1_PIN};

static rcu_periph_enum KEY_CLK[KEYn] = {REV0_GPIO_CLK, REV1_GPIO_CLK,
                                        REV2_GPIO_CLK, REV3_GPIO_CLK,
                                        DOOR_GPIO_CLK, BUT1_GPIO_CLK};

static exti_line_enum KEY_EXTI_LINE[KEYn] = {REV0_EXTI_LINE, REV1_EXTI_LINE,
                                             REV2_EXTI_LINE, REV3_EXTI_LINE,
                                             DOOR_EXTI_LINE, BUT1_EXTI_LINE};

static uint8_t KEY_PORT_SOURCE[KEYn] = {REV0_EXTI_PORT_SOURCE,
                                        REV1_EXTI_PORT_SOURCE,
                                        REV2_EXTI_PORT_SOURCE,
                                        REV3_EXTI_PORT_SOURCE,
                                        DOOR_EXTI_PORT_SOURCE,
                                        BUT1_EXTI_PORT_SOURCE};

static uint8_t KEY_PIN_SOURCE[KEYn] = {REV0_EXTI_PIN_SOURCE,
                                       REV1_EXTI_PIN_SOURCE,
                                       REV2_EXTI_PIN_SOURCE,
                                       REV3_EXTI_PIN_SOURCE,
                                       DOOR_EXTI_PIN_SOURCE,
                                       BUT1_EXTI_PIN_SOURCE};

static uint8_t KEY_IRQn[KEYn] = {REV0_EXTI_IRQn, 
                                 REV1_EXTI_IRQn,
                                 REV2_EXTI_IRQn,
                                 REV3_EXTI_IRQn,
                                 DOOR_EXTI_IRQn,
                                 BUT1_EXTI_IRQn};

extern void CheckGpioCallback(GpioEventT event);

void gd_key_init(key_typedef_enum key_num, keymode_typedef_enum key_mode)
{
    /* enable the key clock */
    rcu_periph_clock_enable(KEY_CLK[key_num]);
    rcu_periph_clock_enable(RCU_AF);

    /* configure button pin as input */
    gpio_init(KEY_PORT[key_num], GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, KEY_PIN[key_num]);

    if (key_mode == KEY_MODE_EXTI) {
        /* enable and set key EXTI interrupt to the lowest priority */
        nvic_irq_enable(KEY_IRQn[key_num], 2U, 0U);

        /* connect key EXTI line to key GPIO pin */
        gpio_exti_source_select(KEY_PORT_SOURCE[key_num], KEY_PIN_SOURCE[key_num]);

        /* configure key EXTI line */
        exti_init(KEY_EXTI_LINE[key_num], EXTI_INTERRUPT, EXTI_TRIG_FALLING);
        exti_interrupt_flag_clear(KEY_EXTI_LINE[key_num]);
    }
}


FlagStatus gd_key_state_get(key_typedef_enum key)
{
    return gpio_input_bit_get(KEY_PORT[key], KEY_PIN[key]);
}

void InitCheckGpio(void)
{
    gd_key_init(KEY_REV0, KEY_MODE_GPIO);
    gd_key_init(KEY_REV1, KEY_MODE_GPIO);
    gd_key_init(KEY_REV2, KEY_MODE_GPIO);
    gd_key_init(KEY_REV3, KEY_MODE_GPIO);
    gd_key_init(KEY_DOOR, KEY_MODE_GPIO);
    gd_key_init(KEY_BUT1, KEY_MODE_GPIO);
}


//void gpio_check_task(void* arg)
void gpio_check_task(void)
{
	FlagStatus ret = RESET;
	uint8_t initflag = 0;
    
    ret = gd_key_state_get(KEY_DOOR);
    if(ret == SET)
        CheckGpioCallback(COVER_OPEN);
    else
        CheckGpioCallback(COVER_CLOSED);  
    
    ret = gd_key_state_get(KEY_BUT1);
    if(ret == RESET)
        CheckGpioCallback(BUTTON_1);

}

