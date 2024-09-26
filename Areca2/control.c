
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//#include <nvs_flash.h>
//#include <driver/uart.h>
//#include <soc/uart_struct.h>
#include "argtable3.h"


#include "app.h"

#define	GOOD				1
#define	NORMAL				2
#define	BAD					3
#define	VERY_BAD			4

#define	FAN_LEVEL_OFF		0
#define	FAN_LEVEL_SLEEP		1
#define	FAN_LEVEL_LOW		2
#define	FAN_LEVEL_MIDDLE	3
#define	FAN_LEVEL_HIGH		4
#define	FAN_LEVEL_TURBO		5
#define	FAN_LEVEL_MAX		6

/*
#define KP	0.18
#define KI	0.5
#define KD	0.05
*/

#define KP	0.18
#define KI	0.2
#define KD	0.1

static struct {
    struct arg_int *pRevHour;
    struct arg_end *pEnd;
}CmdArgs;

static struct {
    struct arg_int *command;
    struct arg_int *step;
    struct arg_end *end;
}FauCmd;

int RevTimerInstance = 0;
//static QueueHandle_t rx_queue;

PIDCtrlT PIDCtrl;
PersistDataInfoT *pPersistDataInfo = NULL;
SensorInfoT *pSensorInfo = NULL;
AboveTxInfoT *pTxData = NULL;
AboveRxInfoT *pRxData = NULL;
SystemInfoT *pSystemInfo = NULL;

PIDCtrlT *pPID = NULL;
float Input = 0, SetPoint = 0, Output = 0;

	unsigned char FanLevel = 0, OldFanLevel = -1;
	int converted_VSPOffset = 0, Rpm = 0, rc = 0, OldRpm = 0;
    

