#include "gd32f30x.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "serial.h"



static rcu_periph_enum COM_CLK[COMn] = {COM1_CLK, COM2_CLK, COM3_CLK, COM4_CLK};
static uint32_t COM_TX_PIN[COMn] = {COM1_TX_PIN, COM2_TX_PIN, COM3_TX_PIN, COM4_TX_PIN};
static uint32_t COM_RX_PIN[COMn] = {COM1_RX_PIN, COM2_RX_PIN, COM3_RX_PIN, COM4_RX_PIN};
static uint32_t COM_GPIO_PORT[COMn] = {COM1_GPIO_PORT, COM2_GPIO_PORT, COM3_GPIO_PORT, COM4_GPIO_PORT};
static rcu_periph_enum COM_GPIO_CLK[COMn] = {COM1_GPIO_CLK, COM2_GPIO_CLK, COM3_GPIO_CLK, COM4_GPIO_CLK};
FlagStatus    UART1_TX_Sts = RESET;
FlagStatus    UART1_RX_Sts = RESET;
FlagStatus    UART2_TX_Sts = RESET;
FlagStatus    UART2_RX_Sts = RESET;
FlagStatus    UART3_TX_Sts = RESET;
FlagStatus    UART3_RX_Sts = RESET;
FlagStatus    UART4_TX_Sts = RESET;
FlagStatus    UART4_RX_Sts = RESET;
//extern SystemInfoT  SystemInfo;

void com_init(uint32_t com, uint32_t baudrate)
{
    uint32_t com_id = 0U;
    
    if(COM1 == com){
        com_id = 0U;
    }else if(COM2 == com){
        com_id = 1U;
    }else if(COM3 == com){
        com_id = 2U;
    }else if(COM4== com){
        com_id = 3U;
    }
    
    /* enable GPIO clock */
    if(COM4== com){        
        rcu_periph_clock_enable(RCU_GPIOC);
//        rcu_periph_clock_enable(RCU_GPIOD);
    }
    else {
        rcu_periph_clock_enable(COM_GPIO_CLK[com_id]);
    }
//    rcu_periph_clock_enable(RCU_AF);
   
    /* enable USART clock */
    rcu_periph_clock_enable(COM_CLK[com_id]);
    if(COM4== com){
        rcu_periph_clock_enable(RCU_UART4);
    }
    /* connect port to USARTx_Tx */
    gpio_init(COM_GPIO_PORT[com_id], GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, COM_TX_PIN[com_id]);

    /* connect port to USARTx_Rx */
    if(COM4== com){
        gpio_init(GPIOD, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, COM4_RX_PIN);
    }
    else {
        gpio_init(COM_GPIO_PORT[com_id], GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, COM_RX_PIN[com_id]);
    }
    
    /* USART configure */
    usart_deinit(com);
//    usart_word_length_set(USART0, USART_WL_8BIT);
//    usart_stop_bit_set(USART0, USART_STB_1BIT);
//    usart_parity_config(USART0, USART_PM_NONE);

    usart_baudrate_set(com, baudrate);
    usart_receive_config(com, USART_RECEIVE_ENABLE);
    usart_transmit_config(com, USART_TRANSMIT_ENABLE);
    usart_enable(com);
}

void InitSerialDriver(void) 
{
    rcu_periph_clock_enable(RCU_GPIOA);
    gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_1);
    gpio_bit_reset(GPIOA, GPIO_PIN_1);

    
    rcu_periph_clock_enable(RCU_AF);
    rcu_periph_clock_enable(RCU_GPIOB);
    gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_4);
    gpio_bit_reset(GPIOB, GPIO_PIN_4);
    gpio_pin_remap_config(GPIO_SWJ_NONJTRST_REMAP, ENABLE);    
    
    if(gpio_input_bit_get(GPIOA, GPIO_PIN_6) == SET)
        com_init(COM1,460800  );    // Zigbee
    else
        com_init(COM1,115200 );    // Zigbee 
    
    
    com_init(COM2,9600);    // Motor
    com_init(COM3,9600);    // PM sensor
    com_init(COM4,9600);    // LED
}

