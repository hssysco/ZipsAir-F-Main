#include <stdio.h>
#include <string.h>

//#include <nvs_flash.h>
//#include <mqtt_client.h>
#include <nvs.h>


#include "app.h"
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

//static struct
//{
//    struct arg_str *serial;
//    struct arg_end *end;
//}serial_args;
//
//static struct
//{
//    struct arg_str *ssid;
//    struct arg_end *end;
//}ssid_args;
//
//static struct
//{
//    struct arg_str *pwd;
//    struct arg_end *end;
//}pwd_args;
//
//static struct
//{
//    struct arg_int *netid;
//    struct arg_end *end;
//}netid_args;
//
//static struct
//{
//    struct arg_int *roomid;
//    struct arg_end *end;
//}roomid_args;

uint16_t Motor_CommTime = 0;
uint16_t Uart_TxTime = 0;
uint16_t Uart_RxTime = 0;
uint8_t  Wdt_Outsts = 0;

//extern uint8_t Coordinator = 0;
    
extern void gpio_check_task();
extern FlagStatus gd_key_state_get(key_typedef_enum key);
extern void delay_1ms(uint32_t count);
extern void PIDTask();
extern void Abov_Tx();
extern int Abov_Rx();

extern void Thermostat_Rx();
extern void Zigbee_CodiTx();
extern int Zigbee_CodiRx();
extern void Zigbee_RouterTx();
extern int Zigbee_RouterRx();
extern void AnalysisTaskInit();
extern void AnalysisTask();
extern void FactoryTask();
extern void Cm1106Task(void* arg);
extern void Pm2008Task();
extern void RmtRXInit();
extern void RmtRX();
extern void InitAbovComms(void);
extern void InitZigbeeComms(void);
extern void LED_Ctrl(void);
   
void WdtPort_Init(void);
void InitNVS(void);
void LoadNvsData(void);
void InitData(void);
void InitSys(void) ;


void app_main(void) 
{
//    WdtPort_Init();
    delay_1ms(1000);
	InitNVS();
    LoadNvsData();
	InitData();	
    
    if(gd_key_state_get(KEY_REV1) == SET)
        SystemInfo.Zigbee_OnOff = true;
    else
        SystemInfo.Zigbee_OnOff = false; 
    
    if(gd_key_state_get(KEY_REV2) == SET)
        SystemInfo.Zigbee_Codi = true;
    else
        SystemInfo.Zigbee_Codi = false; 
    

	InitSys();
    
	InitAbovComms();
	InitThermostatComms();
    InitZigbeeComms();
    RmtRXInit();

    
    while(1) {
        gpio_check_task();
//        if(Motor_CommTime >= 1000) {
//            Motor_CommTime = 0;
//            __set_FAULTMASK(1);
//            NVIC_SystemReset();
////            InitSerialDriver();
////            InitSerilInterrupt();
//        }
        Thermostat_Rx();
        if(Uart_TxTime >= 200) {               
            Abov_Tx();
            Abov_Rx();
            
            if(SystemInfo.Zigbee_OnOff == true) {
                if(SystemInfo.Zigbee_Codi == true) {
                    Zigbee_CodiTx();
                    Zigbee_CodiRx();
                }
                else {
                    Zigbee_RouterTx();
                    Zigbee_RouterRx();
                }
            }

            Uart_TxTime = 0;
        }  
        
        LED_Ctrl();
        RmtRX();
        Pm2008();
    }
}


void software_reset(void)
{
    /* set FAULTMASK */
    __set_FAULTMASK(1);
    NVIC_SystemReset();
}

void WdtPort_Init(void) {
    rcu_periph_clock_enable(RCU_GPIOC);

    gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_13);
    gpio_bit_set(GPIOC, GPIO_PIN_13);
}

void Wdt_ClkOut(void) {
    if(Motor_CommTime >= 1000) return;
    
    if(Wdt_Outsts == 0) {
        gpio_bit_reset(GPIOC, GPIO_PIN_13);
        Wdt_Outsts = 1;
    }
    else {
        gpio_bit_set(GPIOC, GPIO_PIN_13);
        Wdt_Outsts = 0;
    }
}


