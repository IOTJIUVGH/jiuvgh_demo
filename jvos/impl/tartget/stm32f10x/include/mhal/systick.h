#ifndef __SYSTICK_H
#define __SYSTICK_H 			   
  
#include "stm32f10x.h"

void systick_init(void);
void delay_us(uint32_t nus);
void delay_xms(uint32_t nms);
#endif
