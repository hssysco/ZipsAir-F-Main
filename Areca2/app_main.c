#include <stdio.h>
#include <string.h>
//#include <freertos/FreeRTOS.h>
//#include <freertos/task.h>
//#include <esp_system.h>
//#include <esp_spi_flash.h>
//#include <esp_spiffs.h>
//#include <esp_vfs.h>
//

//#include <nvs_flash.h>
//#include <mqtt_client.h>
#include <nvs.h>
#include "argtable3.h"

#include "app.h"
#include <esp_console.h>
#include "i2c.h"
#include "at24cxx.h"
#include "gd32f30x.h"

#define MAX_CO2_VAL     9999
#define MIN_CO2_VAL     400

#define MAX_PM_VAL      999
#define MIN_PM_VAL      1

#define SW_VER_MAJOR    0
#define SW_VER_MINOR    9
#define SW_VER_PATCH    6

PersistDataInfoT        PersistDataInfo;
SystemInfoT             SystemInfo;
WifiInfoT               WifiInfo;
SensorInfoT             SensorInfo;

AboveTxInfoT            AboveTxInfo;
AboveRxInfoT            AboveRxInfo;
CommInfoT               CommInfo;
MqttInfoT               MqttInfo;

DrvDustDevT             DustDev;
DrvCo2DevT              Co2Dev;
DrvPressureDevT         PressureDev;
DrvRmtDevT              RmtDev;
DrvGpioDevT 		GpioDev;

static struct
{
    struct arg_str *serial;
    struct arg_end *end;
}serial_args;

static struct
{
    struct arg_str *ssid;
    struct arg_end *end;
}ssid_args;

static struct
{
    struct arg_str *pwd;
    struct arg_end *end;
}pwd_args;

static struct
{
    struct arg_int *netid;
    struct arg_end *end;
}netid_args;

static struct
{
    struct arg_int *roomid;
    struct arg_end *end;
}roomid_args;

uint16_t Uart_TxTime = 0;

extern void gpio_check_task();
extern FlagStatus gd_key_state_get(key_typedef_enum key);
extern void delay_1ms(uint32_t count);
extern void PIDTask();
extern void AbovCommTask();
extern void ThermostatCommTask();
extern void AnalysisTaskInit();
extern void AnalysisTask();
extern void FactoryTask();
extern void Cm1106Task(void* arg);
extern void Pm2008Task(void* arg);
extern void RmtRXTask(void* arg);

void software_reset(void)
{
    /* set FAULTMASK */
    __set_FAULTMASK(1);
    NVIC_SystemReset();
}

void SensorDustCallback(DustEventT *pEvt) 
{
	if (SensorInfo.pm1_0!=(int)pEvt->pm_1_0) 
	{
		if (pEvt->pm_1_0 > MIN_PM_VAL && pEvt->pm_1_0 < MAX_PM_VAL)
		{
			SensorInfo.pm1_0 = (int)pEvt->pm_1_0;
		}
	}
	
	if (SensorInfo.pm10_0!=(int)pEvt->pm_10_0) 
	{
		if (pEvt->pm_10_0 > MIN_PM_VAL && pEvt->pm_10_0 < MAX_PM_VAL)
		{
			SensorInfo.pm10_0 = (int)pEvt->pm_10_0;
		}
	}
	
	if (SensorInfo.pm2_5!=(int)pEvt->pm_2_5) 
	{
		if (pEvt->pm_2_5 > MIN_PM_VAL && pEvt->pm_2_5 < MAX_PM_VAL)
		{
			SensorInfo.pm2_5 = (int)pEvt->pm_2_5;
		}
	}

	return;
}

void SensorCo2Callback(int Co2_value) 
{

	if (Co2_value < MIN_CO2_VAL) Co2_value = MIN_CO2_VAL;
	else if (Co2_value > MAX_CO2_VAL) Co2_value = MAX_CO2_VAL;

	if (SensorInfo.co2!=(int)Co2_value) 
	{
		SensorInfo.co2 = (int)Co2_value;
	}

	return;
}

void SensorPressureCallback(PressureEventT *pEvt) 
{
	int temp_int;
	float temp_real;

	/* adjustment */
	temp_real = pEvt->temperature*0.8;

	 /* use %0.1f */
	temp_int = (int)(temp_real*10);
	temp_real = (float)temp_int/10;

	if (SensorInfo.temp != temp_real) 
	{
		SensorInfo.temp = temp_real;
	}

	if (SensorInfo.smell_iaq != (int)pEvt->iaq) 
	{
		SensorInfo.smell_iaq = (int)pEvt->iaq;
	}

	if (SensorInfo.humidity != (int)pEvt->humidity) 
	{
		SensorInfo.humidity = (int)pEvt->humidity;
	}

	if (SensorInfo.pressure != pEvt->pressure) 
	{
		SensorInfo.pressure = pEvt->pressure;
	}
	
	if (SensorInfo.gas != pEvt->gas) 
	{
		SensorInfo.gas = pEvt->gas;
	}

	return;
}

