#include "gd32f30x.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <time.h>
//#include <sys/time.h>
//#include <unistd.h>

//#include "esp_log.h"
//#include "driver/i2c.h"
//#include "sdkconfig.h"

#include "pressure.h"

//#define I2C1_SPEED              400000
#define I2C_PAGE_SIZE           8

#define DATA_LENGTH						512  /*!<Data buffer length for test buffer*/
#define RW_TEST_LENGTH					129  /*!<Data length for r/w test, any value from 0-DATA_LENGTH*/
#define DELAY_TIME_BETWEEN_ITEMS_MS		1234 /*!< delay time between different test items */

//#define I2C_SCL_IO				21    /*!<gpio number for i2c slave clock  */
//#define I2C_SDA_IO				4    /*!<gpio number for i2c slave data */
//#define I2C_NUM					I2C_NUM_0    /*!<I2C port number for slave dev */

#define I2C_SLAVE_ADDR					0x76
#define I2C_DATA_LEN					64
#define I2C_MASTER_FREQ_HZ    100000     /*!< I2C master clock frequency */
#define WRITE_BIT  I2C_MASTER_WRITE /*!< I2C master write */
#define READ_BIT   I2C_MASTER_READ  /*!< I2C master read */
#define ACK_CHECK_EN   0x1     /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS  0x0     /*!< I2C master will not check ack from slave */
#define ACK_VAL    0x0         /*!< I2C ack value */
#define NACK_VAL   0x1         /*!< I2C nack value */

DrvPressureDevT *pInitDev = NULL;


static int i2c_driver_initialize() 
{
	int rc = 0;
//    i2c_config_t conf;
//
//    conf.sda_io_num = I2C_SDA_IO;
//    conf.scl_io_num = I2C_SCL_IO;
//	conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
//	conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
//    conf.mode = I2C_MODE_MASTER;
//    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
//
//    i2c_param_config(I2C_NUM, &conf);
//	
//	rc = i2c_driver_install(I2C_NUM, I2C_MODE_MASTER, 0, 0, 0);
//
//	printf("## Bme680 i2c_driver_initialize: rc (%d) \r\n",rc);
    
    /* enable GPIOB clock */
    rcu_periph_clock_enable(RCU_GPIOB);
    /* enable I2C1 clock */
    rcu_periph_clock_enable(RCU_I2C1);

    /* connect PB6 to I2C0_SCL */
    /* connect PB7 to I2C0_SDA */
    gpio_init(GPIOB, GPIO_MODE_AF_OD, GPIO_OSPEED_50MHZ, GPIO_PIN_6 | GPIO_PIN_7);
    
    /* enable I2C clock */
    rcu_periph_clock_enable(RCU_I2C1);
    /* configure I2C clock */
    i2c_clock_config(RCU_I2C1,I2C_MASTER_FREQ_HZ,I2C_DTCY_2);
    /* configure I2C address */
    i2c_mode_addr_config(RCU_I2C1,I2C_I2CMODE_ENABLE,I2C_ADDFORMAT_7BITS,I2C_SLAVE_ADDR);
    /* enable RCU_I2C1 */
    i2c_enable(RCU_I2C1);
    /* enable acknowledge */
    i2c_ack_config(RCU_I2C1,I2C_ACK_ENABLE);
    
    printf("## Bme680 i2c_driver_initialize: rc (%d) \r\n",rc);

	return rc;
}

//static int i2c_read(i2c_port_t i2c_num, uint8_t dev_addr, uint8_t *data_rd, size_t size)
static int i2c_read(uint8_t dev_addr, uint8_t *data_rd, size_t size)
{
	int ret = 0;

	if (size == 0) {
	    return 0;
	}

//	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
//
//	ret = i2c_master_start(cmd);
//	
//	ret = i2c_master_write_byte(cmd, (dev_addr << 1) | READ_BIT, ACK_CHECK_EN);
//	if (size > 1) {
//	    ret = i2c_master_read(cmd, data_rd, size - 1, ACK_VAL);
//	}
//	
//	ret = i2c_master_read_byte(cmd, data_rd + size - 1, NACK_VAL);
//
//	ret = i2c_master_stop(cmd);
//
//	ret = i2c_master_cmd_begin(i2c_num, cmd, 100);
//
//	i2c_cmd_link_delete(cmd);
	return ret;
}