void GetFauFanLevel (int fauNormalMode, int SelectedPm, int PmValue, int *pFauFanLevel) 
{

	if((pFauFanLevel == NULL) || (SelectedPm > PM_10_0)) 
	{
		return;
	}

	switch(SelectedPm) 
	{
	case PM_1_0:
	case PM_2_5:
		if (fauNormalMode) 
		{
			/* RoomInfo[MYROOM].sens.pm_level variable will determine a real level of fau*/
			switch(*pFauFanLevel) 
			{
			case 0: 
				if (PmValue>=0 && PmValue<=15) *pFauFanLevel = 0;
				else if (PmValue>15 && PmValue<=35) *pFauFanLevel = 1;
				else if (PmValue>35 && PmValue<=75) *pFauFanLevel = 2;
				else if (PmValue>75 && PmValue<=150) *pFauFanLevel = 3;
				else if (PmValue>150) *pFauFanLevel = 4;
				break;
			case 1:
				if (PmValue>=0 && PmValue<=12) *pFauFanLevel = 0;
				else if (PmValue>12 && PmValue<=35) *pFauFanLevel = 1;
				else if (PmValue>35 && PmValue<=75) *pFauFanLevel = 2;
				else if (PmValue>75 && PmValue<=150) *pFauFanLevel = 3;
				else if (PmValue>150) *pFauFanLevel = 4;
				break;
			case 2:
				if (PmValue>=0 && PmValue<=15) *pFauFanLevel = 0;
				else if (PmValue>15 && PmValue<=31) *pFauFanLevel = 1;
				else if (PmValue>31 && PmValue<=75) *pFauFanLevel = 2;
				else if (PmValue>75 && PmValue<=150) *pFauFanLevel = 3;
				else if (PmValue>150) *pFauFanLevel = 4;
				break;
			case 3:
				if (PmValue>=0 && PmValue<=15) *pFauFanLevel = 0;
				else if (PmValue>15 && PmValue<=35) *pFauFanLevel = 1;
				else if (PmValue>35 && PmValue<=71) *pFauFanLevel = 2;
				else if (PmValue>71 && PmValue<=150) *pFauFanLevel = 3;
				else if (PmValue>150) *pFauFanLevel = 4;
				break;
			case 4:
				if (PmValue>=0 && PmValue<=15) *pFauFanLevel = 0;
				else if (PmValue>15 && PmValue<=35) *pFauFanLevel = 1;
				else if (PmValue>35 && PmValue<=75) *pFauFanLevel = 2;
				else if (PmValue>75 && PmValue<=146) *pFauFanLevel = 3;
				else if (PmValue>146) *pFauFanLevel = 4;
				break;
			default:
				if (PmValue>=0 && PmValue<=15) *pFauFanLevel = 0;
				else if (PmValue>15 && PmValue<=35) *pFauFanLevel = 1;
				else if (PmValue>35 && PmValue<=75) *pFauFanLevel = 2;
				else if (PmValue>75 && PmValue<=150) *pFauFanLevel = 3;
				else if (PmValue>150) *pFauFanLevel = 4;
				break;
			}
		}
		else 
		{

			switch(*pFauFanLevel) 
			{
			case 0: 
				if (PmValue>=0 && PmValue<=9) *pFauFanLevel = 0;
				else if (PmValue>10 && PmValue<=17) *pFauFanLevel = 1;
				else if (PmValue>18 && PmValue<=37) *pFauFanLevel = 2;
				else if (PmValue>38 && PmValue<=77) *pFauFanLevel = 3;
				else if (PmValue>77) *pFauFanLevel = 4;
				break;
			case 1:
				if (PmValue>=0 && PmValue<=6) *pFauFanLevel = 0;
				else if (PmValue>6 && PmValue<=17) *pFauFanLevel = 1;
				else if (PmValue>18 && PmValue<=37) *pFauFanLevel = 2;
				else if (PmValue>38 && PmValue<=77) *pFauFanLevel = 3;
				else if (PmValue>77) *pFauFanLevel = 4;
				break;
			case 2:
				if (PmValue>=0 && PmValue<=9) *pFauFanLevel = 0;
				else if (PmValue>10 && PmValue<=14) *pFauFanLevel = 1;
				else if (PmValue>14 && PmValue<=37) *pFauFanLevel = 2;
				else if (PmValue>38 && PmValue<=77) *pFauFanLevel = 3;
				else if (PmValue>77) *pFauFanLevel = 4;
				break;
			case 3:
				if (PmValue>=0 && PmValue<=9) *pFauFanLevel = 0;
				else if (PmValue>10 && PmValue<=17) *pFauFanLevel = 1;
				else if (PmValue>18 && PmValue<=34) *pFauFanLevel = 2;
				else if (PmValue>34 && PmValue<=77) *pFauFanLevel = 3;
				else if (PmValue>77) *pFauFanLevel = 4;
				break;
			case 4:
				if (PmValue>=0 && PmValue<=9) *pFauFanLevel = 0;
				else if (PmValue>10 && PmValue<=17) *pFauFanLevel = 1;
				else if (PmValue>18 && PmValue<=37) *pFauFanLevel = 2;
				else if (PmValue>38 && PmValue<=74) *pFauFanLevel = 3;
				else if (PmValue>74) *pFauFanLevel = 4;
				break;
			default:
				if (PmValue>=0 && PmValue<=9) *pFauFanLevel = 0;
				else if (PmValue>10 && PmValue<=17) *pFauFanLevel = 1;
				else if (PmValue>18 && PmValue<=37) *pFauFanLevel = 2;
				else if (PmValue>38 && PmValue<=77) *pFauFanLevel = 3;
				else if (PmValue>77) *pFauFanLevel = 4;
				break;
			}
		}
		break;
	case PM_10_0:
		/* RoomInfo[MYROOM].sens.pm_level variable will determine a real level of fau*/
		switch(*pFauFanLevel) {
		case 0: 
			if (PmValue>=0 && PmValue<=9) *pFauFanLevel = 0;
			else if (PmValue>10 && PmValue<=32) *pFauFanLevel = 1;
			else if (PmValue>32 && PmValue<=82) *pFauFanLevel = 2;
			else if (PmValue>82 && PmValue<=152) *pFauFanLevel = 3;
			else if (PmValue>152) *pFauFanLevel = 4;
			break;
		case 1:
			if (PmValue>=0 && PmValue<=6) *pFauFanLevel = 0;
			else if (PmValue>6 && PmValue<=32) *pFauFanLevel = 1;
			else if (PmValue>32 && PmValue<=82) *pFauFanLevel = 2;
			else if (PmValue>82 && PmValue<=152) *pFauFanLevel = 3;
			else if (PmValue>152) *pFauFanLevel = 4;
			break;
		case 2:
			if (PmValue>=0 && PmValue<=9) *pFauFanLevel = 0;
			else if (PmValue>10 && PmValue<=28) *pFauFanLevel = 1;
			else if (PmValue>28 && PmValue<=82) *pFauFanLevel = 2;
			else if (PmValue>82 && PmValue<=152) *pFauFanLevel = 3;
			else if (PmValue>152) *pFauFanLevel = 4;
			break;
		case 3:
			if (PmValue>=0 && PmValue<=9) *pFauFanLevel = 0;
			else if (PmValue>10 && PmValue<=32) *pFauFanLevel = 1;
			else if (PmValue>32 && PmValue<=78) *pFauFanLevel = 2;
			else if (PmValue>78 && PmValue<=152) *pFauFanLevel = 3;
			else if (PmValue>152) *pFauFanLevel = 4;
			break;
		case 4:
			if (PmValue>=0 && PmValue<=9) *pFauFanLevel = 0;
			else if (PmValue>10 && PmValue<=32) *pFauFanLevel = 1;
			else if (PmValue>32 && PmValue<=82) *pFauFanLevel = 2;
			else if (PmValue>82 && PmValue<=148) *pFauFanLevel = 3;
			else if (PmValue>148) *pFauFanLevel = 4;
			break;
		default:
			if (PmValue>=0 && PmValue<=9) *pFauFanLevel = 0;
			else if (PmValue>10 && PmValue<=32) *pFauFanLevel = 1;
			else if (PmValue>32 && PmValue<=82) *pFauFanLevel = 2;
			else if (PmValue>82 && PmValue<=152) *pFauFanLevel = 3;
			else if (PmValue>152) *pFauFanLevel = 4;
			break;
		}
		break;
	default:
		break;
	}

	return;
}

