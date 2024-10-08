
#ifndef	__APP_H__
#define __APP_H__

#include "gd32f30x.h"
typedef enum
{
  false = 0,
  true
} bool;

#include "pressure.h"
#include "dust.h"
#include "co2.h"
#include "remote.h"
#include "led.h"
#include "console.h"
#include "gpio_dev.h"
#include "wifi.h"
#include "sntp.h"
#include "mqtt.h"
#include "control.h"
#include "audio.h"
#include "serial.h"
#include "nvs_api.h"
#include "timer.h"
#include "abov_comms.h"
#include "thermostat_comms.h"
#include "pid.h"
#include "factory.h"

#define MAX_FAU_LVL				5
#define MAX_ERV_LVL				4

#define MAX_CO2_VAL				9999
#define MIN_CO2_VAL				400

#define MAX_PM_VAL				999
#define MIN_PM_VAL				1

#define PM_INITIAL_VALUE		(-1)
#define CO2_INITIAL_VALUE		(-1)
#define HUMIDITY_INITIAL_VALUE	(-1)
#define TEMP_INITIAL_VALUE		(5)
#define IAQ_INITIAL_VALUE		(0)
#define MAX_FILTER_USE_TIME 3000

#define FAU_CONN_RESET_WDT    	20
#define FAU_DEFAULT_VSP_OFFSET  0x7F

#define MAX_SERIAL_STR_LEN		64
#define MAX_SSID_STR_LEN		64
#define MAX_PWD_STR_LEN			64
#define MAX_VERSION_STR_LEN		64

#define MAX_MODULE_STR_LEN		19

#define PID_VSP1 				60 // 3.0*20
#define PID_VSP2				64 // 3.2*20
#define PID_VSP3				68 // 3.4*20
#define PID_VSP4				74 // 3.7*20
#define PID_VSP5				80 // 4*20	

#define PID_TARGET_PPS1			138
#define PID_TARGET_PPS2			159
#define PID_TARGET_PPS3			178
#define PID_TARGET_PPS4			209
#define PID_TARGET_PPS5			236

#define ESP32_MOUDLE_ID 		"101"

#define DEFAULT_SETUP_SSID			"zipsair-setup"
#define DEFAULT_SETUP_PWD			"zipsair1661"
#define DEFAULT_NETWORK_ID				101

#define PROP_NAME_SYS_CERTIMODE			"certimode"
#define PROP_NAME_SYS_ROOMID			"id"
#define PROP_NAME_SYS_SSID				"ssid"
#define PROP_NAME_SYS_PWD				"ssid_pwd"
#define PROP_NAME_SYS_SERIAL    		"serial"
#define PROP_NAME_SYS_NETID				"net_id"
#define PROP_NAME_SYS_MODULEID			"moduleid"

#define PROP_NAME_SYS_FAU_FLTPRSLMT		"fltdifprslmt"
#define PROP_NAME_SYS_FACTORY_DONE		"factory_done"
#define PROP_NAME_SYS_FAU_MOD_NORMAL	"fau_normal"
#define PROP_NAME_SYS_MQTT				"mqtt"
#define PROP_NAME_SYS_BCAST_ENCRT		"bcast_enc"
#define PROP_NAME_APP_UPGRADE			"app_upgrade"
#define PROP_NAME_UG_HOST				"ug_host"
#define PROP_NAME_UG_PORT				"ug_port"
#define PROP_NAME_UG_DIR				"ug_dir"
#define PROP_NAME_UG_ID					"ug_id"
#define PROP_NAME_UG_PWD				"ug_pwd"
#define PROP_NAME_FAU_EN				"fau_on"
#define PROP_NAME_SENSOR_PM_ADJ    		"pmadjmode"

#define PROP_NAME_RPM_1					"rpm1"
#define PROP_NAME_RPM_2					"rpm2"
#define PROP_NAME_RPM_3					"rpm3"
#define PROP_NAME_RPM_4					"rpm4"
#define PROP_NAME_RPM_5					"rpm5"

#define DEV_DISCONNECT_CNT 20

enum {
	/* do not change the starting value */
	PM_1_0 = 0,
	PM_2_5 = 1,
	DEFAULT_PM = PM_2_5,
	PM_10_0 = 2,
	PM_MAX
};

enum {
	DEV_CONNECTION_NONE = 0x0,
	DEV_CONNECTION_WIFI = 0x1,
	DEV_CONNECTION_RS485 = 0x2,
	DEV_CONNECTION_MAX = 0x3
};


typedef enum OpMode 
{
	/* do not change the starting value */
	OP_MODE_OFF = 0,
	OP_MODE_AUTO,
	OP_MODE_NORMAL,
	OP_MODE_MAX
}OpModeT;