void SensorDustCallback(DustEventT *pEvt) 
{
//	if (SensorInfo.pm1_0!=(int)pEvt->pm_1_0) 
//	{
//		if (pEvt->pm_1_0 > MIN_PM_VAL && pEvt->pm_1_0 < MAX_PM_VAL)
//		{
//			SensorInfo.pm1_0 = (int)pEvt->pm_1_0;
//		}
//	}
//	
//	if (SensorInfo.pm10_0!=(int)pEvt->pm_10_0) 
//	{
//		if (pEvt->pm_10_0 > MIN_PM_VAL && pEvt->pm_10_0 < MAX_PM_VAL)
//		{
//			SensorInfo.pm10_0 = (int)pEvt->pm_10_0;
//		}
//	}
//	
//	if (SensorInfo.pm2_5!=(int)pEvt->pm_2_5) 
//	{
//		if (pEvt->pm_2_5 > MIN_PM_VAL && pEvt->pm_2_5 < MAX_PM_VAL)
//		{
//			SensorInfo.pm2_5 = (int)pEvt->pm_2_5;
//		}
//	}
}

void SensorCo2Callback(int Co2_value) 
{
//	if (Co2_value < MIN_CO2_VAL) Co2_value = MIN_CO2_VAL;
//	else if (Co2_value > MAX_CO2_VAL) Co2_value = MAX_CO2_VAL;
//
//	if (SensorInfo.co2!=(int)Co2_value) 
//	{
//		SensorInfo.co2 = (int)Co2_value;
//	}
}

void SensorPressureCallback(PressureEventT *pEvt) 
{
//	int temp_int;
//	float temp_real;
//
//	/* adjustment */
//	temp_real = pEvt->temperature*0.8;
//
//	 /* use %0.1f */
//	temp_int = (int)(temp_real*10);
//	temp_real = (float)temp_int/10;
//
//	if (SensorInfo.temp != temp_real) 
//	{
//		SensorInfo.temp = temp_real;
//	}
//
//	if (SensorInfo.smell_iaq != (int)pEvt->iaq) 
//	{
//		SensorInfo.smell_iaq = (int)pEvt->iaq;
//	}
//
//	if (SensorInfo.humidity != (int)pEvt->humidity) 
//	{
//		SensorInfo.humidity = (int)pEvt->humidity;
//	}
//
//	if (SensorInfo.pressure != pEvt->pressure) 
//	{
//		SensorInfo.pressure = pEvt->pressure;
//	}
//	
//	if (SensorInfo.gas != pEvt->gas) 
//	{
//		SensorInfo.gas = pEvt->gas;
//	}

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
//			CtrlMode(CTRL_MODE_SLEEP);
		}
		else
		{
//			CtrlMode(mode);
//			CtrlFan(level);
		}

		OldDoorStatus = COVER_CLOSED;
	}
	else if((event == COVER_OPEN) && (OldDoorStatus != COVER_OPEN))
	{
		AboveRxInfo.Err |= OPENED_COVER_ERR;

		mode = AboveTxInfo.Mode;
		level = AboveTxInfo.FanLevel;
//		CtrlMode(0);

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





void InitNVS (void) 
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
	InitCheckGpio();

    InitDust();
	InitSerialDriver();
    InitSerilInterrupt();
    InitLed();

    if(gd_key_state_get(KEY_REV0) == SET)
        SystemInfo.HasSensors = true;
    else
        SystemInfo.HasSensors = false; 
}

