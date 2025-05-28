#include "gd32f30x.h"
#include <stdio.h>
#include <string.h>
#include "remote.h"
#include "gd32f30x_gpio.h"

typedef enum
{
  false = 0,
  true
} bool;


#define IR_TOOLS_FLAGS_PROTO_EXT (1 << 0) /*!< Enable Extended IR protocol */
#define IR_TOOLS_FLAGS_INVERSE (1 << 1)   /*!< Inverse the IR signal, i.e. take high level as low, and vice versa */

#define RMT_RX_CHANNEL			0     /*!< RMT channel for receiver */
#define RMT_RX_GPIO_NUM			13     /*!< GPIO number for receiver */

#define RMT_CLK_DIV				100    /*!< RMT counter clock divider */
#define RMT_TICK_10_US			(80000000/RMT_CLK_DIV/100000)   /*!< RMT counter value for 10 us.(Source clock is APB clock) */
#define RMT_TIMEOUT_US			9500   /*!< RMT receiver timeout value(us) */

#if 0
#define RMT_MARGIN				350                          
#define LEADING_CODE_HIGH_US	9000
#define LEADING_CODE_LOW_US		4500
#define PAYLOAD_ONE_HIGH_US		560
#define PAYLOAD_ONE_LOW_US		1690
#define PAYLOAD_ZERO_HIGH_US	560
#define PAYLOAD_ZERO_LOW_US		560
#define REPEAT_CODE_HIGH_US		9000
#define REPEAT_CODE_LOW_US		2250
#define ENDING_CODE_HIGH_US		560

#define DATA_FRAME_RMT_WORDS	34
#define REPEAT_FRAME_RMT_WORDS	2
#else

//#define RMT_MARGIN				200
#define RMT_MARGIN				350

#define LEADING_CODE_HIGH_US	3000
#define LEADING_CODE_LOW_US		1800

//#define PAYLOAD_ONE_HIGH_US		500
//#define PAYLOAD_ONE_LOW_US		500

//#define PAYLOAD_ZERO_HIGH_US		500
//#define PAYLOAD_ZERO_LOW_US		1500

#define PAYLOAD_ONE_HIGH_US		500
#define PAYLOAD_ONE_LOW_US		1500

#define PAYLOAD_ZERO_HIGH_US	500
#define PAYLOAD_ZERO_LOW_US		500


#define REPEAT_CODE_HIGH_US		9000
#define REPEAT_CODE_LOW_US		2250

#define ENDING_CODE_HIGH_US		500

#define DATA_FRAME_RMT_WORDS	50
//#define DATA_FRAME_RMT_WORDS	34
#define REPEAT_FRAME_RMT_WORDS	2

#endif


/* depending on remote controller */
#define	RMT_CUSTOM_CODE		0x15ab
#define RMT_VUP				0x9e61
#define RMT_VDN				0x9d62
//#else
//#define	RMT_CUSTOM_CODE		0x20
//#endif

void timer_configuration(void)
{
    /* TIMER2 configuration: input capture mode -------------------
    the external signal is connected to TIMER2 CH1 pin (PB5)
    the rising edge is used as active edge
    the TIMER2 CH1CV is used to compute the frequency value
    ------------------------------------------------------------ */
    timer_ic_parameter_struct timer_icinitpara;
    timer_parameter_struct timer_initpara;

    rcu_periph_clock_enable(RCU_TIMER2);

    timer_deinit(TIMER2);

    /* TIMER2 configuration */
    
#ifdef GD32F330
    timer_initpara.prescaler         = 83;
#endif /* GD32F330 */
#ifdef GD32F350
    timer_initpara.prescaler         = 107;
#endif /* GD32F350 */
    
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.period            = 65535;
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
    timer_initpara.repetitioncounter = 0;
    timer_init(TIMER2, &timer_initpara);

    /* TIMER2  configuration */
    /* TIMER2 CH0 input capture configuration */
    timer_icinitpara.icpolarity  = TIMER_IC_POLARITY_RISING;
    timer_icinitpara.icselection = TIMER_IC_SELECTION_DIRECTTI;
    timer_icinitpara.icprescaler = TIMER_IC_PSC_DIV1;
    timer_icinitpara.icfilter    = 0x0;
    timer_input_capture_config(TIMER2, TIMER_CH_1, &timer_icinitpara);

    /* auto-reload preload enable */
    timer_auto_reload_shadow_enable(TIMER2);
    /* clear channel 0 interrupt bit */
    timer_interrupt_flag_clear(TIMER2, TIMER_INT_CH1);
    /* channel 0 interrupt enable */
    timer_interrupt_enable(TIMER2, TIMER_INT_CH1);

    /* TIMER2 counter enable */
    timer_enable(TIMER2);
}