typedef struct RtcEvent 
{
	int year; /* sould be added 1900 */
	int mon; /* sould be added 1, Jan is 1 */
	int mday; /* 1 to 31 */
	int wday; /* Sunday is starting from 0 */
	int hour;
	int min;
	int sec;
}RtcEventT;

typedef struct CommInfo
{
	unsigned int Sync;
	unsigned char SyncWired;
	unsigned int VSPSet;
	unsigned int RPMSet;
}CommInfoT;
	
typedef struct AboveTxInfo
{
//	unsigned char	Power;
	unsigned char	FanLevel;
	unsigned int	Mode;

	unsigned char	FltTmrRst;
	unsigned short	FltTmr;
	unsigned short	FltTmrLmt;

	unsigned char	VSP[MAX_FAU_LVL];
	int				VSPOffset;
	int				Led;

	int				RPMSet;
	int				RPM[MAX_FAU_LVL];
	
}AboveTxInfoT;

typedef struct AboveRxInfo
{
	char			Serial[MAX_SERIAL_STR_LEN];
	uint8_t         VerH;
	int				VerL;

//	unsigned char	Power;
	unsigned char	FanLevel;
	OpModeT			Mode;

	unsigned char	FltTmrRst;
	unsigned short	FltTmr;
	unsigned short	FltTmrLmt;

	unsigned short	PPS;
	unsigned char	VSP[MAX_FAU_LVL];
	int				VSPOffset;
	int				Err;
}AboveRxInfoT;

typedef struct SensorInfo 
{
	float temp;
	int co2;
	int co2_level;
	int pm1_0;
	int pm2_5;
	int pm10_0;
	int pm_level;
	int selected_pm;
	unsigned int pm_sync;
	int humidity;
	int smell_iaq;
	int smell_level;
	float pressure;
	float gas;
}SensorInfoT;

typedef struct PersistDataInfo 
{
	unsigned char DeviceId;
	char SSID[MAX_SSID_STR_LEN];
	char PWD[MAX_PWD_STR_LEN];
	char Serial[MAX_SERIAL_STR_LEN];
	char ModuleID[MAX_MODULE_STR_LEN];
	char UpgradeOn;
	unsigned int NetId;

	unsigned char Volume;
	unsigned int fltdifprslmt;
	unsigned char PmAdjustment;
	unsigned char FactoryDone;
	unsigned char FauNormalMode;
	unsigned char Mqtt;
	
	char UgHost[256];
	unsigned int UgPort;
	char UgDir[256];
	char UgId[256];
	char UgPwd[256];
	unsigned char BcastEncryption;

	unsigned int rpm1;
	unsigned int rpm2;
	unsigned int rpm3;
	unsigned int rpm4;
	unsigned int rpm5;
}PersistDataInfoT;

typedef struct SystemInfo
{
	unsigned int SWVer;
	char Version[MAX_VERSION_STR_LEN];
	RtcEventT RtcTime;
	int RtcSet;
	int UpgradeMode;
	int ConnType;
	bool HasSensors;

	int MqttUgMode;
	int MqttUgSendPktCnt;

	int ResvStatus;
	int ResvTime;
	int ResvTimer;
	int ResvTimeSet;
	int ResvTimerInstance;

	bool IsCertFileValid;
}SystemInfoT;

typedef struct WifiInfo 
{
	char Status;
	unsigned char SSID[MAX_SSID_STR_LEN];
	unsigned char PWD[MAX_PWD_STR_LEN];
	unsigned int FullIp;
	unsigned int Ip;
	int Rssi;
}WifiInfoT;

void software_reset(void);
void GetSystemInfo( SystemInfoT **ppSystemInfo );
void GetPersistDataInfo( PersistDataInfoT **ppPersistDataInfo );
void GetWifiInfo( WifiInfoT * * ppWifiInfo);
void GetSensorInfo( SensorInfoT **ppSensorInfo );

void GetAbovTxInfo( AboveTxInfoT **ppData );

void GetAbovRxInfo( AboveRxInfoT **ppData );
void GetCommInfo( CommInfoT **ppData ) ;

void SensorDustCallback(DustEventT *pEvt);
void SensorPressureCallback(PressureEventT *pEvt);
void SensorCo2Callback(int Co2_value);
void Register_GetSensorInfo(void);

void DefaultSetting(int needToLoadNvs);

void SetId(unsigned char Id);
void SetSSID(char *pSSID);
void SetPWD(char *pPWD);
void SetRPM(unsigned int index, unsigned int rpm);
//void SetRPM(unsigned int index, unsigned int rpm, unsigned char *pVSP);
unsigned char PPS2VSP (unsigned int pps);

void Register_Update(void);
void HTTPSUpdateSW(void);

void Register_CommandApp(void);
void Register_CommandFactory(void);

#endif /* __APP_H__*/