void GetErvFanLevel (int fauNormalMode, int Co2Value, int *pErvFanLevel) 
{

	if(pErvFanLevel == NULL) 
	{
		return;
	}

	if (fauNormalMode) 
	{
		switch(*pErvFanLevel) 
		{
		case 0: 
			if (Co2Value>=0 && Co2Value<=700) *pErvFanLevel = 0;
			else if (Co2Value>700 && Co2Value<=1000) *pErvFanLevel = 1;
			else if (Co2Value>1000 && Co2Value<=1500) *pErvFanLevel = 2;
			else if (Co2Value>1500 && Co2Value<=2025) *pErvFanLevel = 3;
			else if (Co2Value>2025) *pErvFanLevel = 4;
			break;
		case 1:
			if (Co2Value>=0 && Co2Value<=650) *pErvFanLevel = 0;
			else if (Co2Value>650 && Co2Value<=1000) *pErvFanLevel = 1;
			else if (Co2Value>1000 && Co2Value<=1500) *pErvFanLevel = 2;
			else if (Co2Value>1500 && Co2Value<=2025) *pErvFanLevel = 3;
			else if (Co2Value>2025) *pErvFanLevel = 4;
			break;
		case 2:
			if (Co2Value>=0 && Co2Value<=700) *pErvFanLevel = 0;
			else if (Co2Value>700 && Co2Value<=950) *pErvFanLevel = 1;
			else if (Co2Value>950 && Co2Value<=1500) *pErvFanLevel = 2;
			else if (Co2Value>1500 && Co2Value<=2025) *pErvFanLevel = 3;
			else if (Co2Value>2025) *pErvFanLevel = 4;
			break;
		case 3:
			if (Co2Value>=0 && Co2Value<=700) *pErvFanLevel = 0;
			else if (Co2Value>700 && Co2Value<=1000) *pErvFanLevel = 1;
			else if (Co2Value>1000 && Co2Value<=1450) *pErvFanLevel = 2;
			else if (Co2Value>1450 && Co2Value<=2025) *pErvFanLevel = 3;
			else if (Co2Value>2025) *pErvFanLevel = 4;
			break;
		case 4:
			if (Co2Value>=0 && Co2Value<=700) *pErvFanLevel = 0;
			else if (Co2Value>700 && Co2Value<=1000) *pErvFanLevel = 1;
			else if (Co2Value>1000 && Co2Value<=1500) *pErvFanLevel = 2;
			else if (Co2Value>1500 && Co2Value<=1975) *pErvFanLevel = 3;
			else if (Co2Value>1975) *pErvFanLevel = 4;
			break;
		default:
			if (Co2Value>=0 && Co2Value<=700) *pErvFanLevel = 0;
			else if (Co2Value>700 && Co2Value<=1000) *pErvFanLevel = 1;
			else if (Co2Value>1000 && Co2Value<=1500) *pErvFanLevel = 2;
			else if (Co2Value>1500 && Co2Value<=2025) *pErvFanLevel = 3;
			else if (Co2Value>2025) *pErvFanLevel = 4;
			break;
		}
	}
	else 
	{
		/* RoomInfo[MYROOM].sens.co2_level will determine a real fan level of fau */
		switch(*pErvFanLevel) 
		{
		case 0: 
			if (Co2Value>=0 && Co2Value<=475) *pErvFanLevel = 0;
			else if (Co2Value>475 && Co2Value<=725) *pErvFanLevel = 1;
			else if (Co2Value>725 && Co2Value<=1025) *pErvFanLevel = 2;
			else if (Co2Value>1025 && Co2Value<=2025) *pErvFanLevel = 3;
			else if (Co2Value>2025) *pErvFanLevel = 4;
			break;
		case 1:
			if (Co2Value>=0 && Co2Value<=450) *pErvFanLevel = 0;
			else if (Co2Value>450 && Co2Value<=725) *pErvFanLevel = 1;
			else if (Co2Value>725 && Co2Value<=1025) *pErvFanLevel = 2;
			else if (Co2Value>1025 && Co2Value<=2025) *pErvFanLevel = 3;
			else if (Co2Value>2025) *pErvFanLevel = 4;
			break;
		case 2:
			if (Co2Value>=0 && Co2Value<=475) *pErvFanLevel = 0;
			else if (Co2Value>475 && Co2Value<=675) *pErvFanLevel = 1;
			else if (Co2Value>675 && Co2Value<=1025) *pErvFanLevel = 2;
			else if (Co2Value>1025 && Co2Value<=2025) *pErvFanLevel = 3;
			else if (Co2Value>2025) *pErvFanLevel = 4;
			break;
		case 3:
			if (Co2Value>=0 && Co2Value<=475) *pErvFanLevel = 0;
			else if (Co2Value>475 && Co2Value<=725) *pErvFanLevel = 1;
			else if (Co2Value>725 && Co2Value<=975) *pErvFanLevel = 2;
			else if (Co2Value>975 && Co2Value<=2025) *pErvFanLevel = 3;
			else if (Co2Value>2025) *pErvFanLevel = 4;
			break;
		case 4:
			if (Co2Value>=0 && Co2Value<=475) *pErvFanLevel = 0;
			else if (Co2Value>475 && Co2Value<=725) *pErvFanLevel = 1;
			else if (Co2Value>725 && Co2Value<=1025) *pErvFanLevel = 2;
			else if (Co2Value>1025 && Co2Value<=1975) *pErvFanLevel = 3;
			else if (Co2Value>1975) *pErvFanLevel = 4;
			break;
		default:
			if (Co2Value>=0 && Co2Value<=475) *pErvFanLevel = 0;
			else if (Co2Value>475 && Co2Value<=725) *pErvFanLevel = 1;
			else if (Co2Value>725 && Co2Value<=1025) *pErvFanLevel = 2;
			else if (Co2Value>1025 && Co2Value<=2025) *pErvFanLevel = 3;
			else if (Co2Value>2025) *pErvFanLevel = 4;
			break;
		}
	}

	return;
}

void GetSmellFanLevel (int fauNormalMode, int SmellIaq, int *pSmellFanLevel, int *pSmellLevel ) 
{
	if((pSmellFanLevel == NULL) || (pSmellLevel == NULL))
	{
		return;
	}

	if (fauNormalMode)
	{
		if (SmellIaq <= 150)
		{
			*pSmellFanLevel = 0;
			*pSmellLevel = 0;
		}
		else if (SmellIaq <= 200)
		{
			*pSmellFanLevel = 1;
			*pSmellLevel = 1;
		}
		else if (SmellIaq <= 300) 
		{
			*pSmellFanLevel = 2;
			*pSmellLevel = 1;
		}
		else if (SmellIaq <= 400)
		{
			*pSmellFanLevel = 3;
			*pSmellLevel = 2;
		}
		else
		{
			*pSmellFanLevel = 4;
			*pSmellLevel = 3;
		}
	}
	else
	{
		if (SmellIaq <= 150) 
		{
			*pSmellFanLevel = 1;
			*pSmellLevel = 0;
		}
		else if (SmellIaq <= 250)
		{
			*pSmellFanLevel = 2;
			*pSmellLevel = 1;
		}
		else if (SmellIaq <= 400)
		{
			*pSmellFanLevel = 3;
			*pSmellLevel = 2;
		}
		else
		{
			*pSmellFanLevel = 4;
			*pSmellLevel = 3;
		}
	}

	return;
}

