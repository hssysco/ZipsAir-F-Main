/*!
    \file    main.c
    \brief   GPIO running led

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

//==============================================================================
#include "gd32f30x.h"
//#include "gd32f303c_eval.h"
#include "systick.h"
#include <stdio.h>


extern void app_main();


//==============================================================================
//    brief      main function
//    param[in]  none
//    param[out] none
//    retval     none
//==============================================================================
int main(void)
{  
    /* initialize the LEDs */
//    led_init();
    
    /* configure systick */
    systick_config();

    /* USART interrupt configuration */
    nvic_irq_enable(USART0_IRQn, 0, 0);
    nvic_irq_enable(USART1_IRQn, 0, 0);
    nvic_irq_enable(UART3_IRQn, 0, 0);
    nvic_irq_enable(UART4_IRQn, 0, 0);
    
    app_main();
}
