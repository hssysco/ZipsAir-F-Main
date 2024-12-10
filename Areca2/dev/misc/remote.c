#include "gd32f30x.h"
#include <stdio.h>
#include <string.h>
#include "remote.h"

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

#define CHECK(a, str, goto_tag, ret_value, ...)                               \
	do																			  \
{																			  \
	if (!(a))																  \
	{																		  \
		ret = ret_value;													  \
		goto goto_tag;														  \
	}																		  \
} while (0)

#if 0
/* depending on remote controller */
#define	RMT_CUSTOM_CODE		0x15ab
#define RMT_VUP				0x9e61
#define RMT_VDN				0x9d62
#else
#define	RMT_CUSTOM_CODE		0x20
#endif

/**
 * @brief IR device type
 *
 */
typedef void *ir_dev_t;

/**
 * @brief IR parser type
 *
 */
typedef struct ir_parser_s ir_parser_t;


/**
 * @brief Type definition of IR parser
 *
 */
struct ir_parser_s 
{
	int16_t (*input)(ir_parser_t *parser, void *raw_data, uint32_t length);

	int16_t (*get_scan_code)(ir_parser_t *parser, uint32_t *address, uint32_t *command, bool *repeat);

	/**
	 * @brief Free resources used by IR parser
	 *
	 * @param[in] parser: Handle of IR parser
	 *
	 * @return
	 *	   - ESP_OK: Free resource successfully
	 *	   - ESP_FAIL: Free resources fail failed because some error occurred
	 */
	int16_t (*del)(ir_parser_t *parser);
};


/**
 * @brief Configuration type of IR parser
 *
 */
typedef struct 
{
	ir_dev_t dev_hdl;	/*!< IR device handle */
	uint32_t flags; 	/*!< Flags for IR parser, different flags will enable different features */
	uint32_t margin_us; /*!< Timing parameter, indicating the tolerance to environment noise */
} ir_parser_config_t;

typedef struct 
{
	ir_parser_t parent;
	uint32_t flags;
	uint32_t leading_code_high_ticks;
	uint32_t leading_code_low_ticks;
	uint32_t repeat_code_high_ticks;
	uint32_t repeat_code_low_ticks;
	uint32_t payload_logic0_high_ticks;
	uint32_t payload_logic0_low_ticks;
	uint32_t payload_logic1_high_ticks;
	uint32_t payload_logic1_low_ticks;
	uint32_t margin_ticks;
//	rmt_item32_t *buffer;
	uint32_t cursor;
	uint32_t last_address;
	uint32_t last_command;
	bool repeat;
	bool inverse;
}parser_t;



static bool parse_head(parser_t *pParser)
{
    bool ret = true;
	return ret;
}

static bool parse_logic0(parser_t *pParser)
{
    bool ret = true;
    
	return ret;
}

static bool parse_logic1(parser_t *pParser)
{
    bool ret = true;

	return ret;
}

static int16_t parse_logic(ir_parser_t *parser, bool *logic)
{
	int16_t ret = -1;
	bool logic_value = false;
	return ret;
}

static bool parse_repeat_frame(parser_t *pParser)
{
    bool ret = true;

	return ret;
}

static int16_t parser_input(ir_parser_t *parser, void *raw_data, uint32_t length)
{
    int16_t ret = true;

	return ret;
}

static int16_t parser_get_scan_code(ir_parser_t *parser, uint32_t *address, uint32_t *command, bool *repeat)
{
	int16_t ret = -1;

	return ret;
}


/*
 * @brief RMT receiver initialization
 */
