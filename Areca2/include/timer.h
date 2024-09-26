#ifndef __TIMER_H__
#define __TIMER_H__

int CreateTimer( char *pTimerName, char periodicflag, uint64_t period_us, void *pFunction );
void DeleteTimerInstance ( int Instance );
void DeleteTimerHandler ( void *pTimerHandler );

#endif