void CtrlMode(CtrlModeTypeT Mode)
{
	AboveTxInfoT *pTxData = NULL;
	CommInfoT *pCommInfo = NULL;

	unsigned char CurrentMode = 0;

	GetCommInfo (&pCommInfo);
	GetAbovTxInfo(&pTxData);

	if(pTxData->Mode == CTRL_MODE_MANUAL)
	{
		if(pTxData->FanLevel == 1)
		{
			CurrentMode = CTRL_MODE_SLEEP;
		}
		else
		{
			CurrentMode = pTxData->Mode;
		}
	}
	else
	{
		CurrentMode = pTxData->Mode;
	}

	if(CurrentMode != Mode)
	{
		if(Mode == CTRL_MODE_SLEEP)
		{
			pTxData->Mode = CTRL_MODE_MANUAL;
			pTxData->FanLevel = 1;
		}
		else if(Mode == CTRL_MODE_MANUAL)
		{
			pTxData->Mode = CTRL_MODE_MANUAL;
			pTxData->FanLevel = 2;
		}
		else
		{
			pTxData->Mode = Mode;
			pTxData->FanLevel = 0;
		}

		pCommInfo->Sync++;
		pCommInfo->SyncWired++;
	}

	return;
}

void CtrlFan(uint8_t FanLevel)
{
	AboveTxInfoT *pTxData = NULL;
	unsigned char enable = 0;
	CommInfoT *pCommInfo = NULL;

	if(FanLevel == 1)
	{
		return;
	}

	GetCommInfo (&pCommInfo);
	GetAbovTxInfo(&pTxData);

	if((pTxData->Mode == CTRL_MODE_MANUAL)&&(pTxData->FanLevel != 1))
	{
		enable = 1;
	}
	else
	{
		enable = 0;
	}

	if(enable)
	{
		if(pTxData->FanLevel != FanLevel)
		{
			pTxData->FanLevel = FanLevel;		
			pCommInfo->Sync++;
			pCommInfo->SyncWired++;
		}
	}

	return;
}

void FocedOffMode(void)
{
	AboveTxInfoT *pTxData = NULL;
	CommInfoT *pCommInfo = NULL;

	GetCommInfo (&pCommInfo);
	GetAbovTxInfo(&pTxData);

	pTxData->Mode = 0;
	pTxData->FanLevel = 0;
	
	pCommInfo->Sync++;
	pCommInfo->SyncWired++;

	return;	
}



///------------------------------------------------------
void AnalysisTaskInit()
{
//    int PmValue = 0, PMLevel = 0;
//	int OldLed = 0, Led = 0, Tmp = 0, OldTmp = 0;
//	OpModeT FauMode = OP_MODE_OFF;
//
//	unsigned char FauLevel = 0;
//	unsigned char index = 0;

	PersistDataInfoT *pPersistDataInfo = NULL;
	SensorInfoT *pSensorInfo = NULL;
	AboveTxInfoT *pTxData = NULL;
	AboveRxInfoT *pRxData = NULL;
	SystemInfoT *pSystemInfo = NULL;

	GetAbovTxInfo(&pTxData);
	GetAbovRxInfo(&pRxData);
	GetSystemInfo(&pSystemInfo);

	GetSensorInfo (&pSensorInfo);
	GetPersistDataInfo (&pPersistDataInfo);
}

