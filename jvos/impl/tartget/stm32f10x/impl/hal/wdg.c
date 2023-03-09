#include "stm32f10x_iwdg.h"
#include "jerr.h"

merr_t mhal_wdg_open(uint32_t timeout)
{
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);  
	
	IWDG_SetPrescaler(IWDG_Prescaler_64);  
	
	IWDG_SetReload(0x0fff & ((uint16_t) (0.625 * timeout)));  
	
	IWDG_ReloadCounter(); 
	
	IWDG_Enable();

    return kNoErr;
}

void mhal_wdg_feed(void)
{
    IWDG_ReloadCounter();		
}

merr_t mhal_wdg_close(void)
{
    return kCanceledErr;
}
