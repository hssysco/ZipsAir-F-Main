#include <stdio.h>
#include <string.h>
//#include <unistd.h>
//#include "esp_timer.h"
//#include "esp_log.h"
//#include "esp_sleep.h"
//#include "sdkconfig.h"

#include "app.h"

#define TIMER_MAX_NUM	20
#define STR_NUM			32

typedef struct TimerConfig {
	char used;
	char name[STR_NUM];
	char periodicflag;
	unsigned long long period_us;
	int instance;
//	esp_timer_handle_t timerHandler;	
}TimerConfigT;

TimerConfigT TimerConfig[TIMER_MAX_NUM];

int GetAvailableInstance(void) 
{
	unsigned int instance = 0, i = 0;

	for(i = 0; i < TIMER_MAX_NUM; i++) 
	{
		if(TimerConfig[i].used == 0) 
		{
			break;
		}
	}

	if(i != TIMER_MAX_NUM) 
	{
		instance = i;
	}
	else 
	{
		instance = -1;
	}

	return instance;
}

int CreateTimer( char *pTimerName, char periodicflag, uint64_t period_us, void *pFunction ) 
{

	int instance = 0, dataLen = 0;
//	esp_err_t err = 0;
//	esp_timer_create_args_t timer_args; 
//	esp_timer_handle_t timerHandle = NULL;

	if((pTimerName == NULL) || (pFunction == NULL)) 
	{
		printf("%s:%d Invalid parameter\n", __FUNCTION__, __LINE__);
		return -1;
	}

	instance = GetAvailableInstance();
	if(instance < 0) 
	{
		printf("%s:%d No valid timer exists\n", __FUNCTION__, __LINE__);
		return -2;
	}

	TimerConfig[instance].instance = instance;

//	memset(&timer_args,0,sizeof(esp_timer_create_args_t));	
//	timer_args.callback = pFunction;
//	timer_args.name = pTimerName; 
//	timer_args.arg = (void *)&(TimerConfig[instance].instance);
//
//	err = esp_timer_create(&timer_args, &timerHandle);

//	if(err != ESP_OK )
//	{
//		printf("%s:%d esp timer create fail\n", __FUNCTION__, __LINE__);
//		return -3;
//	}
//
//	if(periodicflag) 
//	{
//		err = 0;
//		err = esp_timer_start_periodic(timerHandle, period_us);
//	}
//	else 
//	{
//		err = esp_timer_start_once(timerHandle, period_us);
//	}	
//
//	if(err != ESP_OK )
//	{
//		printf("%s:%d esp timer start fail\n", __FUNCTION__, __LINE__);
//
//		if(timerHandle != NULL) 
//		{
//			esp_timer_stop(timerHandle);
//			esp_timer_delete(timerHandle);
//			timerHandle = NULL;
//		}
//	}
//	else 
//	{
//		TimerConfig[instance].used  = 1;
//		TimerConfig[instance].periodicflag = periodicflag;
//		TimerConfig[instance].period_us = period_us;
//		TimerConfig[instance].timerHandler = timerHandle;
//
//		dataLen = strlen(pTimerName);
//
//		if(dataLen >= STR_NUM) 
//		{
//			sprintf(TimerConfig[instance].name, "TIMER%d", instance);		
//		}
//		else 
//		{
//			strncpy(TimerConfig[instance].name, pTimerName, dataLen );
//		}
//	}

	return instance;
}

void DeleteTimerInstance (int Instance) 
{

	if((Instance >= TIMER_MAX_NUM) || (TimerConfig[Instance].used  == 0)) 
	{
		printf("Request delete invalid timerinstance %d\n", Instance);
		return;
	}

//	esp_timer_stop(TimerConfig[Instance].timerHandler);
//	esp_timer_delete(TimerConfig[Instance].timerHandler);

	TimerConfig[Instance].used = 0;
	TimerConfig[Instance].periodicflag = 0;
	TimerConfig[Instance].period_us = 0;
//	TimerConfig[Instance].timerHandler = NULL;

	return;
}

void DeleteTimerHandler ( void *pTimerHandler ) 
{

	unsigned int i = 0;

	if(pTimerHandler  == NULL) 
	{
		return;
	}

	for(i = 0; i < TIMER_MAX_NUM; i++) 
	{
//		if((TimerConfig[i].used)&&(TimerConfig[i].timerHandler == pTimerHandler))
//		{
//			break;
//		}
	}

	if(i >= TIMER_MAX_NUM) 
	{
		return;
	}

//	esp_timer_stop(TimerConfig[i].timerHandler);
//	esp_timer_delete(TimerConfig[i].timerHandler);

	TimerConfig[i].used = 0;
	TimerConfig[i].periodicflag = 0;
	TimerConfig[i].period_us = 0;
//	TimerConfig[i].timerHandler = NULL;

	return;
}


