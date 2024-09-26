
#ifndef __CO2_H__
#define __CO2_H__

typedef struct DrvCo2Dev
{
	unsigned long ScanPeriod;
	void (*fnCo2Callback)(int);
}DrvCo2DevT;

void InitCo2 (DrvCo2DevT *pCo2dev);

#endif /* __CO2_H__ */

