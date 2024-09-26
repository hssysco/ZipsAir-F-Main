
#ifndef __SERIAL_H__
#define __SERIAL_H__

//#define HIGH	1
//#define LOW		0

typedef enum ChannelType{
	AVOVE = 0,
	THERMOSTAT,
	CO2,
	ICON_LED,
	DUST,
	CHNNEL_TYPE_MAX
}tChannelType;
//#define THERMOSTAT 1


typedef enum ModeType{
	MODE_RX = 0,
	MODE_TX,
	MODE_TYPE_MAX
}tModeType;

#define COMn                             4U

// ZIGBEE
#define COM1                        USART0
#define COM1_CLK                    RCU_USART0
#define COM1_TX_PIN                 GPIO_PIN_9
#define COM1_RX_PIN                 GPIO_PIN_10
#define COM1_GPIO_PORT              GPIOA
#define COM1_GPIO_CLK               RCU_GPIOA
// ABOV
#define COM2                        USART1
#define COM2_CLK                    RCU_USART1
#define COM2_TX_PIN                 GPIO_PIN_2
#define COM2_RX_PIN                 GPIO_PIN_3
#define COM2_GPIO_PORT              GPIOA
#define COM2_GPIO_CLK               RCU_GPIOA
// PM Sensor
#define COM3                        UART3
#define COM3_CLK                    RCU_UART3
#define COM3_TX_PIN                 GPIO_PIN_10
#define COM3_RX_PIN                 GPIO_PIN_11
#define COM3_GPIO_PORT              GPIOC
#define COM3_GPIO_CLK               RCU_GPIOC

#define COM4                        UART4
#define COM4_CLK                    RCU_UART4
#define COM4_TX_PIN                 GPIO_PIN_12
#define COM4_RX_PIN                 GPIO_PIN_2
#define COM4_GPIO_PORT              GPIOC
#define COM4_GPIO_CLK               RCU_GPIOC

#define UART_RTS	(UART_PIN_NO_CHANGE)
#define UART_CTS	(UART_PIN_NO_CHANGE)


void InitSerialDriver(void);
void InitSerilInterrupt(void);
//void SetRS485Mode(tChannelType channel, tModeType mode);
int SerialRead(uint8_t channel, unsigned char *pData, unsigned int dataLen);
int SerialWrite(uint8_t channel, unsigned char *pData, unsigned int dataLen);

uint16_t CRC16Checksum (unsigned char * d, int size);


#endif /* __SERIAL_H__ */