///*!
//    \brief      set GPIO mode
//    \param[in]  gpio_periph: GPIOx(x = A,B,C,D,F)
//                only one parameter can be selected which is shown as below:
//      \arg        GPIOx(x = A,B,C,D,F)
//    \param[in]  mode: gpio pin mode
//                only one parameter can be selected which is shown as below:
//      \arg        GPIO_MODE_INPUT: input mode
//      \arg        GPIO_MODE_OUTPUT: output mode
//      \arg        GPIO_MODE_AF: alternate function mode
//      \arg        GPIO_MODE_ANALOG: analog mode
//    \param[in]  pull_up_down: gpio pin with pull-up or pull-down resistor
//                only one parameter can be selected which is shown as below:
//      \arg        GPIO_PUPD_NONE: floating mode, no pull-up and pull-down resistors
//      \arg        GPIO_PUPD_PULLUP: with pull-up resistor
//      \arg        GPIO_PUPD_PULLDOWN:with pull-down resistor
//    \param[in]  pin: GPIO pin
//                one or more parameters can be selected which are shown as below:
//      \arg        GPIO_PIN_x(x=0..15), GPIO_PIN_ALL
//    \param[out] none
//    \retval     none
//*/
//void gpio_mode_set(uint32_t gpio_periph, uint32_t mode, uint32_t pull_up_down, uint32_t pin)
//{
//    uint16_t i;
//    uint32_t ctl, pupd;
//
//    ctl = GPIO_CTL(gpio_periph);
//    pupd = GPIO_PUD(gpio_periph);
//
//    for(i = 0U; i < 16U; i++) {
//        if((1U << i) & pin) {
//            /* clear the specified pin mode bits */
//            ctl &= ~GPIO_MODE_MASK(i);
//            /* set the specified pin mode bits */
//            ctl |= GPIO_MODE_SET(i, mode);
//
//            /* clear the specified pin pupd bits */
//            pupd &= ~GPIO_PUPD_MASK(i);
//            /* set the specified pin pupd bits */
//            pupd |= GPIO_PUPD_SET(i, pull_up_down);
//        }
//    }
//
//    GPIO_CTL(gpio_periph) = ctl;
//    GPIO_PUD(gpio_periph) = pupd;
//}
//
///*!
//    \brief      set GPIO output type and speed
//    \param[in]  gpio_periph: GPIOx(x = A,B,C,D,F)
//                only one parameter can be selected which is shown as below:
//      \arg        GPIOx(x = A,B,C,D,F)
//    \param[in]  otype: gpio pin output mode
//                only one parameter can be selected which is shown as below:
//      \arg        GPIO_OTYPE_PP: push pull mode
//      \arg        GPIO_OTYPE_OD: open drain mode
//    \param[in]  speed: gpio pin output max speed
//                only one parameter can be selected which is shown as below:
//      \arg        GPIO_OSPEED_2MHZ: output max speed 2MHz
//      \arg        GPIO_OSPEED_10MHZ: output max speed 10MHz
//      \arg        GPIO_OSPEED_50MHZ: output max speed 50MHz
//      \arg        GPIO_OSPEED_MAX: GPIO very high output speed, max speed more than 50MHz
//    \param[in]  pin: GPIO pin
//                one or more parameters can be selected which are shown as below:
//      \arg        GPIO_PIN_x(x=0..15), GPIO_PIN_ALL
//    \param[out] none
//    \retval     none
//*/
//void gpio_output_options_set(uint32_t gpio_periph, uint8_t otype, uint32_t speed, uint32_t pin)
//{
//    uint16_t i;
//    uint32_t ospeed0, ospeed1;
//
//    if(GPIO_OTYPE_OD == otype) {
//        GPIO_OMODE(gpio_periph) |= (uint32_t)pin;
//    } else {
//        GPIO_OMODE(gpio_periph) &= (uint32_t)(~pin);
//    }
//
//    /* get the specified pin output speed bits value */
//    ospeed0 = GPIO_OSPD0(gpio_periph);
//
//    if(GPIO_OSPEED_MAX == speed) {
//        ospeed1 = GPIO_OSPD1(gpio_periph);
//
//        for(i = 0U; i < 16U; i++) {
//            if((1U << i) & pin) {
//                /* enable very high output speed function of the pin when the corresponding OSPDy(y=0..15)
//                   is "11" (output max speed 50MHz) */
//                ospeed0 |= GPIO_OSPEED_SET(i, 0x03);
//                ospeed1 |= (1U << i);
//            }
//        }
//        GPIO_OSPD0(gpio_periph) = ospeed0;
//        GPIO_OSPD1(gpio_periph) = ospeed1;
//    } else {
//        for(i = 0U; i < 16U; i++) {
//            if((1U << i) & pin) {
//                /* clear the specified pin output speed bits */
//                ospeed0 &= ~GPIO_OSPEED_MASK(i);
//                /* set the specified pin output speed bits */
//                ospeed0 |= GPIO_OSPEED_SET(i, speed);
//            }
//        }
//        GPIO_OSPD0(gpio_periph) = ospeed0;
//    }
//}
//
//
///*!
//    \brief      set GPIO alternate function
//    \param[in]  gpio_periph: GPIOx(x = A,B,C)
//                only one parameter can be selected which is shown as below:
//      \arg        GPIOx(x = A,B,C)
//    \param[in]  alt_func_num: GPIO pin af function, please refer to specific device datasheet
//                only one parameter can be selected which is shown as below:
//      \arg        GPIO_AF_0: TIMER2, TIMER13, TIMER14, TIMER16, SPI0, SPI1, I2S0, CK_OUT, USART0, CEC,
//                              IFRP, TSI, CTC, I2C0, I2C1, SWDIO, SWCLK
//      \arg        GPIO_AF_1: USART0, USART1, TIMER2, TIMER14, I2C0, I2C1, IFRP, CEC
//      \arg        GPIO_AF_2: TIMER0, TIMER1, TIMER15, TIMER16, I2S0
//      \arg        GPIO_AF_3: TSI, I2C0, TIMER14
//      \arg        GPIO_AF_4(port A,B only): USART1, I2C0, I2C1, TIMER13
//      \arg        GPIO_AF_5(port A,B only): TIMER15, TIMER16, USBFS, I2S0
//      \arg        GPIO_AF_6(port A,B only): CTC, SPI1
//      \arg        GPIO_AF_7(port A,B only): CMP0, CMP1
//    \param[in]  pin: GPIO pin
//                one or more parameters can be selected which are shown as below:
//      \arg        GPIO_PIN_x(x=0..15), GPIO_PIN_ALL
//    \param[out] none
//    \retval     none
//*/
//void gpio_af_set(uint32_t gpio_periph, uint32_t alt_func_num, uint32_t pin)
//{
//    uint16_t i;
//    uint32_t afrl, afrh;
//
//    afrl = GPIO_AFSEL0(gpio_periph);
//    afrh = GPIO_AFSEL1(gpio_periph);
//
//    for(i = 0U; i < 8U; i++) {
//        if((1U << i) & pin) {
//            /* clear the specified pin alternate function bits */
//            afrl &= ~GPIO_AFR_MASK(i);
//            afrl |= GPIO_AFR_SET(i, alt_func_num);
//        }
//    }
//
//    for(i = 8U; i < 16U; i++) {
//        if((1U << i) & pin) {
//            /* clear the specified pin alternate function bits */
//            afrh &= ~GPIO_AFR_MASK(i - 8U);
//            afrh |= GPIO_AFR_SET(i - 8U, alt_func_num);
//        }
//    }
//
//    GPIO_AFSEL0(gpio_periph) = afrl;
//    GPIO_AFSEL1(gpio_periph) = afrh;
//}