///------------------------------------------------------
void AnalysisTask() 
{
	int PmValue = 0, PMLevel = 0;
	int OldLed = 0, Led = 0, Tmp = 0, OldTmp = 0;
	OpModeT FauMode = OP_MODE_OFF;

	unsigned char FauLevel = 0;
	unsigned char index = 0;

//
//	GetAbovTxInfo(&pTxData);
//	GetAbovRxInfo(&pRxData);
//	GetSystemInfo(&pSystemInfo);
//
//	GetSensorInfo (&pSensorInfo);
//	GetPersistDataInfo (&pPersistDataInfo);

//	while(1)
//	{
//		if((pSensorInfo == NULL) || (pPersistDataInfo == NULL)|| (pTxData == NULL) || (pRxData == NULL))
//		{
////			vTaskDelay(100); /* 1s */
////			continue;
//            return;
//		}

		if(pSystemInfo->ConnType == DEV_CONNECTION_NONE)
		{
			OffAllLed();
			FocedOffMode();
//			vTaskDelay(100); /* 1s */
//			continue;
            return;
		}

		if(pTxData->Led == 0)
		{
			FauMode = pTxData->Mode;
			FauLevel = pTxData->FanLevel;

			switch(pSensorInfo->selected_pm)
			{
				case PM_1_0: PmValue = pSensorInfo->pm1_0; break;
				case PM_10_0: PmValue = pSensorInfo->pm10_0; break;
				default: PmValue = pSensorInfo->pm2_5; break;
			}

			if(pSensorInfo->selected_pm == PM_10_0)
			{
				if (PmValue>=0 && PmValue<=30) PMLevel = 0;
				else if (PmValue>30 && PmValue<=80) PMLevel = 1;
				else if (PmValue>80 && PmValue<=150) PMLevel = 2;
				else if (PmValue < 0) PMLevel = 0;
				else PMLevel = 3;
			}
			else
			{
				if (PmValue>=0 && PmValue<=15) PMLevel = 0;
				else if (PmValue>15 && PmValue<=35) PMLevel = 1;
				else if (PmValue>35 && PmValue<=75) PMLevel = 2;
				else if (PmValue < 0) PMLevel = 0;
				else PMLevel = 3;
			}

			if(FauMode > OP_MODE_OFF && FauLevel != FAN_LEVEL_SLEEP)
			{
				if(PMLevel == 0)
				{
					OnLed(LED_DUST_BLUE);
				}
				else if(PMLevel == 1)
				{
					OnLed(LED_DUST_GREEN);
				}
				else if(PMLevel == 2)
				{
					OnLed(LED_DUST_DGREEN);
				}
				else if(PMLevel == 3)
				{
					OnLed(LED_DUST_RED);
				}
			}

			if(FauMode == OP_MODE_OFF)
			{
				OffAllLed();
			}
			else if(FauMode == OP_MODE_AUTO)
			{
				OnLed(LED_POWER);
				OnLed(LED_FAN);
				OffLed(LED_SLEEP);
				OffLed(LED_FAN_LOW);
				OffLed(LED_FAN_MID);
				OffLed(LED_FAN_HIGH);
				OffLed(LED_FAN_TURBO);
				if(pRxData->FltTmr >= pRxData->FltTmrLmt)
				{
					OnLed(LED_FILTER_ERR);
				}
				else
				{
					OffLed(LED_FILTER_ERR);
				}
			}
			else if(FauMode == OP_MODE_NORMAL)
			{
				if(FauLevel == FAN_LEVEL_SLEEP)
				{
					OnLed(LED_SLEEP);
					OffLed(LED_POWER);
					OffLed(LED_FAN);
					OffLed(LED_LOCK);
					OffLed(LED_FILTER_ERR);
					OffLed(LED_AI);
					OffLed(LED_FAN_LOW);
					OffLed(LED_DUST_BLUE);
					OffLed(LED_DUST_GREEN);
					OffLed(LED_DUST_DGREEN);
					OffLed(LED_DUST_RED);
				}
				else
				{
					OnLed(LED_POWER);
					OnLed(LED_FAN);
					OffLed(LED_SLEEP);

					if(pRxData->FltTmr >= pRxData->FltTmrLmt)
					{
						OnLed(LED_FILTER_ERR);
					}
					else
					{
						OffLed(LED_FILTER_ERR);
					}

					switch(FauLevel)
					{
					case FAN_LEVEL_LOW:
						OnLed(LED_FAN_LOW);
						break;
					case FAN_LEVEL_MIDDLE:
						OnLed(LED_FAN_MID);
						break;
					case FAN_LEVEL_HIGH:
						OnLed(LED_FAN_HIGH);
						break;
					case FAN_LEVEL_TURBO:
					case FAN_LEVEL_MAX:
						OnLed(LED_FAN_TURBO);
						break;
					case FAN_LEVEL_OFF:
					default:
						OffLed(LED_FAN_LOW);
						break;
					}
				}
			}
			else
			{
				printf("Unrecoginized mode %d\n", FauMode);
			}
		}
		else
		{
			Led = pTxData->Led;

			if(OldLed != Led)
			{
				if(Led == 0xBFFF)
				{
					OnAllLed();
				}
				else if(Led == 0x8000)
				{
					OffAllLed();
				}
				else if(Led == 0x4000)
				{
                    software_reset();
				}
				else
				{
					Tmp = Led;
					OldTmp = OldLed;

					Tmp &=0x000F;
					OldTmp &=0x000F;
					
					if(OldTmp != Tmp)
					{
						if(OldTmp == 0x0008)
						{
							OffLed(LED_DUST_RED);
						}
						else if(OldTmp == 0x0004)
						{
							OffLed(LED_DUST_DGREEN);
						}
						else if(OldTmp == 0x0002)
						{
							OffLed(LED_DUST_GREEN);
						}
						else if(OldTmp == 0x0001)
						{
							OffLed(LED_DUST_BLUE);
						}

						if(Tmp == 0x0008)
						{
							OnLed(LED_DUST_RED);
						}
						else if(Tmp == 0x0004)
						{
							OnLed(LED_DUST_DGREEN);
						}
						else if(Tmp == 0x0002)
						{
							OnLed(LED_DUST_GREEN);
						}
						else if(Tmp == 0x0001)
						{
							OnLed(LED_DUST_BLUE);
						}
					}

					Tmp = Led;
					OldTmp = OldLed;

					Tmp &=0x03F0;
					Tmp >>= 4; 
					OldTmp &=0x03F0;
					OldTmp >>= 4;
					if(OldTmp != Tmp)
					{
						for(index = 0; index < 6; index++)
						{
							if(Tmp&(0x0001<<index))
							{
								OnLed(LED_POWER + index);
							}
							else
							{
								OffLed(LED_POWER + index);
							}
						}
					}					

					Tmp = Led;
					OldTmp = OldLed;
					
					Tmp &=0x3C00;
					Tmp >>= 10; 
					OldTmp &=0x3C00;
					OldTmp >>= 10;

					if(OldTmp != Tmp)
					{
						if(OldTmp == 0x0008)
						{
							OffLed(LED_FAN_TURBO);
						
						}
						else if(OldTmp == 0x0004)
						{
							OffLed(LED_FAN_HIGH);
						
						}
						else if(OldTmp == 0x0002)
						{
							OffLed(LED_FAN_MID);
						
						}
						else if(OldTmp == 0x0001)
						{
							OffLed(LED_FAN_LOW);							
						}
						
						if(Tmp == 0x0008)
						{
							OnLed(LED_FAN_TURBO);
						}
						else if(Tmp == 0x0004)
						{
							OnLed(LED_FAN_HIGH);
						
						}
						else if(Tmp == 0x0002)
						{
							OnLed(LED_FAN_MID);
						
						}
						else if(Tmp == 0x0001)
						{
							OnLed(LED_FAN_LOW); 						
						}
					
					}
					
				}

				OldLed = Led;
			}
		}

//		vTaskDelay(50); /* 500ms */
//	}
}