void CheckGpioCallback(GpioEventT event)
{
	static unsigned int mode = 0;
	static unsigned char level = 0;;
	static int OldDoorStatus = COVER_CLOSED;

	if((event == COVER_CLOSED) && (OldDoorStatus != COVER_CLOSED))
	{
		AboveRxInfo.Err &= ~(OPENED_COVER_ERR);

		if((mode == CTRL_MODE_MANUAL)&&(level == 1))
		{
			CtrlMode(CTRL_MODE_SLEEP);
		}
		else
		{
			CtrlMode(mode);
			CtrlFan(level);
		}

		OldDoorStatus = COVER_CLOSED;
	}
	else if((event == COVER_OPEN) && (OldDoorStatus != COVER_OPEN))
	{
		AboveRxInfo.Err |= OPENED_COVER_ERR;

		mode = AboveTxInfo.Mode;
		level = AboveTxInfo.FanLevel;
		CtrlMode(0);

		OldDoorStatus = COVER_OPEN;
	}
	else if(event == BUTTON_1)
	{
		DefaultSetting(false);

        delay_1ms(200);
		software_reset();
	}

	return;
}

int ShowSensorInfo(int argc, char **argv) 
{

	printf("# Co2 co2(%d) co2Level(%d) \r\n", SensorInfo.co2, SensorInfo.co2_level );
	printf("# Dust selected_pm(%d) pm1_0(%d) pm2_5(%d) pm10_0(%d) pm_level(%d) \r\n", SensorInfo.selected_pm, SensorInfo.pm1_0, SensorInfo.pm2_5, SensorInfo.pm10_0, SensorInfo.pm_level);
	printf("# Pressure temp(%f) pressure(%f) smell_iaq(%d) smell_level(%d) gas(%f) humidity(%d) \r\n", SensorInfo.temp, SensorInfo.pressure, SensorInfo.smell_iaq, SensorInfo.smell_level, SensorInfo.gas, SensorInfo.humidity);

	return 0;
}

