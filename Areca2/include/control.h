#ifndef __CTRL_H__
#define __CTRL_H__
void Register_Reservation (void);
void Register_GetFauInfo(void);
void Register_CtrlFau (void);
//int RmtCallback(int data);
void RmtCallback(RemoteEventT *pEvt);

void InitControlFuncs (void);

void ResrvTimerFunction(void *pArg);
int EnableFauResvTime(     int ResvTime );
void DisableFauResvTime(int instance);


typedef enum CtrlModeType{
	CTRL_MODE_OFF = 0,
	CTRL_MODE_AUTO,
	CTRL_MODE_MANUAL,
	CTRL_MODE_SLEEP
}CtrlModeTypeT;

void CtrlMode(CtrlModeTypeT Mode);
void CtrlFan(unsigned char FanLevel);
void FocedOffMode(void);

#endif

