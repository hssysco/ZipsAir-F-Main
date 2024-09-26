#include <stdio.h>
//#include "freertos/FreeRTOS.h"
//#include "freertos/task.h"
//#include "driver/i2s.h"
//#include "esp_system.h"
//#include "esp_console.h"
#include "argtable3/argtable3.h"

#include <math.h>
#include "audio.h"


#define SAMPLE_RATE     (44100)
#define I2S_NUM         (0)
#define WAVE_FREQ_HZ    (100)

#define I2S_BCK_PIN		(GPIO_NUM_22)
#define I2S_LRCK_PIN	(GPIO_NUM_25)
#define I2S_DATA_PIN	(GPIO_NUM_26)
#define I2S_DI_IO       (-1)

#define SAMPLE_PER_CYCLE (SAMPLE_RATE/WAVE_FREQ_HZ)


int i2s_driver_initialize (void) {

//	esp_err_t err;
//
//	i2s_config_t i2s_config = {
//		.mode = I2S_MODE_MASTER | I2S_MODE_TX,									// Only TX
//		.sample_rate = 44100,
//		.bits_per_sample = 16,
//		.channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,							//1-channels
//		.communication_format = I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB,
//		.dma_buf_count = 6,
//		.dma_buf_len = 74,
//		.use_apll = 0,
//		.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1								//Interrupt level 1
//	};
//
//	i2s_pin_config_t pin_config = {
//		.bck_io_num = I2S_BCK_PIN,
//		.ws_io_num = I2S_LRCK_PIN,
//		.data_out_num = I2S_DATA_PIN,
//		.data_in_num = -1                                                       //Not used
//	};
//
//    err = i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL);
//	if(err != 0)
//		return -1;
//	
//	err = i2s_set_pin(I2S_NUM, &pin_config);
//	if(err != 0)
//		return -2;

	return 0;
}

int InitAudio (void) {

	int ret = 0;
	ret = i2s_driver_initialize();
	return ret;
}