//static int i2c_write(i2c_port_t i2c_num, uint8_t dev_addr, uint8_t *data_wr, size_t size)
static int i2c_write(uint8_t dev_addr, uint8_t *data_wr, size_t size)
{
	int ret = 0;

//	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
//
//	ret = i2c_master_start(cmd);
//	
//	ret = i2c_master_write_byte(cmd, (dev_addr << 1) | WRITE_BIT, ACK_CHECK_EN);
//
//	ret = i2c_master_write(cmd, data_wr, size, ACK_CHECK_EN);
//
//	ret = i2c_master_stop(cmd);
//
//	ret = i2c_master_cmd_begin(i2c_num, cmd, 100);
//
//	i2c_cmd_link_delete(cmd);

	return ret;
}



/*!
 * @brief           Write operation in either I2C or SPI
 *
 * param[in]        dev_addr        I2C or SPI device address
 * param[in]        reg_addr        register address
 * param[in]        reg_data_ptr    pointer to the data to be written
 * param[in]        data_len        number of bytes to be written
 *
 * @return          result of the bus communication function
 */
static int8_t bme680_bus_write(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data_ptr, uint16_t data_len) 
{
    int8_t i;

	uint8_t data[I2C_DATA_LEN];
//	esp_err_t result = 0;
	int8_t ret = 0;
	
	data[0] = reg_addr;

	for(i=1;i<=data_len;i++) 
	{
		data[i] = reg_data_ptr[i-1];
	}

//	result = i2c_write(I2C_NUM, dev_addr, data, (data_len + 1));
//	if(result != 0) 
//	{
//		ret  = 1;
//	}
	
	return ret;
}

/*!
 * @brief           Read operation in either I2C or SPI
 *
 * param[in]        dev_addr        I2C or SPI device address
 * param[in]        reg_addr        register address
 * param[out]       reg_data_ptr    pointer to the memory to be used to store the read data
 * param[in]        data_len        number of bytes to be read
 *
 * @return          result of the bus communication function
 */
static int8_t bme680_bus_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data_ptr, uint16_t data_len) 
{
//	esp_err_t result = 0;
	int8_t ret = 0;

//	result = i2c_write(I2C_NUM, dev_addr, &reg_addr, 1);
//	if(result != 0) 
//	{
//		ret  = 1;
//	}

//	result = i2c_read(I2C_NUM, dev_addr, reg_data_ptr, data_len);
//	if(result != 0) 
//	{
//		ret  = 1;
//	}

    return ret;
}

/*!
 * @brief           System specific implementation of sleep function
 *
 * @param[in]       t_ms    time in milliseconds
 *
 * @return          none
 */
static void bme680_sleep(u32 msek) 
{
	if(!msek) return;
	usleep(msek*1000);
}

/*!
 * @brief           Capture the system time in microseconds
 *
 * @return          system_current_time    current system timestamp in microseconds
 */
static int64_t bme680_get_timestamp_us() 
{
    int64_t system_current_time = 0;
    struct timespec time;
	
//	clock_gettime(CLOCK_REALTIME, &time);
	system_current_time = (int64_t)time.tv_sec*1000*1000+(int64_t)time.tv_nsec/1000;
	
    return system_current_time;
}

/*!
 * @brief           Handling of the ready outputs
 *
 * @param[in]       timestamp       time in nanoseconds
 * @param[in]       iaq             IAQ signal
 * @param[in]       iaq_accuracy    accuracy of IAQ signal
 * @param[in]       temperature     temperature signal
 * @param[in]       humidity        humidity signal
 * @param[in]       pressure        pressure signal
 * @param[in]       raw_temperature raw temperature signal
 * @param[in]       raw_humidity    raw humidity signal
 * @param[in]       gas             raw gas sensor signal
 * @param[in]       bsec_status     value returned by the bsec_do_steps() call
 *
 * @return          none
 */