static void RmtRXInit() 
{
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

void RmtRXTask(void* arg) 
{
	int Channel = RMT_RX_CHANNEL;
	DrvRmtDevT *pRmtDev = (DrvRmtDevT *)arg;
//	RingbufHandle_t rb = NULL;
//	rmt_item32_t *pData = NULL;
	size_t rx_size = 0;

//	RemoteEventT param;
//
	uint32_t addr = 0, cmd =0, length = 0, counter_clk_hz = 0;
	bool repeat = false;
	ir_parser_config_t ir_parser_config;
	ir_parser_t *ir_parser = NULL;

	unsigned char custom = 0;
	unsigned char on = 0;
	unsigned char mode = 0;
	unsigned char sleep = 0;
	unsigned char flt = 0;

	unsigned char fan = 0;
	unsigned char time = 0;
	unsigned char resv = 0;
	uint32_t tmp = 0;

	if(pRmtDev == NULL) 
	{
		return;
	}

	memset(&ir_parser_config, 0, sizeof(ir_parser_config_t));

	ir_parser_config.dev_hdl = (ir_dev_t)RMT_RX_CHANNEL;
	ir_parser_config.flags = 0; 
	ir_parser_config.margin_us = RMT_MARGIN;

//	counter_clk_hz = (80000000/RMT_CLK_DIV);
//	ir_parser = ir_parser_rmt_new(&ir_parser_config, counter_clk_hz);
//	if(ir_parser == NULL) 
//	{
//		return;
//	}

	RmtRXInit();

//	rmt_get_ringbuf_handle(Channel, &rb);
//	rmt_rx_start(Channel, 1);
//
//	while(rb) 
//	{
//		rx_size = 0;
//		pData = (rmt_item32_t *) xRingbufferReceive(rb, &rx_size, 1000);
//		if(pData != NULL) 
//		{
//			length = rx_size;
//			length /= 4; // one RMT = 4 Bytes ==> 32bits
//
//			if (ir_parser->input(ir_parser, pData, length) == ESP_OK) 
//			{
//				addr = 0;
//				cmd = 0;
//				repeat = false;
//				if (ir_parser->get_scan_code(ir_parser, &addr, &cmd, &repeat) == ESP_OK) 
//				{			
//					//printf("@@ Rmt 2(%.6x : %.6x) !!! \r\n", addr , cmd);
//
//					if(repeat == false)
//					{
//#if 0
//						if(addr == RMT_CUSTOM_CODE)
//						{
//							switch(cmd) 
//							{
//								case RMT_VUP:
//									EventData = EVENT_VUP;
//									break;
//
//								case RMT_VDN:
//									EventData = EVENT_VDN;
//									break;
//
//								default:
//									break;				
//							}
//
//							if(EventData != EVENT_TYPE_MAX)
//							{
//								if (pRmtDev->fnRmtCallback) 
//								{
//									pRmtDev->fnRmtCallback(EventData);
//								}								
//							}					
//						}
//#else
//						tmp = addr;
//						custom = (tmp&0xff);
//
//						memset(&param, 0, sizeof(RemoteEventT));
//
//						if(custom == RMT_CUSTOM_CODE)
//						{
//							on = ((tmp >> 8)&0xff);
//							if(on)
//							{
//								mode = ((tmp >> 16)&0x10);
//								sleep = ((tmp >> 16)&0x01);
//								flt = ((tmp >> 16)&0x02);
//
//								fan = (cmd&0xff);
//								time = ((cmd >> 8)&0xff);
//
//								////printf("@@ Rmt 3 (%x) !!! \r\n", time); 					
//
//								resv = ((time&0x80) >> 7);
//
//								if(resv)
//								{
//									time &= ~(0x80);
//#if 1
//									param.ResvOn = 1;
//									switch(time)
//									{
//										case 0x01:
//											param.ResvTime = 1;
//											break;
//										case 0x02:
//											param.ResvTime = 2;
//											break;
//										case 0x03:
//											param.ResvTime = 3;
//											break;
//										case 0x04:
//											param.ResvTime = 4;
//											break;
//										case 0x05:
//											param.ResvTime = 5;
//											break;
//										case 0x06:
//											param.ResvTime = 6;
//											break;
//										case 0x07:
//											param.ResvTime = 7;
//											break;
//										case 0x08:
//											param.ResvTime = 8;
//											break;
//										case 0x09:
//											param.ResvTime = 9;
//											break;
//										case 0x0a:
//											param.ResvTime = 10;
//											break;
//										case 0x0b:
//											param.ResvTime = 11;
//											break;
//										case 0x0c:
//											param.ResvTime = 12;
//											break;
//										default:
//											break;
//									}
//#endif
//								}
//								else
//								{
//									param.ResvOn = 0;
//								}
//
//								//								//printf("@@ Rmt 4( %x :%d : %d: %d: %d: %d ) !!! \r\n", mode, sleep, flt, fan, resv, time);							
//
//								if(mode == 0)	/* Auto */
//								{
//									param.Mode = 1;
//								}
//								else			/* Manual */
//								{
//									param.Mode = 2;
//								}
//
//								if((param.Mode == 2) && (sleep == 0))
//								{
//									if(fan < 4)
//									{
//										param.FanLevel = fan + 2;
//									}
//								}
//								else if((param.Mode == 2) && (sleep == 1))
//								{
//									param.FanLevel = 1;
//								}
//								else
//								{
//									param.FanLevel = 0;
//								}
//
//								if(flt)
//								{
//									param.ResetFlt = 1;
//								}
//								else
//								{
//									param.ResetFlt = 0;
//								}
//
//								param.ResvOn = resv;
//								param.ResvTime = time;
//							}
//
//							//							//printf("@@ Rmt 5( %d : %d: %d: %d: %d ) !!! \r\n", param.Mode, param.FanLevel, param.ResetFlt, param.ResvOn, param.ResvTime);							
//
//							if (pRmtDev->fnRmtCallback) 
//							{
//								pRmtDev->fnRmtCallback(&param);
//							}								
//
//						}
//#endif						
//
//					}
//				}
//			}
//
//			vRingbufferReturnItem(rb, (void*) pData);
//		}
//	}
//
//	ir_parser->del(ir_parser);
}

void InitRemote (DrvRmtDevT *pRmtDev)
{
	if(pRmtDev == NULL)
	{
		return;
	}

//	xTaskCreatePinnedToCore(&RmtRXTask, "Remote Task", 4096, (void *)pRmtDev, 5, NULL, 1);	
}

