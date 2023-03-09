#include "jtimer.h"
#include "gtimer.h"

static jtimer_t svtimer[JTIMER_NUM]; 
typedef void(*PFN_Callback_t)(void);

void jtimer_init()
{
	for (int i = 0; i < JTIMER_NUM; i++)
	{
		svtimer[i].msec = 0;
		svtimer[i].pCallback = 0;
	}
	
	mhal_gtimer_open(TIM2, PERIOIC, 1,  &Vtimer_UpdateHandler, NULL);
	mhal_gtimer_start(TIM2)
}

void jtimer_settimer(jtimer_name_t name,timer_res_t  msec,void* pCallback)  
{
	svtimer[name].msec = msec;
	svtimer[name].pCallback = pCallback;
}

void jtimer_killtimer(jtimer_name_t name)
{
	svtimer[name].msec = 0;
	svtimer[name].pCallback = 0;
}

u8 jtimer_timerelapsed(jtimer_name_t name)
{
	if (svtimer[name].msec == 0)
		return TRUE_V;
	else
		return FALSE_V;
}

void jtimer_updatehandler(void)    		
{
	for (int i = 0; i < JTIMER_NUM; i++)  //11
	{
		if (svtimer[i].msec != 0)
		{
			svtimer[i].msec--;
			if (svtimer[i].pCallback != 0)
			{
				if (svtimer[i].msec == 0) 
				{
					((PFN_Callback_t)svtimer[i].pCallback)();
				}
			}
		}
	}
}