/*
 * @brief RMT receiver initialization
 */
void RmtRXInit() 
{

    /*configure PB4 (TIMER2 CH1) as alternate function*/
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_AF);
    gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_5);
    
    nvic_priority_group_set(NVIC_PRIGROUP_PRE1_SUB3);
    nvic_irq_enable(TIMER2_IRQn, 1, 1);
    
//	rmt_config_t rmt_rx;
//	rmt_rx.channel = RMT_RX_CHANNEL;
//	rmt_rx.gpio_num = RMT_RX_GPIO_NUM;
//	rmt_rx.clk_div = RMT_CLK_DIV;
//	rmt_rx.mem_block_num = 1;
//	rmt_rx.rmt_mode = RMT_MODE_RX;
//	rmt_rx.rx_config.filter_en = true;
//	rmt_rx.rx_config.filter_ticks_thresh = 100;
//	rmt_rx.rx_config.idle_threshold = RMT_TIMEOUT_US / 10 * (RMT_TICK_10_US);
//
//	rmt_config(&rmt_rx);
//
//	gpio_set_pull_mode (RMT_RX_GPIO_NUM, GPIO_PULLUP_ONLY);
//
//	rmt_driver_install(rmt_rx.channel, 1000, 0);	
}


void RmtRX() 
{

}