void LoadNvsData (void) 
{
//	int Ret = 0;
//	char StrBuf[128] = {0,};
//	unsigned char	Value8 = 0;
//	unsigned int 	Value32 = 0;
//	int datalen = 0;
//
//	Ret = GetValueU8(PROP_NAME_SYS_ROOMID, &Value8);
//	if(Ret == 0) 
//	{
//		if(Value8 != PersistDataInfo.DeviceId) 
//		{
//			PersistDataInfo.DeviceId = Value8;
//		}
//	}
//
//	Ret = GetValueStr(PROP_NAME_SYS_SERIAL, StrBuf);
//	if(Ret == 0) 
//	{
//		datalen = strlen(StrBuf);
//		if(datalen > 0) 
//		{
//			if( strcmp(StrBuf,PersistDataInfo.Serial) != 0) 
//			{
//			
//				memset(PersistDataInfo.Serial, 0,sizeof(PersistDataInfo.Serial));
//				strncpy(PersistDataInfo.Serial,StrBuf,datalen);
//			}
//		}
//	}
//
//	memset(StrBuf, 0, sizeof(StrBuf));
//	Ret = GetValueStr(PROP_NAME_SYS_SSID, StrBuf);
//	if(Ret == 0) 
//	{
//		datalen = strlen(StrBuf);
//		if(datalen > 0) 
//		{
//			if(strcmp(StrBuf,PersistDataInfo.SSID) != 0) 
//			{
//				memset(PersistDataInfo.SSID, 0,sizeof(PersistDataInfo.SSID));
//				strncpy(PersistDataInfo.SSID,StrBuf,datalen);
//			}
//		}
//	}
//
//	memset(StrBuf, 0, sizeof(StrBuf));
//	Ret = GetValueStr(PROP_NAME_SYS_PWD, StrBuf);
//	if(Ret == 0) 
//	{
//		datalen = strlen(StrBuf);
//		if(datalen > 0) 
//		{
//			if( strcmp(StrBuf, PersistDataInfo.PWD) != 0) 
//			{
//				memset(PersistDataInfo.PWD, 0,sizeof(PersistDataInfo.PWD));
//				strncpy(PersistDataInfo.PWD,StrBuf,datalen);
//			}
//		}
//	}
//
//	memset(StrBuf, 0, sizeof(StrBuf));
//	Ret = GetValueStr(PROP_NAME_SYS_MODULEID, StrBuf);
//	if(Ret == 0) 
//	{
//		datalen = strlen(StrBuf);
//		if(datalen > 0) 
//		{
//			if( strcmp(StrBuf, PersistDataInfo.ModuleID) != 0) 
//			{
//				memset(PersistDataInfo.ModuleID, 0,sizeof(PersistDataInfo.ModuleID));			
//				strncpy(PersistDataInfo.ModuleID,StrBuf,datalen);
//			}
//		}
//	}
//
//	Value32 = 0;
//	Ret = GetValueU32(PROP_NAME_SYS_NETID, &Value32);
//	if(Ret == 0)
//	{
//		if(Value32 != PersistDataInfo.NetId)
//		{
//			PersistDataInfo.NetId = Value32;
//		}
//	}
//
//	Value32 = 0;
//	Ret = GetValueU32(PROP_NAME_SYS_FAU_FLTPRSLMT, &Value32);
//	if(Ret == 0) 
//	{
//		if(Value32 != PersistDataInfo.fltdifprslmt)
//		{
//			PersistDataInfo.fltdifprslmt = Value32;
//		}
//	}
//
//	Value8 = 0;
//	Ret = GetValueU8(PROP_NAME_SYS_FACTORY_DONE, &Value8);
//	if(Ret == 0) 
//	{
//		if(Value8 != PersistDataInfo.FactoryDone) 
//		{
//			PersistDataInfo.FactoryDone = Value8;
//		}
//	}
//
//	Value8 = 0;
//	Ret = GetValueU8(PROP_NAME_SYS_FAU_MOD_NORMAL, &Value8);
//	if(Ret == 0) 
//	{
//		if(Value8 != PersistDataInfo.FauNormalMode) 
//		{
//			PersistDataInfo.FauNormalMode = Value8;
//		}
//	}
//
//	Value8 = 0;
//	Ret = GetValueU8(PROP_NAME_APP_UPGRADE, &Value8);
//	if(Ret == 0) 
//	{
//		if(Value8 != PersistDataInfo.UpgradeOn) 
//		{
//			PersistDataInfo.UpgradeOn = Value8;
//		}
//	}
//
//	memset(StrBuf, 0, sizeof(StrBuf));
//	Ret = GetValueStr(PROP_NAME_UG_HOST, StrBuf);
//	if(Ret == 0) 
//	{
//		datalen = strlen(StrBuf);
//		if(datalen > 0) 
//		{
//			if( strcmp(StrBuf,PersistDataInfo.UgHost) != 0) 
//			{
//				memset(PersistDataInfo.UgHost, 0,sizeof(PersistDataInfo.UgHost));
//				strncpy(PersistDataInfo.UgHost,StrBuf,datalen);
//			}
//		}
//	}
//
//	Value32 = 0;
//	Ret = GetValueU32(PROP_NAME_UG_PORT, &Value32);
//	if(Ret == 0) 
//	{
//		if(Value32 != PersistDataInfo.UgPort) 
//		{
//			PersistDataInfo.UgPort = Value32;
//		}
//	}
//
//	memset(StrBuf, 0, sizeof(StrBuf));
//	Ret = GetValueStr(PROP_NAME_UG_DIR, StrBuf);
//	if(Ret == 0) 
//	{
//		datalen = strlen(StrBuf);
//		if(datalen > 0) 
//		{
//			if( strcmp(StrBuf,PersistDataInfo.UgDir) != 0) 
//			{
//				memset(PersistDataInfo.UgDir, 0,sizeof(PersistDataInfo.UgDir));
//				strncpy(PersistDataInfo.UgDir,StrBuf,datalen);
//			}
//		}
//	}
//
//	memset(StrBuf, 0, sizeof(StrBuf));
//	Ret = GetValueStr(PROP_NAME_UG_ID, StrBuf);
//	if(Ret == 0) 
//	{
//		datalen = strlen(StrBuf);
//		if(datalen > 0) 
//		{
//			if( strcmp(StrBuf,PersistDataInfo.UgId) != 0) 
//			{
//				memset(PersistDataInfo.UgId, 0,sizeof(PersistDataInfo.UgId));
//				strncpy(PersistDataInfo.UgId,StrBuf,datalen);
//			}
//		}
//	}
//
//	memset(StrBuf, 0, sizeof(StrBuf));
//	Ret = GetValueStr(PROP_NAME_UG_PWD, StrBuf);
//	if(Ret == 0) 
//	{
//		datalen = strlen(StrBuf);
//		if(datalen > 0) 
//		{
//			if( strncmp(StrBuf, PersistDataInfo.UgPwd, datalen) != 0) 
//			{
//				memset(PersistDataInfo.UgPwd, 0,sizeof(PersistDataInfo.UgPwd));
//				strncpy(PersistDataInfo.UgPwd,StrBuf,datalen);
//			}
//		}
//	}
//
//	Value8 = 0;
//	Ret = GetValueU8(PROP_NAME_SYS_BCAST_ENCRT, &Value8);
//	if(Ret == 0) 
//	{
//		if(Value8 != PersistDataInfo.BcastEncryption) 
//		{
//			PersistDataInfo.BcastEncryption = Value8;
//		}
//	}
//
//	Ret = GetValueU32(PROP_NAME_RPM_1, &Value32);
//	if(Ret == 0) 
//	{
//		if(Value32 != PersistDataInfo.rpm1) 
//		{
//			PersistDataInfo.rpm1 = Value32;
//		}
//	}
//
//	Ret = GetValueU32(PROP_NAME_RPM_2, &Value32);
//	if(Ret == 0) 
//	{
//		if(Value32 != PersistDataInfo.rpm2) 
//		{
//			PersistDataInfo.rpm2 = Value32;
//		}
//	}
//
//	Ret = GetValueU32(PROP_NAME_RPM_3, &Value32);
//	if(Ret == 0) 
//	{
//		if(Value32 != PersistDataInfo.rpm3) 
//		{
//			PersistDataInfo.rpm3 = Value32;
//		}
//	}
//
//	Ret = GetValueU32(PROP_NAME_RPM_4, &Value32);
//	if(Ret == 0) 
//	{
//		if(Value32 != PersistDataInfo.rpm4) 
//		{
//			PersistDataInfo.rpm4 = Value32;
//		}
//	}
//
//	Ret = GetValueU32(PROP_NAME_RPM_5, &Value32);
//	if(Ret == 0) 
//	{
//		if(Value32 != PersistDataInfo.rpm5) 
//		{
//			PersistDataInfo.rpm5 = Value32;
//		}
//	}
//
//	SystemInfo.SWVer = (SW_VER_MAJOR<<16) | (SW_VER_MINOR<<8) | SW_VER_PATCH;
//	SystemInfo.MqttUgMode = 0;
//	SystemInfo.MqttUgSendPktCnt = 0;
//	SystemInfo.IsCertFileValid = false;
//
//	SensorInfo.temp = TEMP_INITIAL_VALUE;
//	SensorInfo.co2 = CO2_INITIAL_VALUE;
//	SensorInfo.co2_level = 1;
//	SensorInfo.pm1_0 = PM_INITIAL_VALUE;
//	SensorInfo.pm2_5 = PM_INITIAL_VALUE;
//	SensorInfo.pm10_0 = PM_INITIAL_VALUE;
//	SensorInfo.pm_level = 1;
//	SensorInfo.selected_pm = PM_2_5;
//	SensorInfo.pm_sync = 1;
//	SensorInfo.humidity = HUMIDITY_INITIAL_VALUE;
//	SensorInfo.smell_iaq = IAQ_INITIAL_VALUE;
//	
//	CommInfo.Sync = 0;
//	CommInfo.SyncWired = 0;
//	CommInfo.VSPSet = 0;
//	CommInfo.RPMSet = 0;
//
////	AboveTxInfo.Power = 0;
//	AboveTxInfo.FanLevel = 0;
//	AboveTxInfo.Mode = OP_MODE_OFF;
//	
//	AboveTxInfo.FltTmrRst = 0;
//	AboveTxInfo.FltTmr = 0;
//	AboveTxInfo.FltTmrLmt = PersistDataInfo.fltdifprslmt;
//
//
//	AboveTxInfo.RPM[0] = PersistDataInfo.rpm1;
//	AboveTxInfo.RPM[1] = PersistDataInfo.rpm2;
//	AboveTxInfo.RPM[2] = PersistDataInfo.rpm3;
//	AboveTxInfo.RPM[3] = PersistDataInfo.rpm4;
//	AboveTxInfo.RPM[4] = PersistDataInfo.rpm5;
//	AboveTxInfo.RPMSet = 0;
//
//#if 0
//	AboveTxInfo.VSP[0] = PID_VSP1;
//	AboveTxInfo.VSP[1] = PID_VSP2;
//	AboveTxInfo.VSP[2] = PID_VSP3;
//	AboveTxInfo.VSP[3] = PID_VSP4;
//	AboveTxInfo.VSP[4] = PID_VSP5;
//#else
//	AboveTxInfo.VSP[0] = PPS2VSP(PersistDataInfo.rpm1);
//	AboveTxInfo.VSP[1] = PPS2VSP(PersistDataInfo.rpm2);
//	AboveTxInfo.VSP[2] = PPS2VSP(PersistDataInfo.rpm3);
//	AboveTxInfo.VSP[3] = PPS2VSP(PersistDataInfo.rpm4);
//	AboveTxInfo.VSP[4] = PPS2VSP(PersistDataInfo.rpm5);
//#endif
//	
//	AboveTxInfo.VSPOffset = FAU_DEFAULT_VSP_OFFSET;
//	AboveTxInfo.Led = 0;
//
//	memset(AboveRxInfo.Serial, 0, MAX_SERIAL_STR_LEN);
//	AboveRxInfo.VerH = 'A';
//	AboveRxInfo.VerL = 0;
//	
////	AboveRxInfo.Power = 0;
//	AboveRxInfo.FanLevel = 0;
//	AboveRxInfo.Mode = OP_MODE_OFF;
//	
//	AboveRxInfo.FltTmrRst = 0;
//	AboveRxInfo.FltTmr = 0;
//	AboveRxInfo.FltTmrLmt = PersistDataInfo.fltdifprslmt;
//
//	AboveRxInfo.VSP[0] = PID_VSP1;
//	AboveRxInfo.VSP[1] = PID_VSP2;
//	AboveRxInfo.VSP[2] = PID_VSP3;
//	AboveRxInfo.VSP[3] = PID_VSP4;
//	AboveRxInfo.VSP[4] = PID_VSP5;
//	AboveRxInfo.VSPOffset = FAU_DEFAULT_VSP_OFFSET;
//	AboveRxInfo.Err = 0;
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

}

