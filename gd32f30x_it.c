/*!
    \file    gd32f30x_it.c
    \brief   interrupt service routines

    \version 2021-03-23, V2.0.0, demo for GD32F30x
*/

/*
    Copyright (c) 2021, GigaDevice Semiconductor Inc.

    Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this 
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice, 
       this list of conditions and the following disclaimer in the documentation 
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors 
       may be used to endorse or promote products derived from this software without 
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
OF SUCH DAMAGE.
*/

#include "gd32f30x_it.h"
#include "systick.h"

//#define tx_buffer_size   (countof(tx_buffer))
//#define countof(a) (sizeof(a)/sizeof(*(a)))

extern FlagStatus    UART1_TX_Sts;
extern FlagStatus    UART1_RX_Sts;
extern FlagStatus    UART2_TX_Sts;
extern FlagStatus    UART2_RX_Sts;
extern FlagStatus    UART3_TX_Sts;
extern FlagStatus    UART3_RX_Sts;
extern FlagStatus    UART4_TX_Sts;
extern FlagStatus    UART4_RX_Sts;

//extern uint8_t      tx_buffer[];
//extern uint8_t      rx_buffer[] ;
//extern uint32_t     nbr_data_to_read, nbr_data_to_send;
//extern uint16_t     tx_counter, rx_counter;
extern uint16_t     Uart_TxTime;
extern uint16_t     Uart_RxTime;

extern uint8_t AboveRxData[100];
extern uint8_t AboveRxCnt;
extern uint8_t AboveRxstart;
extern uint8_t AboveRxend;

extern uint8_t ThermoRxData[100];
extern uint8_t ThermoRxstart;
extern uint8_t ThermoRxend;
extern uint8_t ThermoRxCnt;

//==============================================================================
//      NMI exception
//==============================================================================
void NMI_Handler(void)
{
}


//==============================================================================
//      HardFault exception
//==============================================================================
void HardFault_Handler(void)
{
    /* if Hard Fault exception occurs, go to infinite loop */
    while(1){
    }
}


//==============================================================================
//      MemManage exception
//==============================================================================
void MemManage_Handler(void)
{
    /* if Memory Manage exception occurs, go to infinite loop */
    while(1){
    }
}


//==============================================================================
//      BusFault exception
//==============================================================================
void BusFault_Handler(void)
{
    /* if Bus Fault exception occurs, go to infinite loop */
    while(1){
    }
}


//==============================================================================
//      UsageFault exception
//==============================================================================
void UsageFault_Handler(void)
{
    /* if Usage Fault exception occurs, go to infinite loop */
    while(1){
    }
}


//==============================================================================
//      SVC exception
//==============================================================================
void SVC_Handler(void)
{
}


//==============================================================================
//      DebugMon exception
//==============================================================================
void DebugMon_Handler(void)
{
}

//==============================================================================
//      PendSV exception
//==============================================================================
void PendSV_Handler(void)
{
}

//==============================================================================
//      SysTick exception
//==============================================================================
void SysTick_Handler(void)
{
    delay_decrement();
    if(Uart_TxTime < 60000)
       Uart_TxTime++;
    if(Uart_RxTime < 60000)
       Uart_RxTime++;
}


//==============================================================================
//      USART0 exception
//=============================================================================
void USART0_IRQHandler(void)
{
   
    uint8_t rx_tmp;
    
    if(RESET != usart_interrupt_flag_get(USART0, USART_INT_FLAG_RBNE)){
        /* read one byte from the receive data register */
        if(AboveRxCnt < 100) {
            rx_tmp = (uint8_t)usart_data_receive(UART4);
        }
        if(rx_tmp == 0x7E) {
            AboveRxstart = 1;
            AboveRxend = 0;
            AboveRxCnt = 0;            
        }    
        if(AboveRxstart == 1) {
            AboveRxData[AboveRxCnt++] = rx_tmp;
            if(rx_tmp == 0x7f)
                AboveRxend = 1;
        }
        
        if(AboveRxCnt >= 99) {
            AboveRxCnt = 0;
            AboveRxstart = 0;
        }
        Uart_RxTime = 0;
        usart_flag_clear(USART0, USART_FLAG_RBNE);
    }       

    if((RESET != usart_interrupt_flag_get(USART0, USART_INT_FLAG_TC))||(UART1_TX_Sts == SET)){
        UART1_TX_Sts = RESET;
        usart_flag_clear(USART0, USART_FLAG_TC);
    }
}


