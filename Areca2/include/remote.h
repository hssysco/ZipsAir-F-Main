#ifndef __REMOTE_H__
#define __REMOTE_H__

/*
typedef enum EventType{
	EVENT_POWER = 0,
	EVENT_VUP,
	EVENT_VDN,
	EVENT_CUP,
	EVENT_CDN,
	EVENT_LEFT,
	EVENT_RIGHT,
	EVENT_CENTER,
	EVENT_TYPE_MAX
}EventTypeT;

typedef struct DrvRmtDev
{
	int (*fnRmtCallback)(int);

}DrvRmtDevT;

*/

typedef struct RemoteEvent
{
	unsigned char Mode;
	unsigned char FanLevel;
	unsigned char ResetFlt;
	unsigned char ResvOn;
	unsigned char ResvTime;
}RemoteEventT;

typedef struct DrvRmtDev
{
	void (*fnRmtCallback)(RemoteEventT *);

}DrvRmtDevT;


void InitRemote (DrvRmtDevT *pRmtDev);

#endif /* __REMOTE_H__ */