void DefaultSetting(int needToLoadNvs)
{
	InitData();

//	Ret = SetValueU8(PROP_NAME_SYS_ROOMID, PersistDataInfo.DeviceId);
//	Ret = SetValueStr(PROP_NAME_SYS_SSID, DEFAULT_SETUP_SSID);
//	Ret = SetValueStr(PROP_NAME_SYS_PWD, DEFAULT_SETUP_PWD);
//	Ret = SetValueU32(PROP_NAME_SYS_NETID, DEFAULT_NETWORK_ID);
//	Ret = SetValueU32(PROP_NAME_RPM_1, PersistDataInfo.rpm1);
//	Ret = SetValueU32(PROP_NAME_RPM_2, PersistDataInfo.rpm2);
//	Ret = SetValueU32(PROP_NAME_RPM_3, PersistDataInfo.rpm3);
//	Ret = SetValueU32(PROP_NAME_RPM_4, PersistDataInfo.rpm4);
//	Ret = SetValueU32(PROP_NAME_RPM_5, PersistDataInfo.rpm5);
//    
//	if(needToLoadNvs)
		LoadNvsData();
}

void SetId(unsigned char Id)
{
//	int16_t Ret = 0;
//	unsigned char	Value8 = 0;
//
////	Ret = SetValueU8(PROP_NAME_SYS_ROOMID, Id);
//
//	Ret = GetValueU8(PROP_NAME_SYS_ROOMID, &Value8);
//	if(Ret == 0) 
//	{
//		if(PersistDataInfo.DeviceId != Value8) 
//		{
//			PersistDataInfo.DeviceId = Value8;
//		}
//	}
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
//	int Ret = 0;
//	unsigned int	Value32 = 0;
//	
//	if(index > 5)
//	{
//		return;
//	}
//
//	switch (index)
//	{
//		case 1:
//			Ret = SetValueU32(PROP_NAME_RPM_1, rpm);
//			Ret = GetValueU32(PROP_NAME_RPM_1, &Value32);
//			if(Ret == 0) 
//			{
//				if(PersistDataInfo.rpm1 != Value32) 
//				{
//					PersistDataInfo.rpm1 = Value32;
//				}
//			}
//
//			break;
//			
//		case 2:
//			Ret = SetValueU32(PROP_NAME_RPM_2, rpm);
//		
//			Ret = GetValueU32(PROP_NAME_RPM_2, &Value32);
//			if(Ret == 0) 
//			{
//				if(PersistDataInfo.rpm2 != Value32) 
//				{
//					PersistDataInfo.rpm2 = Value32;
//				}
//			}
//			break;
//
//		case 3:
//			Ret = SetValueU32(PROP_NAME_RPM_3, rpm);
//		
//			Ret = GetValueU32(PROP_NAME_RPM_3, &Value32);
//			if(Ret == 0) 
//			{
//				if(PersistDataInfo.rpm3 != Value32) 
//				{
//					PersistDataInfo.rpm3 = Value32;
//				}
//			}
//
//			break;
//
//		case 4:
//			Ret = SetValueU32(PROP_NAME_RPM_4, rpm);
//		
//			Ret = GetValueU32(PROP_NAME_RPM_4, &Value32);
//			if(Ret == 0) 
//			{
//				if(PersistDataInfo.rpm4 != Value32) 
//				{
//					PersistDataInfo.rpm4 = Value32;
//				}
//			}
//			break;
//
//		case 5:
//			Ret = SetValueU32(PROP_NAME_RPM_5, rpm);
//		
//			Ret = GetValueU32(PROP_NAME_RPM_5, &Value32);
//			if(Ret == 0) 
//			{
//				if(PersistDataInfo.rpm5 != Value32) 
//				{
//					PersistDataInfo.rpm5 = Value32;
//				}
//			}
//			break;
//			
//		default:
//			break;
//
//	}
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




