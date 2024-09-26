#include "gd32f30x.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "serial.h"



//#define BUF_SIZE (1024)
//
//#define BUFF_SIZE	512
//#define BAUD_RATE	9600

//typedef struct SWUART
//{
//	unsigned char channel;
//	char rx;
//	char tx;
//	unsigned int buffSize;
//	unsigned int bitTime;
//	unsigned int rx_start_time;
//	unsigned int rx_end_time;
////	bool invert;
////	bool overflow;
//	volatile unsigned int inPos;
//	volatile unsigned int outPos;
//	unsigned char buffer[BUFF_SIZE];
//}tSWUart;
//
//tSWUart sSWUART[COMn];

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

//static void IRAM_ATTR SWUartRxHandler(void *args);

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
    rcu_periph_clock_enable(COM_GPIO_CLK[com_id]);
    if(COM4== com){        
        rcu_periph_clock_enable(RCU_GPIOD);
    }
    
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
    
    com_init(COM1,9600);    // Zigbee
    com_init(COM2,9600);    // Motor
    com_init(COM3,9600);    // PM sensor
    com_init(COM4,9600);    // LED
}

void InitSerilInterrupt(void)
{
    /* enable USART0 receive, transmit interrupt */
    usart_interrupt_enable(USART0, USART_INT_RBNE);
    usart_interrupt_enable(USART0, USART_INT_TC);
    
    /* enable USART1 receive, transmit interrupt */
    usart_interrupt_enable(USART1, USART_INT_RBNE);
    usart_interrupt_enable(USART1, USART_INT_TC);
    
    /* enable USART3 receive, transmit interrupt */
    usart_interrupt_enable(UART3, USART_INT_RBNE);
    usart_interrupt_enable(UART3, USART_INT_TC);
    
    /* enable USART4 receive, transmit interrupt */
    usart_interrupt_enable(UART4, USART_INT_RBNE);
    usart_interrupt_enable(UART4, USART_INT_TC);
}


void uart_write_bytes(uint8_t uart_num, const char *src, size_t size) {
    uint8_t i;
    
    for (i = 0; i < sizeof(src); i++) {
        if(uart_num == 0) {
            UART1_TX_Sts = SET;
            usart_data_transmit(COM1, *src); 
//            while (usart_flag_get(COM1, USART_FLAG_TC) == RESET); // Wait for transmit buffer empty           
            while (UART1_TX_Sts == SET); // Wait for transmit buffer empty
        }
        else if(uart_num == 1) {
            UART2_TX_Sts = SET;
            usart_data_transmit(COM2, *src); 
            while (UART2_TX_Sts == SET); // Wait for transmit buffer empty
        } 
        else if(uart_num == 2) {
            UART3_TX_Sts = SET;
            usart_data_transmit(COM3, *src); 
            while (UART3_TX_Sts == SET); // Wait for transmit buffer empty
        }
        else if(uart_num == 3) {
            UART4_TX_Sts = SET;
            usart_data_transmit(COM4, *src); 
            while (UART4_TX_Sts == SET); // Wait for transmit buffer empty
        }
    }
}

int SerialWrite(uint8_t channel, unsigned char *pData, unsigned int dataLen) 
{
	if((channel >= CHNNEL_TYPE_MAX ) || (pData == NULL)) 
	{
		return -1;
	}
	
    uart_write_bytes(channel, (const char *)pData, dataLen);
 
	return 0;
}

//int uart_read_bytes(uart_port_t uart_num, uint8_t *buf, size_t length, TickType_t ticks_to_wait) {
int uart_read_bytes(uint8_t channel, uint8_t *buf, size_t length) {
    // Error checking (e.g., validate uart_num, buf, length)

    // Access the UART hardware registers for the given port
//    uart_dev_t *uart = &UART[uart_num];
//
    // Check for available bytes in the UART receive FIFO
    int bytes_read = 0;
    while (bytes_read < length) {
        // Read a byte from the FIFO if available
        // (This might involve waiting if the FIFO is empty)

        // Increment bytes_read
    }

    // Return the number of bytes successfully read
}

extern uint8_t AboveRxCnt;

int SerialRead(uint8_t channel, unsigned char *pData, unsigned int dataLen) 
{

	int ret = -1;
	uint16_t i = 0, readData = 0;
	uint8_t index = 0, cnt = 0;
	int tmp = 0;

	if((channel >= CHNNEL_TYPE_MAX ) || (pData == NULL)) 
	{
		return ret;
	}

    if(channel == 3) {
        while(AboveRxCnt<dataLen);
        
    }
    
//	if(channel <= THERMOSTAT)
//	{
//		if(channel == AVOVE) 
//		{
////			ret = uart_read_bytes(UART_NUM_1, pData, dataLen, 200);
//		}
//		else 
//		{
////			ret = uart_read_bytes(UART_NUM_2, pData, dataLen, 20 / portTICK_RATE_MS);
//		}
//	}
//	else 
//	{
//		index = (channel - CO2);		
//
//		while(1) 
//		{
////			tmp =  SWUartAvail(index);
//			if(tmp > 0) 
//			{
//				for(i = 0; i < tmp; i++) 
//				{
////					SWUartRead (index, pData);
//					pData++;			
//				}
//
//				if(dataLen >= tmp) 
//				{
//					dataLen -= tmp;
//				}
//
//				readData += tmp;
//				if(readData == dataLen) 
//				{
//					break;
//				}				
//			}
//			else 
//			{
//				if(cnt >= 20) 
//				{
//					break;
//				}
//				
//				cnt++;
////				usleep(10000); /* 10ms */
//			}
//		}
//
//		if(readData > 0)
//			ret = readData;
//		else
//			ret = -2;
//
//	}

	return ret;
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