//==============================================================================
//      USART1 exception
//=============================================================================
void USART1_IRQHandler(void)
{
    if(RESET != usart_interrupt_flag_get(USART1, USART_INT_FLAG_RBNE)){
        /* read one byte from the receive data register */
        usart_flag_clear(USART1, USART_FLAG_RBNE);
        Uart_RxTime = 0;
    }       

    if((RESET != usart_interrupt_flag_get(USART1, USART_INT_FLAG_TC))||(UART2_TX_Sts == SET)){
        UART2_TX_Sts = RESET;
        usart_flag_clear(USART1, USART_FLAG_TC);
    }
}

//==============================================================================
//      UART3 exception
//=============================================================================
void UART3_IRQHandler(void)
{
    if(RESET != usart_interrupt_flag_get(UART3, USART_INT_FLAG_RBNE)){
        usart_flag_clear(UART3, USART_FLAG_RBNE);
        Uart_RxTime = 0;
    }       
    if((SET == usart_interrupt_flag_get(UART3, USART_INT_FLAG_TC))||(UART3_TX_Sts == SET)){
        /* write one byte to the transmit data register */
        UART3_TX_Sts = RESET;
        usart_interrupt_flag_clear(UART3, USART_INT_FLAG_TC);
        usart_flag_clear(UART3, USART_FLAG_TC);
    }
}


//==============================================================================
//      UART4 exception
//=============================================================================


void UART4_IRQHandler(void)
{
    uint8_t rx_tmp;

    if(RESET != usart_interrupt_flag_get(UART4, USART_INT_FLAG_RBNE)){
        /* read one byte from the receive data register */
        if(ThermoRxCnt < 100) {
            rx_tmp = (uint8_t)usart_data_receive(UART4);
        }
        if(rx_tmp == 0x7E) {
            ThermoRxstart = 1;
            ThermoRxend = 0;
            ThermoRxCnt = 0;            
        }    
        if(ThermoRxstart == 1) {
            ThermoRxData[ThermoRxCnt++] = rx_tmp;
            if(rx_tmp == 0x7f)
                ThermoRxend = 1;
        }
        
        if(ThermoRxCnt >= 99) {
            ThermoRxCnt = 0;
            ThermoRxstart = 0;
        }
        Uart_RxTime = 0;
        usart_flag_clear(UART4, USART_FLAG_RBNE);

    }       
    else if((RESET != usart_interrupt_flag_get(UART4, USART_INT_FLAG_TC))||(UART4_TX_Sts == SET)){
        /* write one byte to the transmit data register */
        UART4_TX_Sts = RESET;
        usart_flag_clear(UART4, USART_FLAG_TC);
    }
}

// BUT1 Interrupt
void EXTI0_IRQHandler(void)
{
    if(RESET != exti_interrupt_flag_get(EXTI_0)){
//        gpio_bit_write(GPIOB, GPIO_PIN_15, 
//                    (bit_status)(1 - gpio_input_bit_get(GPIOB, GPIO_PIN_15)));
        exti_interrupt_flag_clear(EXTI_0);
    }
}


// Door Interrupt
void EXTI5_9_IRQHandler(void) {
    if (exti_interrupt_flag_get(EXTI_6)) { // 예: EXTI 라인 8 확인
        exti_interrupt_flag_clear(EXTI_6); // 인터럽트 플래그 지우기
        // 인터럽트 처리 로직
    }
}

//void EXTI5_9_IRQHandler(void)
//{
//    if(RESET != exti_interrupt_flag_get(EXTI_6)){
////        gpio_bit_write(GPIOB, GPIO_PIN_15, 
////                    (bit_status)(1 - gpio_input_bit_get(GPIOB, GPIO_PIN_15)));
//        exti_interrupt_flag_clear(EXTI_6);
//    }
//}