//==============================================================================
#include "IR.h"
#include "in.h"
#include "gpio.h"
#include "timer.h"

#define MARK        0
#define SPACE       1



/*Availabe protocols*/
#define AVAILABLE_PROTOCOLS 2
typedef enum
{
	NEC = 0,
	SONY
}tPROTOCOL;


/*General parameters*/
#define TOLERANCE 			 0.25f
#define GapPeriod   		 5000
#define GapPeriodTICKS   GapPeriod/USPERTICK


/*Sony protocol Parameters*/
#define SONY_numOfBits          50
#define SONY_HeaderPeriod       2400
#define SONY_OneMarkPeriod      1200
#define SONY_ZeroMarkPeriod     600
#define SONY_SpacePeriod        600
#define SONY_BufferSize         (2*SONY_numOfBits)+3


/*NEC protocol Parameters*/
#define NEC_numOfBits          	32
#define NEC_HeaderMarkPeriod    9000
#define NEC_HeaderSpacePeriod   4500
#define NEC_MarkPeriod          560
#define NEC_ZeroSpacePeriod     560
#define NEC_OneSpacePeriod      1690
#define NEC_BufferSize         (2*NEC_numOfBits)+3


/*State Machine definitions for SONY protocol*/
typedef enum
{
    IR_IDLE = 0,
    IR_MARK,
    IR_SPACE
}tIR_STATE;

typedef unsigned char        tBOOL;

tIR_STATE currentState;


/*Private global variables*/
/*------------------------------------------------------------------------------------*/
/*------------------------------- Receiving Paramaters -------------------------------*/
/*------------------------------------------------------------------------------------*/
tBOOL newCodeFlag; //1 If a key is pressed and decoded successfully
tIR_DATA receivedData; //the received data in hexa
unsigned int recvTimer;
static char index;
unsigned int periods[NEC_BufferSize+2]; //2 more places to handle overflow cases
volatile tBOOL IRinput; //polling the IR input pin
volatile tBOOL deb;


/*------------------------------------------------------------------------------------*/
/*-------------------------------- Sending Paramaters --------------------------------*/
/*------------------------------------------------------------------------------------*/
unsigned int sendTimer;
unsigned char currentBit;



/*Private Interface*/