#if 0
void RmtActionTask(void* arg) 
{
	unsigned int Evt = 0;
	int rc = 0;

	int Faulevel = 0;
	unsigned char CurrentMode = 0;

	AboveTxInfoT *pTxData = NULL; 
	CommInfoT *pCommInfo = NULL;
	
	GetAbovTxInfo (&pTxData);
	GetCommInfo (&pCommInfo);

	while(1) 
	{
		rc = xQueueReceive(rx_queue, &Evt, 0);
		if(rc) 
		{
			if(pTxData->Mode == OP_MODE_OFF)
			{
				CurrentMode	 = 0;
			}
			else if(pTxData->Mode == OP_MODE_AUTO)
			{
				CurrentMode	 = 1;
			}
			else if(pTxData->Mode == OP_MODE_NORMAL)
			{
				if(pTxData->FanLevel == 1)
				{
					CurrentMode	 = 3;
				}	
				else
				{
					CurrentMode  = 2;
				}
			}

			if(Evt == EVENT_VUP) 
			{
				if(CurrentMode == 2)
				{
					/* Fan Level change... 0(off)->1(low)->2(middle)->3(high)->4(turbo)->5(Max) */
					Faulevel = pTxData->FanLevel;
					Faulevel += 1;
					
					if(Faulevel == 1) 
					{
						Faulevel = 2;
					}
					
					if(Faulevel >= 6) 
					{
						Faulevel = 2;
					}
					
					pTxData->FanLevel = Faulevel;				
					pCommInfo->Sync++;

				}
				else
				{
					Evt = EVENT_TYPE_MAX;
				}
			}
			else if(Evt == EVENT_VDN) 
			{
				/* Fau mode change */

				CurrentMode++;				
				CurrentMode %= 4;

				if(CurrentMode == 2)
				{
					pTxData->Mode = OP_MODE_NORMAL;
					pTxData->FanLevel = 2;
					pCommInfo->Sync++;
				}
				else if(CurrentMode == 3)
				{
					pTxData->Mode = OP_MODE_NORMAL;
					pTxData->FanLevel = 1;
					pCommInfo->Sync++;
				}
				else
				{
					pTxData->Mode = CurrentMode;
					pTxData->FanLevel = 0;
					pCommInfo->Sync++;
				}				

			}

		}
		else
		{
			if(Evt == EVENT_VUP) 
			{
				if(pTxData->FanLevel != Faulevel)
				{
					pTxData->FanLevel = Faulevel;
					pCommInfo->Sync++;					
				}
				else
				{
					Evt = EVENT_TYPE_MAX;
				}
			}
			else if(Evt == EVENT_VDN) 
			{
				if(CurrentMode >= 2)
				{
					if(pTxData->Mode != OP_MODE_NORMAL)
					{
						pTxData->Mode = OP_MODE_NORMAL;

						if(CurrentMode == 3)
						{
							pTxData->FanLevel = 1;
						}
						else
						{
							pTxData->FanLevel = 2;
						}
						pCommInfo->Sync++;
					}
					else
					{
						Evt = EVENT_TYPE_MAX;
					}
				}
				else
				{
					if(pTxData->Mode != CurrentMode)
					{
						pTxData->Mode = CurrentMode;
						pTxData->FanLevel = 0;						
						pCommInfo->Sync++;
					}
					else
					{
						Evt = EVENT_TYPE_MAX;
					}
				}
			}
		}

		vTaskDelay(50); /* 500ms */
			
	}

	return;
}

int RmtCallback(int data) 
{
	int ret = 0;

	if(rx_queue) 
	{
		ret = xQueueSend(rx_queue, &data, portMAX_DELAY);
	}

	OnLed(LED_AI);
	OffLed(LED_AI);
	
	return ret;
}

#else

void RmtCallback(RemoteEventT *pEvt) 
{
	int ret = 0;
/*
	if(rx_queue) 
	{
		ret = xQueueSend(rx_queue, &data, portMAX_DELAY);
	}
*/
	AboveTxInfoT *pTxData = NULL; 
	CommInfoT *pCommInfo = NULL;
	SystemInfoT *pSystemInfo = NULL;

	GetAbovTxInfo (&pTxData);
	GetCommInfo (&pCommInfo);
	GetSystemInfo (&pSystemInfo);

	printf("## RmtCallback ( %d: %d: %d: %d: %d ) !!! \r\n", pEvt->Mode, pEvt->FanLevel, pEvt->ResetFlt, pEvt->ResvOn, pEvt->ResvTime); 
	
	if(pEvt->Mode == 0)
	{
		if(pTxData->Mode != pEvt->Mode)
		{
			pTxData->Mode = pEvt->Mode;
			pTxData->FanLevel = 0;						
			pCommInfo->Sync++;
			pCommInfo->SyncWired++;
		}
	}
	else if(pEvt->Mode == 1)
	{
		if(pTxData->Mode != pEvt->Mode)
		{
			pTxData->Mode = pEvt->Mode;
			pCommInfo->Sync++;
			pCommInfo->SyncWired++;
		}
		
		if(pTxData->FanLevel != pEvt->FanLevel)
		{
			pTxData->FanLevel = pEvt->FanLevel;
			pCommInfo->Sync++;
			pCommInfo->SyncWired++;
		}

	}
	else if(pEvt->Mode == 2)
	{
		if(pTxData->Mode != pEvt->Mode)
		{
			pTxData->Mode = pEvt->Mode;
			pCommInfo->Sync++;
			pCommInfo->SyncWired++;
		}
		
		if(pTxData->FanLevel != pEvt->FanLevel)
		{
			pTxData->FanLevel = pEvt->FanLevel;
			pCommInfo->Sync++;
			pCommInfo->SyncWired++;
		}
	}

	if(pTxData->FltTmrRst != pEvt->ResetFlt)
	{
		pTxData->FltTmrRst = pEvt->ResetFlt;
		pCommInfo->Sync++;
		pCommInfo->SyncWired++;
	}

	if(pEvt->ResvOn == 1)
	{
		if(pSystemInfo->ResvStatus == 0)
		{
			EnableFauResvTime(pEvt->ResvTime);
			pSystemInfo->ResvTimeSet = 1;
			pSystemInfo->ResvStatus = 1;
			pCommInfo->Sync++;
			pCommInfo->SyncWired++;
			printf("resv on, time : %d \r\n", pSystemInfo->ResvTime);
		}
		else if(pSystemInfo->ResvStatus && pEvt->ResvTime != pSystemInfo->ResvTime)
		{
			EnableFauResvTime(pEvt->ResvTime);
			pSystemInfo->ResvTimeSet = 1;
			pCommInfo->Sync++;
			pCommInfo->SyncWired++;
			printf("resv uptadte, time : %d \r\n", pSystemInfo->ResvTime);
		}
	}
	else
	{
		if(pSystemInfo->ResvStatus)
		{
			DisableFauResvTime(pSystemInfo->ResvTimerInstance);
			pSystemInfo->ResvTime = 0;
			pSystemInfo->ResvTimer = 0;
			pSystemInfo->ResvTimeSet = 1;
			pSystemInfo->ResvTimerInstance = -1;
			pSystemInfo->ResvStatus = 0;
			pCommInfo->Sync++;
			pCommInfo->SyncWired++;
			printf("resv off\n");
		}
	}

	return;
}
#endif

