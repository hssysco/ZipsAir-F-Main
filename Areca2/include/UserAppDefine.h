//==============================================================================
// UserAppDefine.h
//==============================================================================
/* Includes ------------------------------------------------------------------*/
//#include "main.h"
//#include "stm32f0xx_hal.h"
#include "gd32f30x.h"

typedef enum
{
	False = 0,
	True
} Bool;

typedef enum
{
	false = 0,
	true
} bool;

typedef enum
{
	OnEdge = 0,	
  	On,
	OffEdge,
	Off
} SW_Status;

typedef int esp_err_t;


#define TESTBIT(ADDRESS,BIT)  	(ADDRESS & (1<<BIT))
#define SETBIT(ADDRESS,BIT)		(ADDRESS |= (1<<BIT))
#define CLEARBIT(ADDRESS,BIT)	(ADDRESS &= ~(1<<BIT))

// Switch define
#define	SW_PWR	0
#define	SW_UP	1
#define	SW_STOP	2
#define	SW_DN	3
#define	SW_ALL	4
#define	SW_1	5
#define	SW_2	6
#define	SW_3	7
#define	SW_4	8
#define	SW_5	9
#define	SW_6	10
#define	SW_7	11
#define	SW_8	12
#define	SW_9	13
#define	SW_10	14
#define	SW_11	15
#define	SW_12	16

#define	UP1     0
#define	UP2     2
#define	UP3     4
#define	UP4     6
#define	UP5     8
#define	UP6     10
#define	UP7     12
#define	UP8     14
#define	UP9     16
#define	UP10    18
#define	UP11    20
#define	UP12    22

#define	DN1     1
#define	DN2     3
#define	DN3     5
#define	DN4     7
#define	DN5     9
#define	DN6     11
#define	DN7     13
#define	DN8     15
#define	DN9     17
#define	DN10    19
#define	DN11    21
#define	DN12    23
