#include "systick.h"
#include "misc.h"
#include "FreeRTOS.h"				
#include "task.h"

static uint8_t  fac_us=0;						
static uint16_t fac_ms=0;					
	

void xPortSysTickHandler( void );

void SysTick_Handler(void)
{	
	if(xTaskGetSchedulerState()!=taskSCHEDULER_NOT_STARTED)
	{
		xPortSysTickHandler();
	}
}

void systick_init(void)
{	
	uint32_t reload;

 	SysTick->CTRL&=~(1<<2);				
	fac_us=0xa8;						
				
	reload=0xa8;						
	reload*=1000000/configTICK_RATE_HZ;	
										
	fac_ms=1000/configTICK_RATE_HZ;		
	SysTick->CTRL|=1<<1;   			
	SysTick->LOAD=reload; 				
	SysTick->CTRL|=1<<0;   				

	fac_ms=(uint16_t)fac_us*1000;				
}								    
								   
void delay_us(uint32_t nus)
{		
	uint32_t ticks;
	uint32_t told,tnow,tcnt=0;
	uint32_t reload=SysTick->LOAD;				
	ticks=nus*fac_us; 					
	told=SysTick->VAL;        		
	while(1)
	{
		tnow=SysTick->VAL;	
		if(tnow!=told)
		{	    
			if(tnow<told)tcnt+=told-tnow;	
			else tcnt+=reload-tnow+told;	    
			told=tnow;
			if(tcnt>=ticks)break;			
		}  
	};										    
}  

void delay_xms(uint32_t nms)
{
	uint32_t i;
	for(i=0;i<nms;i++) delay_us(1000);
}
			 