void InitSerilInterrupt(void)
{
    /* enable USART0 receive, transmit interrupt */
    usart_interrupt_enable(USART0, USART_INT_RBNE);
//    usart_interrupt_enable(USART0, USART_INT_TC);
    
    /* enable USART1 receive, transmit interrupt */
    usart_interrupt_enable(USART1, USART_INT_RBNE);
//    usart_interrupt_enable(USART1, USART_INT_TC);
    
    /* enable USART3 receive, transmit interrupt */
    usart_interrupt_enable(UART3, USART_INT_RBNE);
//    usart_interrupt_enable(UART3, USART_INT_TC);
    
    /* enable USART4 receive, transmit interrupt */
    usart_interrupt_enable(UART4, USART_INT_RBNE);
//    usart_interrupt_enable(UART4, USART_INT_TC);
    
    /* USART interrupt configuration */
    nvic_irq_enable(USART0_IRQn, 0, 0);
    nvic_irq_enable(USART1_IRQn, 0, 0);
    nvic_irq_enable(UART3_IRQn, 0, 0);
    nvic_irq_enable(UART4_IRQn, 0, 0);

}

extern uint16_t     Motor_CommTime;

void uart_write_byte(uint8_t uart_num, uint8_t *src) {
    uint8_t tx_byte;
    
    tx_byte = *src;
    if(uart_num == 0) {
        UART1_TX_Sts = SET;
//        while (usart_flag_get(COM1, USART_FLAG_TC) == SET); 
        usart_data_transmit(COM1, tx_byte); 
        while (usart_flag_get(COM1, USART_FLAG_TC) == RESET); 
//        while (UART1_TX_Sts == SET); 
    }
    else if(uart_num == 1) {
        UART2_TX_Sts = SET;
//        while (usart_flag_get(COM2, USART_FLAG_TC) == SET); 
        usart_data_transmit(COM2, tx_byte); 
        while (usart_flag_get(COM2, USART_FLAG_TC) == RESET) {
//            if(Motor_CommTime > 500) {
//                usart_flag_clear(COM2, USART_FLAG_TC);
//                return;
//            }
        }
//        while (UART2_TX_Sts == SET); 
    } 
    else if(uart_num == 2) {
        UART3_TX_Sts = SET;
//        while (usart_flag_get(COM3, USART_FLAG_TC) == SET); 
        usart_data_transmit(COM3, tx_byte); 
        while (usart_flag_get(COM3, USART_FLAG_TC) == RESET); 
//        while (UART3_TX_Sts == SET); 
    }
    else if(uart_num == 3) {
        UART4_TX_Sts = SET;
//        while (usart_flag_get(COM4, USART_FLAG_TC) == SET); 
        usart_data_transmit(COM4, tx_byte); 
        while (usart_flag_get(COM4, USART_FLAG_TC) == RESET); 
//        while (UART4_TX_Sts == SET); 
    }
}

int SerialWrite(uint8_t channel, uint8_t *pData, uint16_t dataLen) 
{
    uint8_t i;

	if((channel >= CHNNEL_TYPE_MAX ) || (pData == NULL)) 
	{
		return -1;
	}
    
    for (i = 0; i < dataLen; i++) {

        uart_write_byte(channel, (uint8_t *)pData++);
    }
	return 0;
}






uint16_t CRC16Checksum (unsigned char * d, int size) 
{ 
	#define CRC16_INIT_VAL   0xFFFF
	#define CRC16_POLY   0xA001
	
	int i, j;
	int len = size;
	unsigned short crc16 = CRC16_INIT_VAL;
	unsigned short rxbitcheck;

	for(i=0; i<len; i++) 
	{
		crc16 ^= d[i];   
		for(j=0; j<8; j++) 
		{
			rxbitcheck = crc16 & 0x0001;
			crc16 >>=1; 
			if(rxbitcheck) crc16 ^= CRC16_POLY;
		}
	}

	return crc16;
}

