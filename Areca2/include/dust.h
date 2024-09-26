#ifndef __DUST_H__
#define __DUST_H__

enum {
	ADJ_MODE_GRIMM = 0,
	ADJ_MODE_RAW,
	ADJ_MODE_TSI = ADJ_MODE_RAW,
	ADJ_MODE_MAX
};


#define SEN_PIN                         GPIO_PIN_3
#define SEN_GPIO_PORT                   GPIOB
#define SEN_GPIO_CLK                    RCU_GPIOB

typedef struct DustEvent{
	unsigned long pm_1_0;
	unsigned long pm_2_5;
	unsigned long pm_10_0;
}DustEventT;

typedef struct DustDev
{
	int mAdjMode;
	unsigned long ScanPeriod;
	void (*fnDustCallback)(DustEventT *);
}DrvDustDevT;

void InitDust (DrvDustDevT *pDustDev);

#endif /* __DUST_H__ */

