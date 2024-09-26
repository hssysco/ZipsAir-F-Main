#include <stdio.h>
#include <string.h>
//#include <unistd.h>
//#include <esp_timer.h>
//#include <esp_log.h>
//#include <esp_sleep.h>

#include "app.h"

#define TICK_SECOND	1000

PIDCtrlT *PID_Create(PIDCtrlT *pPID, float* in, float* out, float* set, float kp, float ki, float kd)
{
	if(pPID == NULL)
	{
		return NULL;
	}
	
	pPID->input = in;
	pPID->output = out;
	pPID->setpoint = set;
	pPID->automode = false;

	PID_Limits(pPID, 0, 255);

	// Set default sample time to 100 ms
//	pPID->sampletime = 100;
	pPID->sampletime = 1000;

	PID_Direction(pPID, E_PID_DIRECT);

	PID_Tune(pPID, kp, ki, kd);

//	pPID->lasttime = (esp_timer_get_time()/1000) - pPID->sampletime;

	return pPID;
}

int PID_Compute(PIDCtrlT *pPID)
{
	float in = 0, error =0, dinput = 0, out = 0;

	if(pPID == NULL)
	{
		return -1;
	}

	// Check if control is enabled
	if (!pPID->automode)
	{
		return -1;
	}
	
	in = *(pPID->input);
	// Compute error
	error = (*(pPID->setpoint)) - in;
	
	// Compute integral
	pPID->iterm += (pPID->Ki * error);
	
	if (pPID->iterm > pPID->omax)
	{
		pPID->iterm = pPID->omax;
	}
	else if (pPID->iterm < pPID->omin)
	{
		pPID->iterm = pPID->omin;
	}

	// Compute differential on input
	dinput = in - pPID->lastin;

	// Compute PID output
	out = pPID->Kp * error + pPID->iterm - pPID->Kd * dinput;
	
	// Apply limit to output value
	if (out > pPID->omax)
	{
		out = pPID->omax;
	}
	else if (out < pPID->omin)
	{
		out = pPID->omin;
	}

	// Output to pointed variable
	(*pPID->output) = out;
	// Keep track of some variables for next execution
	pPID->lastin = in;
//	pPID->lasttime = (esp_timer_get_time()/1000);

	return 0;
}

void PID_Tune(PIDCtrlT *pPID, float kp, float ki, float kd)
{
	// Check for validity
	if (pPID == NULL || kp < 0 || ki < 0 || kd < 0)
		return;
	
	//Compute sample time in seconds
	float ssec = ((float) pPID->sampletime) / ((float) TICK_SECOND);

	pPID->Kp = kp;
	pPID->Ki = ki * ssec;
	pPID->Kd = kd / ssec;

	if (pPID->direction == E_PID_REVERSE) 
	{
		pPID->Kp = 0 - pPID->Kp;
		pPID->Ki = 0 - pPID->Ki;
		pPID->Kd = 0 - pPID->Kd;
	}
}

void PID_Limits(PIDCtrlT *pPID, float min, float max)
{
	if (pPID == NULL || (min >= max)) return;
	
	pPID->omin = min;
	pPID->omax = max;
	//Adjust output to new limits
	if (pPID->automode) 
	{
		if (*(pPID->output) > pPID->omax)
			*(pPID->output) = pPID->omax;
		else if (*(pPID->output) < pPID->omin)
			*(pPID->output) = pPID->omin;

		if (pPID->iterm > pPID->omax)
			pPID->iterm = pPID->omax;
		else if (pPID->iterm < pPID->omin)
			pPID->iterm = pPID->omin;
	}
}

void PID_Auto(PIDCtrlT *pPID)
{
	if(pPID == NULL)
	{
		return;
	}

	// If going from manual to auto
	if (!pPID->automode) 
	{
		pPID->iterm = *(pPID->output);
		pPID->lastin = *(pPID->input);

		if (pPID->iterm > pPID->omax)
		{
			pPID->iterm = pPID->omax;
		}
		else if (pPID->iterm < pPID->omin)
		{
			pPID->iterm = pPID->omin;
		}

		pPID->automode = true;
	}
}

void PID_Manual(PIDCtrlT *pPID)
{
	if(pPID == NULL)
	{
		return;
	}

	pPID->automode = false;
}

void PID_Direction(PIDCtrlT *pPID, PIDCtrlDirT dir)
{
	if(pPID == NULL)
	{
		return;
	}

	if (pPID->automode && pPID->direction != dir) 
	{
		pPID->Kp = (0 - pPID->Kp);
		pPID->Ki = (0 - pPID->Ki);
		pPID->Kd = (0 - pPID->Kd);
	}
	
	pPID->direction = dir;
}