void PIDTask() 
{


	long long activeTime = 0, diffTime = 0;

	GetAbovTxInfo(&pTxData);
	GetAbovRxInfo(&pRxData);
	GetPersistDataInfo(&pPersistDataInfo);

		FanLevel = pRxData->FanLevel;

#if 0
    switch(FanLevel)
    {
        case 1:
            Rpm = PID_TARGET_PPS1;
            break;
        case 2:
            Rpm = PID_TARGET_PPS2;
            break;
        case 3:
            Rpm = PID_TARGET_PPS3;
            break;
        case 4:
            Rpm = PID_TARGET_PPS4;
            break;
        case 5:
            Rpm = PID_TARGET_PPS5;
            break;
        default:
            Rpm = 0;
            break;
    }
#else
    switch(FanLevel)
    {
        case 1:
            Rpm = pPersistDataInfo->rpm1;
            break;
        case 2:
            Rpm = pPersistDataInfo->rpm2;
            break;
        case 3:
            Rpm = pPersistDataInfo->rpm3;
            break;
        case 4:
            Rpm = pPersistDataInfo->rpm4;
            break;
        case 5:
            Rpm = pPersistDataInfo->rpm5;
            break;
        default:
            Rpm = 0;
            break;
    }
#endif

    if ((FanLevel != OldFanLevel) || (Rpm != OldRpm))
    {
        printf("### Rpm (%d:%d) \r\n", Rpm, OldRpm);
    
        OldFanLevel = FanLevel;			
        OldRpm = Rpm;
//      activeTime = (esp_timer_get_time()/1000);
    }

    Input = pRxData->PPS;
    SetPoint = Rpm;
    Output = 0;
    
//  diffTime = ((esp_timer_get_time()/1000) - activeTime);
    
    if((diffTime >= 7000) && (activeTime != 0))
    {
        rc = PID_Compute(pPID);
        if(rc == 0)
        {
            converted_VSPOffset = Output;
            pTxData->VSPOffset = converted_VSPOffset;
        }
    }
}


int ShowFauInfo(int argc, char **argv) 
{

	SensorInfoT *pSensorInfo = NULL;
	AboveTxInfoT *pTxData = NULL;
	AboveRxInfoT *pRxData = NULL;

	GetAbovTxInfo(&pTxData);
	GetAbovRxInfo(&pRxData);	
	GetSensorInfo (&pSensorInfo);

	if((pSensorInfo == NULL)|| (pTxData == NULL)|| (pRxData == NULL)) 
	{
		return -1;
	}

	printf("# Fau above's SW version(%d) \r\n", pRxData->Ver);
	printf("# Fau mode(%d : %d) level(%d : %d) pmLevel(%d)\r\n", pTxData->Mode, pRxData->Mode, pTxData->FanLevel, pRxData->FanLevel, pSensorInfo->pm_level);
	printf("# Fau VSPOffset(%d : %d) vsp(%02d:%02d:%02d:%02d:%02d) pps(%4d)\r\n", pTxData->VSPOffset, pRxData->VSPOffset, pRxData->VSP[0], pRxData->VSP[1], pRxData->VSP[2], pRxData->VSP[3], pRxData->VSP[4], pRxData->PPS);
	printf("# Fau RPM %d:%d:%d:%d:%d\n", pTxData->RPM[0], pTxData->RPM[1], pTxData->RPM[2], pTxData->RPM[3], pTxData->RPM[4]);
	printf("# Fau flttmr(%04d) flttmrlmt(%04d) \r\n", pTxData->FltTmr, pTxData->FltTmrLmt);
	printf("# Fau Err(0x%x) \r\n", pRxData->Err);

	return 0;
}