void Register_GetSensorInfo(void) 
{
    const esp_console_cmd_t cmd = {
        .command = "sensor",
        .help = "Get co2 info",
        .hint = NULL,
        .func = &ShowSensorInfo,
		.argtable = NULL
    };
//    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}

static void InitNVS (void) 
{
    /* enable GPIOB clock */
    rcu_periph_clock_enable(RCU_GPIOB);
    
    /* enable I2C0 clock */
    rcu_periph_clock_enable(RCU_I2C0);
    rcu_periph_clock_enable(RCU_AF);

    gpio_init(GPIOB, GPIO_MODE_AF_OD, GPIO_OSPEED_50MHZ, GPIO_PIN_8);
    gpio_init(GPIOB, GPIO_MODE_AF_OD, GPIO_OSPEED_50MHZ, GPIO_PIN_9);
    gpio_pin_remap_config(GPIO_I2C0_REMAP,ENABLE);

    i2c_deinit(I2C0);
    /* configure I2C clock */
    i2c_clock_config(I2C0,I2C0_SPEED,I2C_DTCY_2);
    /* configure I2C address */
    i2c_mode_addr_config(I2C0,I2C_I2CMODE_ENABLE,I2C_ADDFORMAT_7BITS,I2C0_SLAVE_ADDRESS7);
    /* enable I2C0 */
    i2c_enable(I2C0);
    /* enable acknowledge */
    i2c_ack_config(I2C0,I2C_ACK_ENABLE);
    

    /* initialize EEPROM  */
    EEP_i2c_eeprom_init();
//    i2c_24c02_test();
}

void InitSys (void) 
{
	/* LED */
	InitLed();

	/* Wifi station mode */
//	InitWifiStation((unsigned char *)PersistDataInfo.SSID, (unsigned char *)PersistDataInfo.PWD);

	InitCheckGpio();

#ifndef CONFIG_CUBIC_ENABLE
//	InitAudio();
#endif	

	InitSerialDriver();
    InitSerilInterrupt();

	OffAllLed();

	InitConsole();

	/* Remote controller */
	RmtDev.fnRmtCallback = RmtCallback;
	InitRemote(&RmtDev);


    
    //ret = gd_key_state_get(KEY_REV0);
    if(gd_key_state_get(KEY_REV0) == SET)
        SystemInfo.HasSensors = 1;
    else
        SystemInfo.HasSensors = 0; 

	if(SystemInfo.HasSensors)
	{
		Co2Dev.ScanPeriod = 3000;
		Co2Dev.fnCo2Callback = SensorCo2Callback;
		InitCo2(&Co2Dev);

		DustDev.fnDustCallback = SensorDustCallback;
		DustDev.mAdjMode = ADJ_MODE_GRIMM;
		DustDev.ScanPeriod = 3000;
		InitDust(&DustDev);
        

		PressureDev.ScanPeriod = 3000;
		PressureDev.fnPressureCallback = SensorPressureCallback;
//		InitPressure(&PressureDev);
	}
}

void LoadNvsData (void) 
{
	int Ret = 0;
	char StrBuf[128] = {0,};
	unsigned char	Value8 = 0;
	unsigned int 	Value32 = 0;
	int datalen = 0;

	Ret = GetValueU8(PROP_NAME_SYS_ROOMID, &Value8);
	if(Ret == 0) 
	{
		if(Value8 != PersistDataInfo.DeviceId) 
		{
			PersistDataInfo.DeviceId = Value8;
		}
	}

	Ret = GetValueStr(PROP_NAME_SYS_SERIAL, StrBuf);
	if(Ret == 0) 
	{
		datalen = strlen(StrBuf);
		if(datalen > 0) 
		{
			if( strcmp(StrBuf,PersistDataInfo.Serial) != 0) 
			{
			
				memset(PersistDataInfo.Serial, 0,sizeof(PersistDataInfo.Serial));
				strncpy(PersistDataInfo.Serial,StrBuf,datalen);
			}
		}
	}

	memset(StrBuf, 0, sizeof(StrBuf));
	Ret = GetValueStr(PROP_NAME_SYS_SSID, StrBuf);
	if(Ret == 0) 
	{
		datalen = strlen(StrBuf);
		if(datalen > 0) 
		{
			if(strcmp(StrBuf,PersistDataInfo.SSID) != 0) 
			{
				memset(PersistDataInfo.SSID, 0,sizeof(PersistDataInfo.SSID));
				strncpy(PersistDataInfo.SSID,StrBuf,datalen);
			}
		}
	}

	memset(StrBuf, 0, sizeof(StrBuf));
	Ret = GetValueStr(PROP_NAME_SYS_PWD, StrBuf);
	if(Ret == 0) 
	{
		datalen = strlen(StrBuf);
		if(datalen > 0) 
		{
			if( strcmp(StrBuf, PersistDataInfo.PWD) != 0) 
			{
				memset(PersistDataInfo.PWD, 0,sizeof(PersistDataInfo.PWD));
				strncpy(PersistDataInfo.PWD,StrBuf,datalen);
			}
		}
	}

	memset(StrBuf, 0, sizeof(StrBuf));
	Ret = GetValueStr(PROP_NAME_SYS_MODULEID, StrBuf);
	if(Ret == 0) 
	{
		datalen = strlen(StrBuf);
		if(datalen > 0) 
		{
			if( strcmp(StrBuf, PersistDataInfo.ModuleID) != 0) 
			{
				memset(PersistDataInfo.ModuleID, 0,sizeof(PersistDataInfo.ModuleID));			
				strncpy(PersistDataInfo.ModuleID,StrBuf,datalen);
			}
		}
	}

	Value32 = 0;
	Ret = GetValueU32(PROP_NAME_SYS_NETID, &Value32);
	if(Ret == 0)
	{
		if(Value32 != PersistDataInfo.NetId)
		{
			PersistDataInfo.NetId = Value32;
		}
	}

	Value32 = 0;
	Ret = GetValueU32(PROP_NAME_SYS_FAU_FLTPRSLMT, &Value32);
	if(Ret == 0) 
	{
		if(Value32 != PersistDataInfo.fltdifprslmt)
		{
			PersistDataInfo.fltdifprslmt = Value32;
		}
	}

	Value8 = 0;
	Ret = GetValueU8(PROP_NAME_SYS_FACTORY_DONE, &Value8);
	if(Ret == 0) 
	{
		if(Value8 != PersistDataInfo.FactoryDone) 
		{
			PersistDataInfo.FactoryDone = Value8;
		}
	}

	Value8 = 0;
	Ret = GetValueU8(PROP_NAME_SYS_FAU_MOD_NORMAL, &Value8);
	if(Ret == 0) 
	{
		if(Value8 != PersistDataInfo.FauNormalMode) 
		{
			PersistDataInfo.FauNormalMode = Value8;
		}
	}

	Value8 = 0;
	Ret = GetValueU8(PROP_NAME_APP_UPGRADE, &Value8);
	if(Ret == 0) 
	{
		if(Value8 != PersistDataInfo.UpgradeOn) 
		{
			PersistDataInfo.UpgradeOn = Value8;
		}
	}

	memset(StrBuf, 0, sizeof(StrBuf));
	Ret = GetValueStr(PROP_NAME_UG_HOST, StrBuf);
	if(Ret == 0) 
	{
		datalen = strlen(StrBuf);
		if(datalen > 0) 
		{
			if( strcmp(StrBuf,PersistDataInfo.UgHost) != 0) 
			{
				memset(PersistDataInfo.UgHost, 0,sizeof(PersistDataInfo.UgHost));
				strncpy(PersistDataInfo.UgHost,StrBuf,datalen);
			}
		}
	}

	Value32 = 0;
	Ret = GetValueU32(PROP_NAME_UG_PORT, &Value32);
	if(Ret == 0) 
	{
		if(Value32 != PersistDataInfo.UgPort) 
		{
			PersistDataInfo.UgPort = Value32;
		}
	}

	memset(StrBuf, 0, sizeof(StrBuf));
	Ret = GetValueStr(PROP_NAME_UG_DIR, StrBuf);
	if(Ret == 0) 
	{
		datalen = strlen(StrBuf);
		if(datalen > 0) 
		{
			if( strcmp(StrBuf,PersistDataInfo.UgDir) != 0) 
			{
				memset(PersistDataInfo.UgDir, 0,sizeof(PersistDataInfo.UgDir));
				strncpy(PersistDataInfo.UgDir,StrBuf,datalen);
			}
		}
	}

	memset(StrBuf, 0, sizeof(StrBuf));
	Ret = GetValueStr(PROP_NAME_UG_ID, StrBuf);
	if(Ret == 0) 
	{
		datalen = strlen(StrBuf);
		if(datalen > 0) 
		{
			if( strcmp(StrBuf,PersistDataInfo.UgId) != 0) 
			{
				memset(PersistDataInfo.UgId, 0,sizeof(PersistDataInfo.UgId));
				strncpy(PersistDataInfo.UgId,StrBuf,datalen);
			}
		}
	}

	memset(StrBuf, 0, sizeof(StrBuf));
	Ret = GetValueStr(PROP_NAME_UG_PWD, StrBuf);
	if(Ret == 0) 
	{
		datalen = strlen(StrBuf);
		if(datalen > 0) 
		{
			if( strncmp(StrBuf, PersistDataInfo.UgPwd, datalen) != 0) 
			{
				memset(PersistDataInfo.UgPwd, 0,sizeof(PersistDataInfo.UgPwd));
				strncpy(PersistDataInfo.UgPwd,StrBuf,datalen);
			}
		}
	}

	Value8 = 0;
	Ret = GetValueU8(PROP_NAME_SYS_BCAST_ENCRT, &Value8);
	if(Ret == 0) 
	{
		if(Value8 != PersistDataInfo.BcastEncryption) 
		{
			PersistDataInfo.BcastEncryption = Value8;
		}
	}

	Ret = GetValueU32(PROP_NAME_RPM_1, &Value32);
	if(Ret == 0) 
	{
		if(Value32 != PersistDataInfo.rpm1) 
		{
			PersistDataInfo.rpm1 = Value32;
		}
	}

	Ret = GetValueU32(PROP_NAME_RPM_2, &Value32);
	if(Ret == 0) 
	{
		if(Value32 != PersistDataInfo.rpm2) 
		{
			PersistDataInfo.rpm2 = Value32;
		}
	}

	Ret = GetValueU32(PROP_NAME_RPM_3, &Value32);
	if(Ret == 0) 
	{
		if(Value32 != PersistDataInfo.rpm3) 
		{
			PersistDataInfo.rpm3 = Value32;
		}
	}

	Ret = GetValueU32(PROP_NAME_RPM_4, &Value32);
	if(Ret == 0) 
	{
		if(Value32 != PersistDataInfo.rpm4) 
		{
			PersistDataInfo.rpm4 = Value32;
		}
	}

	Ret = GetValueU32(PROP_NAME_RPM_5, &Value32);
	if(Ret == 0) 
	{
		if(Value32 != PersistDataInfo.rpm5) 
		{
			PersistDataInfo.rpm5 = Value32;
		}
	}

	SystemInfo.SWVer = (SW_VER_MAJOR<<16) | (SW_VER_MINOR<<8) | SW_VER_PATCH;
	SystemInfo.MqttUgMode = 0;
	SystemInfo.MqttUgSendPktCnt = 0;
	SystemInfo.IsCertFileValid = false;

	SensorInfo.temp = TEMP_INITIAL_VALUE;
	SensorInfo.co2 = CO2_INITIAL_VALUE;
	SensorInfo.co2_level = 1;
	SensorInfo.pm1_0 = PM_INITIAL_VALUE;
	SensorInfo.pm2_5 = PM_INITIAL_VALUE;
	SensorInfo.pm10_0 = PM_INITIAL_VALUE;
	SensorInfo.pm_level = 1;
	SensorInfo.selected_pm = PM_2_5;
	SensorInfo.pm_sync = 1;
	SensorInfo.humidity = HUMIDITY_INITIAL_VALUE;
	SensorInfo.smell_iaq = IAQ_INITIAL_VALUE;
	
	CommInfo.Sync = 0;
	CommInfo.SyncWired = 0;
	CommInfo.VSPSet = 0;
	CommInfo.RPMSet = 0;

	AboveTxInfo.Power = 0;
	AboveTxInfo.FanLevel = 0;
	AboveTxInfo.Mode = OP_MODE_OFF;
	
	AboveTxInfo.FltTmrRst = 0;
	AboveTxInfo.FltTmr = 0;
	AboveTxInfo.FltTmrLmt = PersistDataInfo.fltdifprslmt;


	AboveTxInfo.RPM[0] = PersistDataInfo.rpm1;
	AboveTxInfo.RPM[1] = PersistDataInfo.rpm2;
	AboveTxInfo.RPM[2] = PersistDataInfo.rpm3;
	AboveTxInfo.RPM[3] = PersistDataInfo.rpm4;
	AboveTxInfo.RPM[4] = PersistDataInfo.rpm5;
	AboveTxInfo.RPMSet = 0;

#if 0
	AboveTxInfo.VSP[0] = PID_VSP1;
	AboveTxInfo.VSP[1] = PID_VSP2;
	AboveTxInfo.VSP[2] = PID_VSP3;
	AboveTxInfo.VSP[3] = PID_VSP4;
	AboveTxInfo.VSP[4] = PID_VSP5;
#else
	AboveTxInfo.VSP[0] = PPS2VSP(PersistDataInfo.rpm1);
	AboveTxInfo.VSP[1] = PPS2VSP(PersistDataInfo.rpm2);
	AboveTxInfo.VSP[2] = PPS2VSP(PersistDataInfo.rpm3);
	AboveTxInfo.VSP[3] = PPS2VSP(PersistDataInfo.rpm4);
	AboveTxInfo.VSP[4] = PPS2VSP(PersistDataInfo.rpm5);
#endif
	
	AboveTxInfo.VSPOffset = FAU_DEFAULT_VSP_OFFSET;
	AboveTxInfo.Led = 0;

	memset(AboveRxInfo.Serial, 0, MAX_SERIAL_STR_LEN);
	AboveRxInfo.Ver = -1;
	
	AboveRxInfo.Power = 0;
	AboveRxInfo.FanLevel = 0;
	AboveRxInfo.Mode = OP_MODE_OFF;
	
	AboveRxInfo.FltTmrRst = 0;
	AboveRxInfo.FltTmr = 0;
	AboveRxInfo.FltTmrLmt = PersistDataInfo.fltdifprslmt;

	AboveRxInfo.VSP[0] = PID_VSP1;
	AboveRxInfo.VSP[1] = PID_VSP2;
	AboveRxInfo.VSP[2] = PID_VSP3;
	AboveRxInfo.VSP[3] = PID_VSP4;
	AboveRxInfo.VSP[4] = PID_VSP5;
	AboveRxInfo.VSPOffset = FAU_DEFAULT_VSP_OFFSET;
	AboveRxInfo.Err = 0;

	return;
}

void InitData (void) 
{
	
	memset(&(PersistDataInfo),0,sizeof(PersistDataInfoT));
	memset(&(SystemInfo),0,sizeof(SystemInfoT));
	memset(&(WifiInfo),0,sizeof(WifiInfoT));

	PersistDataInfo.DeviceId = 0;

	snprintf(PersistDataInfo.SSID, sizeof(PersistDataInfo.SSID),"%s",DEFAULT_SETUP_SSID);
	snprintf(PersistDataInfo.PWD, sizeof(PersistDataInfo.PWD), "%s",DEFAULT_SETUP_PWD);
	snprintf(PersistDataInfo.ModuleID ,sizeof(PersistDataInfo.ModuleID),"%s",ESP32_MOUDLE_ID);
	snprintf(SystemInfo.Version, sizeof(SystemInfo.Version),"ARECA-%02d.%02d.%02d",SW_VER_MAJOR, SW_VER_MINOR, SW_VER_PATCH);
	
	PersistDataInfo.Volume = 10;
	PersistDataInfo.fltdifprslmt = MAX_FILTER_USE_TIME;
	PersistDataInfo.FauNormalMode = 1;
	PersistDataInfo.Mqtt = 0;
	PersistDataInfo.BcastEncryption = 1;
	PersistDataInfo.NetId = DEFAULT_NETWORK_ID;

	PersistDataInfo.rpm1 = PID_TARGET_PPS1;
	PersistDataInfo.rpm2 = PID_TARGET_PPS2;
	PersistDataInfo.rpm3 = PID_TARGET_PPS3;
	PersistDataInfo.rpm4 = PID_TARGET_PPS4;
	PersistDataInfo.rpm5 = PID_TARGET_PPS5;

	SystemInfo.ResvTimerInstance = -1;

	printf("ESP FW version : %s\n", SystemInfo.Version);
}

void DefaultSetting(int needToLoadNvs)
{
	int Ret = 0;

	InitData();

	Ret = SetValueU8(PROP_NAME_SYS_ROOMID, PersistDataInfo.DeviceId);
	printf("### DefaultSetting RoomID (%d:%d) \r\n", Ret, PersistDataInfo.DeviceId);

	Ret = SetValueStr(PROP_NAME_SYS_SSID, DEFAULT_SETUP_SSID);
	printf("### DefaultSetting SSID (%d) \r\n", Ret);

	Ret = SetValueStr(PROP_NAME_SYS_PWD, DEFAULT_SETUP_PWD);
	printf("### DefaultSetting PWD (%d) \r\n", Ret);

	Ret = SetValueU32(PROP_NAME_SYS_NETID, DEFAULT_NETWORK_ID);
	printf("### DefaultSetting NETWORK ID (%d:%d) \r\n", Ret, PersistDataInfo.NetId);

	Ret = SetValueU32(PROP_NAME_RPM_1, PersistDataInfo.rpm1);
	printf("### DefaultSetting PROP_NAME_RPM_1 (%d:%d) \r\n", Ret, PersistDataInfo.rpm1);

	Ret = SetValueU32(PROP_NAME_RPM_2, PersistDataInfo.rpm2);
	printf("### DefaultSetting PROP_NAME_RPM_2 (%d:%d) \r\n", Ret, PersistDataInfo.rpm2);

	Ret = SetValueU32(PROP_NAME_RPM_3, PersistDataInfo.rpm3);
	printf("### DefaultSetting PROP_NAME_RPM_3 (%d:%d) \r\n", Ret, PersistDataInfo.rpm3);

	Ret = SetValueU32(PROP_NAME_RPM_4, PersistDataInfo.rpm4);
	printf("### DefaultSetting PROP_NAME_RPM_4 (%d:%d) \r\n", Ret, PersistDataInfo.rpm4);

	Ret = SetValueU32(PROP_NAME_RPM_5, PersistDataInfo.rpm5);
	printf("### DefaultSetting PROP_NAME_RPM_5 (%d:%d) \r\n", Ret, PersistDataInfo.rpm5);

	if(needToLoadNvs)
		LoadNvsData();

	return;
}

void SetId(unsigned char Id)
{
	int16_t Ret = 0;
	unsigned char	Value8 = 0;

//	Ret = SetValueU8(PROP_NAME_SYS_ROOMID, Id);
	printf("### DefaultSetting RoomID (%d) \r\n", Ret);

	Ret = GetValueU8(PROP_NAME_SYS_ROOMID, &Value8);
	if(Ret == 0) 
	{
		if(PersistDataInfo.DeviceId != Value8) 
		{
			PersistDataInfo.DeviceId = Value8;
		}
	}

	return;
}

void SetSSID( char *pSSID)
{
	int Ret = 0;
	char StrBuf[128] = {0,};
	int datalen = 0;

	if(pSSID == NULL)
	{
		return;
	}

//	Ret = SetValueStr(PROP_NAME_SYS_SSID, pSSID);
	printf("### DefaultSetting SSID (%d:%s) \r\n", Ret, pSSID);

	memset(StrBuf, 0, sizeof(StrBuf));
	Ret = GetValueStr(PROP_NAME_SYS_SSID, StrBuf);
	if(Ret == 0) 
	{
		datalen = strlen(StrBuf);
		if(datalen > 0) 
		{
			if(strcmp(StrBuf, PersistDataInfo.SSID) != 0) 
			{
				memset(PersistDataInfo.SSID, 0,sizeof(PersistDataInfo.SSID));
				strncpy(PersistDataInfo.SSID, StrBuf, datalen);
			}
		}
	}

	return;
}

void SetPWD( char *pPWD)
{
	int Ret = 0;
	char StrBuf[128] = {0,};
	int datalen = 0;

	if(pPWD == NULL)
	{
		return;
	}

//	Ret = SetValueStr(PROP_NAME_SYS_PWD, pPWD);
	printf("### DefaultSetting PWD (%d:%s) \r\n", Ret, pPWD);

	memset(StrBuf, 0, sizeof(StrBuf));
	Ret = GetValueStr(PROP_NAME_SYS_PWD, StrBuf);
	if(Ret == 0) 
	{
		datalen = strlen(StrBuf);
		if(datalen > 0) 
		{
			if(strcmp(StrBuf, PersistDataInfo.PWD) != 0) 
			{
				memset(PersistDataInfo.PWD, 0,sizeof(PersistDataInfo.PWD));
				strncpy(PersistDataInfo.PWD, StrBuf, datalen);
			}
		}
	}

	return;
}

unsigned char PPS2VSP (unsigned int pps)
{
	uint8_t result = 0;
	uint16_t rpm = 0;

	if(pps < 93)
	{
		return 0;
	}

	if (pps <= 100)
	{
		result = (2.6*20);
		return result;
	}
	else if (pps <= 114)	
	{
		result = (2.7*20);
		return result;
	}
	else if (pps <= 122)
	{
		result = (2.8* 20);
		return result;
	}
	else if (pps <= 132)
	{
		result  =  (2.9 * 20);
		return result;
	}
	else if (pps <= 143)
	{
		result  =  (3.0 * 20);
		return result;
	}
	else if (pps <= 154)
	{
		result  =  (3.1 * 20);
		return result;	
	}
	else if (pps <= 165)
	{
		result  =  (3.2 * 20);
		return result;	
	}
	else if (pps <= 175)
	{
		result	=  (3.3 * 20);
		return result;	
	}
	else if (pps <= 184)
	{
		result	=  (3.4 * 20);
		return result;	
	}
	else if (pps <= 195)
	{
		result	=  (3.5 * 20);
		return result;	
	}
	else if (pps <= 204)
	{
		result	=  (3.6 * 20);
		return result;	
	}
	else if (pps <= 213)
	{
		result	=  (3.7 * 20);
		return result;	
	}
	else if (pps <= 222)
	{
		result =  (3.8 * 20);
		return result;	
	}
	else if (pps <= 231)	
	{
		result =  (3.9 * 20);
		return result;	
	}
	else if (pps <= 239)
	{
		result =  (4.0 * 20);
		return result;	
	}
	else if (pps > 239)
	{
		result =  (4.1 * 20);
		return result;	
	}

	return result;
	
}

void SetRPM(unsigned int index, unsigned int rpm)
{
	int Ret = 0;
	unsigned int	Value32 = 0;
	
	if(index > 5)
	{
		return;
	}

	switch (index)
	{
		case 1:
			Ret = SetValueU32(PROP_NAME_RPM_1, rpm);
			printf("### DefaultSetting RPM1 (%d:%d) \r\n", index, Ret);

			Ret = GetValueU32(PROP_NAME_RPM_1, &Value32);
			if(Ret == 0) 
			{
				if(PersistDataInfo.rpm1 != Value32) 
				{
					PersistDataInfo.rpm1 = Value32;
				}
			}

			break;
			
		case 2:
			Ret = SetValueU32(PROP_NAME_RPM_2, rpm);
			printf("### DefaultSetting RPM2 (%d:%d) \r\n", index, Ret);
		
			Ret = GetValueU32(PROP_NAME_RPM_2, &Value32);
			if(Ret == 0) 
			{
				if(PersistDataInfo.rpm2 != Value32) 
				{
					PersistDataInfo.rpm2 = Value32;
				}
			}
			break;

		case 3:
			Ret = SetValueU32(PROP_NAME_RPM_3, rpm);
			printf("### DefaultSetting RPM3 (%d:%d) \r\n", index, Ret);
		
			Ret = GetValueU32(PROP_NAME_RPM_3, &Value32);
			if(Ret == 0) 
			{
				if(PersistDataInfo.rpm3 != Value32) 
				{
					PersistDataInfo.rpm3 = Value32;
				}
			}

			break;

		case 4:
			Ret = SetValueU32(PROP_NAME_RPM_4, rpm);
			printf("### DefaultSetting RPM4 (%d:%d) \r\n", index, Ret);
		
			Ret = GetValueU32(PROP_NAME_RPM_4, &Value32);
			if(Ret == 0) 
			{
				if(PersistDataInfo.rpm4 != Value32) 
				{
					PersistDataInfo.rpm4 = Value32;
				}
			}
			break;

		case 5:
			Ret = SetValueU32(PROP_NAME_RPM_5, rpm);
			printf("### DefaultSetting RPM5 (%d:%d) \r\n", index, Ret);
		
			Ret = GetValueU32(PROP_NAME_RPM_5, &Value32);
			if(Ret == 0) 
			{
				if(PersistDataInfo.rpm5 != Value32) 
				{
					PersistDataInfo.rpm5 = Value32;
				}
			}
			break;
			
		default:
			break;

	}

	return;
}

void GetSystemInfo( SystemInfoT **ppSystemInfo ) 
{
	*ppSystemInfo = &(SystemInfo);
	return;
}

void GetPersistDataInfo( PersistDataInfoT **ppPersistDataInfo ) 
{
	*ppPersistDataInfo = &(PersistDataInfo);
	return;
}

void GetWifiInfo( WifiInfoT **ppWifiInfo ) 
{
	*ppWifiInfo = &(WifiInfo);
	return;
}

void GetAbovTxInfo( AboveTxInfoT **ppData ) 
{
	*ppData = &(AboveTxInfo);
	return;
}

void GetAbovRxInfo( AboveRxInfoT **ppData ) 
{
	*ppData = &(AboveRxInfo);
	return;
}

void GetCommInfo( CommInfoT **ppData ) 
{
	*ppData = &(CommInfo);
	return;
}

void GetSensorInfo( SensorInfoT **ppSensorInfo ) 
{
	*ppSensorInfo = &(SensorInfo);
	return;
}


static int CommandSerial(int argc, char **argv)
{
	int Ret = 0;
	int nerrors = arg_parse(argc, argv, (void **)&serial_args);

	if (nerrors != 0)
	{
		arg_print_errors(stderr, serial_args.end, argv[0]);
		return 1;
	}

//	Ret = SetValueStr(PROP_NAME_SYS_SERIAL, (char *)serial_args.serial->sval[0]);
	printf("serial : %s -> %s\n", PersistDataInfo.Serial, serial_args.serial->sval[0]);
	memset(PersistDataInfo.Serial, '\0', MAX_SERIAL_STR_LEN);
//	Ret = SetValueU8(PROP_NAME_SYS_FACTORY_DONE, 1);

	return Ret;
}

static int CommandDelSerial(int argc, char **argv)
{
	int16_t Ret = 0;

//	Ret = Erase(PROP_NAME_SYS_SERIAL);
//	Ret = Erase(PROP_NAME_SYS_FACTORY_DONE);
	printf("Delete serial and factory done\n");

	return Ret;
}

static int CommandSsid(int argc, char **argv)
{
	int16_t Ret = 0;
	int nerrors = arg_parse(argc, argv, (void **)&ssid_args);

	if (nerrors != 0)
	{
		arg_print_errors(stderr, ssid_args.end, argv[0]);
		return 1;
	}

//	Ret = SetValueStr(PROP_NAME_SYS_SSID, (char *)ssid_args.ssid->sval[0]);
	printf("ssid : %s -> %s\n", PersistDataInfo.SSID, ssid_args.ssid->sval[0]);
//
	return Ret;
}

static int CommandPwd(int argc, char **argv)
{
	int Ret = 0;
	int nerrors = arg_parse(argc, argv, (void **)&pwd_args);

	if (nerrors != 0)
	{
		arg_print_errors(stderr, pwd_args.end, argv[0]);
		return 1;
	}

//	Ret = SetValueStr(PROP_NAME_SYS_PWD, (char *)pwd_args.pwd->sval[0]);
	printf("pwd : %s -> %s\n", PersistDataInfo.PWD, pwd_args.pwd->sval[0]);

	return Ret;
}

static int CommandNetid(int argc, char **argv)
{
	int16_t Ret = 0;
	int nerrors = arg_parse(argc, argv, (void **)&netid_args);

	if (nerrors != 0)
	{
		arg_print_errors(stderr, netid_args.end, argv[0]);
		return 1;
	}

//	Ret = SetValueU32(PROP_NAME_SYS_NETID, netid_args.netid->ival[0]);
	printf("netid : %d -> %d\n", PersistDataInfo.NetId, netid_args.netid->ival[0]);

	return Ret;
}

static int CommandRoomId(int argc, char **argv)
{
	int16_t Ret = 0;
	int nerrors = arg_parse(argc, argv, (void **)&roomid_args);

	if (nerrors != 0)
	{
		arg_print_errors(stderr, roomid_args.end, argv[0]);
		return 1;
	}

//	Ret = SetValueU8(PROP_NAME_SYS_ROOMID, roomid_args.roomid->ival[0]);
	printf("room id : %d -> %d\n", PersistDataInfo.DeviceId, roomid_args.roomid->ival[0]);

	return Ret;
}

static int ShowSystemInfo(int argc, char **argv)
{
	char Ip[4] = {0,};

	printf("------- System Info -------\n");
	printf("Version : %s\n", SystemInfo.Version);

	printf("serial  : %s\n", PersistDataInfo.Serial);
	printf("ssid    : %s\n", PersistDataInfo.SSID);
	printf("pwd     : %s\n", PersistDataInfo.PWD);
	printf("net id  : %d\n", PersistDataInfo.NetId);
	printf("room id : %d\n", PersistDataInfo.DeviceId);

	Ip[0] = WifiInfo.FullIp & 0xFF;
	Ip[1] = (WifiInfo.FullIp >> 8) & 0xFF;
	Ip[2] = (WifiInfo.FullIp >> 16) & 0xFF;
	Ip[3] = (WifiInfo.FullIp >> 24) & 0xFF;
	printf("IP      : %d.%d.%d.%d\n", Ip[0], Ip[1], Ip[2], Ip[3]);
	printf("UgHost  : %s\n", PersistDataInfo.UgHost);
	printf("UgPort  : %d\n", PersistDataInfo.UgPort);
	printf("UgDir   : %s\n", PersistDataInfo.UgDir);
	printf("UgId    : %s\n", PersistDataInfo.UgId);
	printf("UgPwd   : %s\n", PersistDataInfo.UgPwd);
	if(SystemInfo.ConnType == (DEV_CONNECTION_WIFI | DEV_CONNECTION_RS485))
		printf("ConnType : WIFI, RS485(%d)\n", SystemInfo.ConnType);
	else if(SystemInfo.ConnType == DEV_CONNECTION_WIFI)
		printf("ConnType : WIFI(%d)\n", SystemInfo.ConnType);
	else if(SystemInfo.ConnType == DEV_CONNECTION_RS485)
		printf("ConnType : RS485(%d)\n", SystemInfo.ConnType);
	else
		printf("ConnType : NONE(%d)\n", SystemInfo.ConnType);

	return 0;
}

void Register_CommandApp(void)
{
	serial_args.serial = arg_str1(NULL, NULL, "<str>", "serial number");
	serial_args.end = arg_end(2);
	ssid_args.ssid = arg_str1(NULL, NULL, "<str>", "wifi ssid");
	ssid_args.end = arg_end(2);
	pwd_args.pwd = arg_str1(NULL, NULL, "<str>", "wifi password");
	pwd_args.end = arg_end(2);
	netid_args.netid = arg_int1(NULL, NULL, "<int>", "network id");
	netid_args.end = arg_end(2);
	roomid_args.roomid = arg_int1(NULL, NULL, "<int>", "room id");
	roomid_args.end = arg_end(2);

	const esp_console_cmd_t cmd_serial =
	{
		.command = "serial",
		.help = "set serial",
		.hint = NULL,
		.func = &CommandSerial,
		.argtable = &serial_args
	};

	const esp_console_cmd_t cmd_delserial =
	{
		.command = "serial_del",
		.help = "delete serial",
		.hint = NULL,
		.func = &CommandDelSerial,
	};

	const esp_console_cmd_t cmd_ssid =
	{
		.command = "ssid",
		.help = "set wifi ssid",
		.hint = NULL,
		.func = &CommandSsid,
		.argtable = &ssid_args
	};

	const esp_console_cmd_t cmd_pwd =
	{
		.command = "pwd",
		.help = "set wifi password",
		.hint = NULL,
		.func = &CommandPwd,
		.argtable = &pwd_args
	};

	const esp_console_cmd_t cmd_netid =
	{
		.command = "netid",
		.help = "set network id",
		.hint = NULL,
		.func = &CommandNetid,
		.argtable = &netid_args
	};

	const esp_console_cmd_t cmd_roomid =
	{
		.command = "roomid",
		.help = "set room id",
		.hint = NULL,
		.func = &CommandRoomId,
		.argtable = &roomid_args
	};

	const esp_console_cmd_t cmd_system =
	{
		.command = "system",
		.help = "show system info",
		.hint = NULL,
		.func = &ShowSystemInfo,
		.argtable = NULL
	};

//    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd_serial) );
//    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd_delserial) );
//    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd_ssid) );
//    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd_pwd) );
//    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd_netid) );
//    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd_roomid) );
//    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd_system) );
}

void app_main() 
{
	/* Print chip information */
//	esp_chip_info_t chip_info;
//	esp_chip_info(&chip_info);
//	printf("This is ESP32 chip with %d CPU cores", chip_info.cores);
//	printf("silicon revision %d, ", chip_info.revision);
//	printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
//		(chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

	InitNVS();
    LoadNvsData();
	InitData();	
	InitSys();
    
//	InitRTC();

	OnLed(LED_POWER);
	InitAbovComms();
	InitThermostatComms();
	InitControlFuncs();
    AnalysisTaskInit();
//	InitWIFIComms();

	if(!PersistDataInfo.FactoryDone)
	{
		printf("Start factory task\n");
//		InitFactory();
	}

//	if(PersistDataInfo.UpgradeOn)
//	{
//		HTTPSUpdateSW();
//	}
    
    while(1) {
        gpio_check_task();
        if(Uart_TxTime >= 100) {
            PIDTask();
            AnalysisTask();
            
            AbovCommTask();
            ThermostatCommTask();
            
            Cm1106Task(&Co2Dev);
            Pm2008Task(&DustDev);
            
//            FactoryTask();
            Uart_TxTime = 0;
        }
        RmtRXTask(&RmtDev);
        
    }
}