/*
    INPUTS: A POINTER TO THE ARRAY OF MEASURED PERIODS
    OUTPUTS: THE RESULTING CODE (1s AND 0s)
*/
void IR_resetRecieveData(void);
tBOOL IR_checkOverflow(void);
tBOOL IR_NECmatchHeaderMark(char index);
tBOOL IR_NECmatchHeaderSpace(char index);
tBOOL IR_NECmatchMark(char index);
tBOOL IR_NECmatchOneSpace(char index);
tBOOL IR_NECmatchZeroSpace(char index);
tBOOL IR_NECdecode(unsigned int* periods);

void 				IR_sendHeader();
void 				IR_sendBit(tBOOL bit);


/*Private interface definitions*/

void IR_resetRecieveData(void)
{
    newCodeFlag  = 0;        //no received code or not complete
    receivedData = 0;   //no decoded data
    recvTimer  = 0;          //reset the timer
    index = 0;
}

tBOOL IR_checkOverflow(void)
{
    return (index > (NEC_BufferSize+1));
}

/**/
tBOOL IR_NECmatchHeaderMark(char index)
{
	float Lmark,Umark;
	
 	Lmark = (1.0-TOLERANCE) * (float)NEC_HeaderMarkPeriod;
	Umark = (1.0+TOLERANCE) * (float)NEC_HeaderMarkPeriod;
	
	return (((periods[index]*USPERTICK)) >= Lmark && ((periods[index]*USPERTICK) <= Umark));
}

tBOOL IR_NECmatchHeaderSpace(char index)
{
	float Lmark,Umark;
	Lmark = (1.0-TOLERANCE) * (float)NEC_HeaderSpacePeriod;
	Umark = (1.0+TOLERANCE) * (float)NEC_HeaderSpacePeriod;
	
	return (((periods[index]*USPERTICK)) >= Lmark && ((periods[index]*USPERTICK) <= Umark));
}

tBOOL IR_NECmatchMark(char index)
{
	float Lmark,Umark;
	Lmark = (1.0-TOLERANCE) * (float)NEC_MarkPeriod;
	Umark = (1.0+TOLERANCE) * (float)NEC_MarkPeriod;

	return (((periods[index]*USPERTICK)) >= Lmark && ((periods[index]*USPERTICK) <= Umark));
}

tBOOL IR_NECmatchOneSpace(char index)
{
	float Lspace,Uspace;
	Lspace = (1.0-TOLERANCE) * (float)NEC_OneSpacePeriod;
	Uspace = (1.0+TOLERANCE) * (float)NEC_OneSpacePeriod;
	
	return (((periods[index]*USPERTICK) >= Lspace) && ((periods[index]*USPERTICK) <= Uspace));
}

tBOOL 			IR_NECmatchZeroSpace(char index)
{
	float Lspace,Uspace;
	Lspace = (1.0-TOLERANCE) * (float)NEC_ZeroSpacePeriod;
	Uspace = (1.0+TOLERANCE) * (float)NEC_ZeroSpacePeriod;
	
	return (((periods[index]*USPERTICK) >= Lspace) && ((periods[index]*USPERTICK) <= Uspace));
}

/**/
tBOOL IR_NECdecode(unsigned int* periods)
{
	
	unsigned char i;
	unsigned char currentBit = 0;
	
	/*Check the header mark*/
	if(!IR_NECmatchHeaderMark(1)) 	 return 0;  //faulty header MARK
	if(!IR_NECmatchHeaderSpace(2))	 return 0;	//faulty header SPACE
	
	/*Loop over the recieved periods, check for a 1 or a 0, then increment by 2*/
	for(i=3 ; i<NEC_BufferSize ; i+= 2)
	{
		/*Check for a prober '1' mark and '1' space*/
		/*if found, mark the current bit as 1*/
		if(IR_NECmatchMark(i) && IR_NECmatchOneSpace(i+1))
		{
			receivedData |= (1 << currentBit++); 
			continue;
		}
		/*Check for a prober '0' mark and '0' space*/
		/*if found, mark the current bit as 0*/
		if(IR_NECmatchMark(i) && IR_NECmatchZeroSpace(i+1))
		{
			currentBit++;
			continue;
		}
		
		return 0; //not a valid 1 or a zero, failed decoding
	}
	
	newCodeFlag = 1;
	return 1; //success
}
/**/
void IR_sendHeader()
{
	/*Send the Mark*/
	TIMER_enablePWM();
 	TIMER_delay(NEC_HeaderMarkPeriod);
		
	/*Send the Space*/
	TIMER_disablePWM();
	TIMER_delay(NEC_HeaderSpacePeriod);
	
}