void Register_GetFauInfo(void) 
{
//    const esp_console_cmd_t cmd = {
//        .command = "fau",
//        .help = "Get fau info",
//        .hint = NULL,
//        .func = &ShowFauInfo,
//		.argtable = NULL
//    };
//    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}

void ResrvTimerFunction(void *pArg) 
{
	SystemInfoT *pSystemInfo = NULL;
	int *pInstance = NULL;

	if(pArg == NULL) 
	{
		return;
	}

	pInstance = (int *)pArg;
	GetSystemInfo (&pSystemInfo);

	if(pSystemInfo) 
	{
		if (pSystemInfo->ResvTimer > 0) 
		{
			pSystemInfo->ResvTimer--;
			printf("Timer %d\n", pSystemInfo->ResvTimer);
			if (pSystemInfo->ResvTimer <= 0) 
			{
				pSystemInfo->ResvTime = pSystemInfo->ResvTimer = 0;
				DeleteTimerInstance(*pInstance);
				pSystemInfo->ResvTimerInstance = -1;
				FocedOffMode();
			}
			else 
			{
				pSystemInfo->ResvTime = (pSystemInfo->ResvTimer/3600) + 1;
			}
			
		}
	}

	return;
}

int EnableFauResvTime(int ResvTime) 
{
	SystemInfoT *pSystemInfo = NULL;

	GetSystemInfo (&pSystemInfo);

	if(( pSystemInfo == NULL) || ( ResvTime  >= 13 ))
	{
		printf("Failed to enable Reservetimer\n");
		return -1;
	}

	pSystemInfo->ResvTime = ResvTime;
	pSystemInfo->ResvTimer = ResvTime*3600;

	if(pSystemInfo->ResvTimerInstance == -1)
	{
//		pSystemInfo->ResvTimerInstance = CreateTimer("FAUTimer", 1, 1000000, &ResrvTimerFunction);
		printf("Creater timer instance %d\n", pSystemInfo->ResvTimerInstance);
	}

	return 0;
}

void DisableFauResvTime(     int instance) 
{

	DeleteTimerInstance(instance);

	return;
}

int MakeReservation(int argc, char **argv) 
{
	int result = 0;
    int nerrors = arg_parse(argc, argv, (void **) &CmdArgs);
	
    if (nerrors != 0) 
	{
        arg_print_errors(stderr, CmdArgs.pEnd, argv[0]);
        return -1;
    }

	if((CmdArgs.pRevHour->ival[0] < 1 )|| (CmdArgs.pRevHour->ival[0] > 12 )) 
	{
		return -2;
	}

//	RevTimerInstance = EnableFauResvTime(CmdArgs.pRevHour->ival[0]);

	return result;
}


void Register_Reservation (void) 
{
	CmdArgs.pRevHour = arg_int0(NULL, "time", "<1~12>", "reservation time( 1hour ~ 12 hour )");
    CmdArgs.pEnd = arg_end(1);
	
//    const esp_console_cmd_t CmdRev = {
//        .command = "mkrev",
//        .help = "mkrev time",
//        .hint = NULL,
//        .func = &MakeReservation,
//	    .argtable = &CmdArgs
//        
//    };
		
//    ESP_ERROR_CHECK( esp_console_cmd_register(&CmdRev) );
}

static int CommandFau(int argc, char **argv) 
{
	int result = 0;
	CtrlModeTypeT data = CTRL_MODE_OFF;
    uint8_t CtrlFanData = 0;

    int nerrors = arg_parse(argc, argv, (void **) &FauCmd);
    if (nerrors != 0) 
	{
        arg_print_errors(stderr, FauCmd.end, argv[0]);
        return 1;
    }

	if(FauCmd.command->ival[0] == 0) 
	{
		switch(FauCmd.step->ival[0])
		{
			case 0:
			default:
				data = CTRL_MODE_OFF;
			break;
			case 1:
				data = CTRL_MODE_AUTO;
			break;
			case 2:
				data = CTRL_MODE_MANUAL;
			break;
			case 3:
				data = CTRL_MODE_SLEEP;
			break;
		}
	
		CtrlMode(data);
		
	}
	else if(FauCmd.command->ival[0] == 1) 
	{
		switch(FauCmd.step->ival[0])
		{
			case 0:
				CtrlFanData = 0;
			break;
			case 2:
				CtrlFanData = 2;
			break;
			case 3:
				CtrlFanData = 3;
			break;
			case 4:
				CtrlFanData = 4;
			break;
			case 5:
				CtrlFanData = 5;
			break;
			default:
				CtrlFanData = 2;
			break;
		}
	
		CtrlFan(CtrlFanData);
	}

    return result;
}

void Register_CtrlFau (void) 
{
	FauCmd.command = arg_int0(NULL, "com", "<0|1>", "fau's com( 0: mode, 1: fan level)");
	FauCmd.step = arg_int1(NULL, "num", "< 0 ~ 3, 2 ~ 5 >", "fau mode (0,1,2,3), fan level (2,3,4,5)");
	FauCmd.end = arg_end(2);
	
//	const esp_console_cmd_t cmd_fau = 
//	{
//	.command = "fauc",
//	.help = "fauc command num",
//	.hint = NULL,
//	.func = &CommandFau,
//	.argtable = &FauCmd
//	};
		
//	ESP_ERROR_CHECK( esp_console_cmd_register(&cmd_fau) );
}

void InitControlFuncs(void) 
{
    
////	rx_queue = xQueueCreate(1, sizeof(uint32_t));
//	xTaskCreatePinnedToCore(AnalysisTask, "Analysis Task", 4096, NULL, 5, NULL, 1);	
////	xTaskCreatePinnedToCore(RmtActionTask, "Action Task", 4096, NULL, 5, NULL, 1);	
//	xTaskCreatePinnedToCore(PIDTask, "PID Task", 4096, NULL, 5, NULL, 1);	

	memset(&(PIDCtrl),0,sizeof(PIDCtrlT));
	pPID = PID_Create(&(PIDCtrl), &Input, &Output, &SetPoint, KP, KI, KD);

	GetAbovTxInfo(&pTxData);
	GetAbovRxInfo(&pRxData);
	GetPersistDataInfo(&pPersistDataInfo);

	if((pTxData == NULL) || (pRxData == NULL) || (pPID == NULL) || (pPersistDataInfo == NULL))
	{
		return;
	}

	PID_Limits(pPID, -100, 100);

	PID_Auto(pPID);
	return;
}