static void bme680_output_ready(int64_t timestamp, float iaq, uint8_t iaq_accuracy, float temperature, float humidity,
     float pressure, float raw_temperature, float raw_humidity, float gas, bsec_library_return_t bsec_status,
     float static_iaq, float co2_equivalent, float breath_voc_equivalent) 
{

    // ...
    // Please insert system specific code to further process or display the BSEC outputs
    // ...
    
	PressureEventT Param;
	Param.temperature = temperature;
	Param.humidity = humidity;
	Param.pressure = pressure;
	Param.gas = gas;
	Param.iaq = iaq;

	if(pInitDev->fnPressureCallback)	
	{
		pInitDev->fnPressureCallback(&Param);
	}
	
}


/*!
 * @brief           Load previous library state from non-volatile memory
 *
 * @param[in,out]   state_buffer    buffer to hold the loaded state string
 * @param[in]       n_buffer        size of the allocated state buffer
 *
 * @return          number of bytes copied to state_buffer
 */
static uint32_t bme680_state_load(uint8_t *state_buffer, uint32_t n_buffer) 
{
    // ...
    // Load a previous library state from non-volatile memory, if available.
    //
    // Return zero if loading was unsuccessful or no state was available, 
    // otherwise return length of loaded state string.
    // ...
    return 0;
}

/*!
 * @brief           Save library state to non-volatile memory
 *
 * @param[in]       state_buffer    buffer holding the state to be stored
 * @param[in]       length          length of the state string to be stored
 *
 * @return          none
 */
static void bme680_state_save(const uint8_t *state_buffer, uint32_t length) 
{
    // ...
    // Save the string some form of non-volatile memory, if possible.
    // ...
}

/*!
 * @brief           Load library config from non-volatile memory
 *
 * @param[in,out]   config_buffer    buffer to hold the loaded state string
 * @param[in]       n_buffer        size of the allocated state buffer
 *
 * @return          number of bytes copied to config_buffer
 */
static uint32_t bme680_config_load(uint8_t *config_buffer, uint32_t n_buffer) 
{
    // ...
    // Load a library config from non-volatile memory, if available.
    //
    // Return zero if loading was unsuccessful or no config was available, 
    // otherwise return length of loaded config string.
    // ...
    return 0;
}

void Bme680Task(void* arg) 
{
	DrvPressureDevT *pPressureDev = (DrvPressureDevT *)arg;
	if(pPressureDev == NULL) 
	{
		return;
	}

	if(pPressureDev != NULL) 
	{
		pInitDev = pPressureDev;
	}
		
	/* Call to endless loop function which reads and processes data based on sensor settings */
	/* State is saved every 10.000 samples, which means every 10.000 * 3 secs = 500 minutes  */
	bsec_iot_loop(bme680_sleep, bme680_get_timestamp_us, bme680_output_ready, bme680_state_save, pPressureDev->ScanPeriod);

	return;

}

void InitPressure (DrvPressureDevT *pPressureDev) 
{

	return_values_init ret;

	i2c_driver_initialize();

	ret = bsec_iot_init(BSEC_SAMPLE_RATE_LP,
				   0.0f,
				   bme680_bus_write,
				   bme680_bus_read,
				   bme680_sleep,
				   bme680_state_load,
				   bme680_config_load);
	
	if (ret.bme680_status || ret.bsec_status) 
	{
		/* Could not intialize BME680 */
		printf("%d: Could not intialize BME680: bme status(%d), bsec status(%d): \n", __LINE__,(int)ret.bme680_status, (int)ret.bsec_status);
	
	}
	else
	{
		xTaskCreatePinnedToCore(Bme680Task, "Pressure Task", 4096, (void *)pPressureDev, 5, NULL, 1);
	}

	return;
}