void IR_sendBit(tBOOL bit)
{
	/*Send the Mark*/
	TIMER_enablePWM();
	TIMER_delay(NEC_MarkPeriod);
	
	/*Send the Space*/
	TIMER_disablePWM();
	if(bit & 0x1) TIMER_delay(NEC_OneSpacePeriod);
	else TIMER_delay(NEC_ZeroSpacePeriod);

}

/*Public interface definitions*/

void IR_init(void)
{
    /*config GPIO input pin*/
		IN_initInputPort(); 
    /*start in IDLE state*/
    tIR_STATE currentState = IR_IDLE;
    IR_resetRecieveData();
}


/*
 *  DESCRIPTION : Checks the current state and Updates the received periods array if it's in any of the receiving states
 *                if the receiving state is IDLE, it just increases the timer to measure the gap
*/
void IR_recvUpdate(void)
 {
    IRinput = IN_readIRinput() ;

    /*Increment the ticks counter*/
    recvTimer++;

    /*State Machine*/
    switch(currentState)
    {
        case IR_IDLE:
						
						/*get input and check for a mark && the end of the current gap*/
						if(IRinput == MARK)
						{ 
							/*Check for a long gap && old clicks are received*/
							if(recvTimer < GapPeriodTICKS || (newCodeFlag == 1))
							{
								recvTimer = 0;
							}
							else
							{
								periods[index++] = recvTimer; //store the period of the gap
								if(IR_checkOverflow())
								{
										//shouldn't be here ever
										//add debug code
										IR_resetRecieveData();
										currentState = IR_IDLE;
								}
								else
								{
										recvTimer = 0;
										currentState = IR_MARK;
								}
							}
						}
					
            break;

        case IR_MARK:
             if(IRinput == SPACE)
            {
                periods[index++] = recvTimer;
                if(IR_checkOverflow())
                {
                    IR_resetRecieveData();
                    currentState = IR_IDLE;
                }
                else
                {
                    recvTimer = 0;
                    currentState = IR_SPACE;
                }
            }
            break;

        case IR_SPACE:
            if(IRinput == MARK)
            {
                periods[index++] = recvTimer;
                if(IR_checkOverflow())
                {
                    IR_resetRecieveData();
                    currentState = IR_IDLE;
                }
                else
                {
                    recvTimer = 0;
                    currentState = IR_MARK;
                }
            }
            else //check for a long gap
            {
                if(recvTimer >= GapPeriodTICKS ) //received full packet or stopped in the middle of packet
                {
                    if(index == (NEC_BufferSize+1))//success
                    {
                    /*decode the received data, update receivedData variable and raise newCode flag*/
                    IR_NECdecode(periods); 
                    }
                    else //failed
                    {
                    /*reset and start over again*/
                    IR_resetRecieveData();
                    }
                    currentState = IR_IDLE;
                }
            }
            break;
    }
}

/*Returns true if a new code(a code that didn't got accessed) is found*/
tBOOL IR_validCodeDetected(void)
{
    return newCodeFlag;
}


tIR_DATA IR_getRecievedCode(void)
{
	/*Reset the routine protocol and start again*/
	tIR_DATA ret = receivedData;
	IR_resetRecieveData();
	currentState = IR_IDLE;
	/*No more data to return*/
	newCodeFlag = 0;
	return ret;
}
/**/


void IR_sendNECCode(tIR_DATA hexData)
{
	TIMER_disablePWM();
	
 	IR_sendHeader();
	for(currentBit = 0 ; currentBit < NEC_numOfBits ; currentBit++)
	{
		IR_sendBit((hexData & (0x1 << currentBit)) >> currentBit) ;
	}
}
