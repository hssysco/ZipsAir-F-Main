
#ifndef __PRESSURE_H__
#define __PRESSURE_H__
#include "bsec_integration.h"

typedef struct PressureEvent
{
	float temperature;
	float pressure;
	float altitude;
	float gas;
	float humidity;
	float iaq;
}PressureEventT;

typedef struct DrvPressureDev
{
	unsigned long ScanPeriod;
	void (*fnPressureCallback)(PressureEventT *);
}DrvPressureDevT;

void InitPressure (DrvPressureDevT *pPressureDev);

#endif /* __PRESSURE_H__ */

