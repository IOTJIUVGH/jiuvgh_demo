#ifndef __JTimer_H
#define __JTimer_H

#include "stm32f10x.h"

enum
{
  FALSE_V,
  TRUE_V
};

typedef enum
{
  JTIM0,
  JTIM1,
  JTIM2,
  JTIM3,
  JTIM4,
  JTIM5,
  JTIM6,
  JTIM7,
  JTIM8,
  JTIM9,
  JTIM10,
  JTIMER_NUM
} jtimer_name_t;

typedef u32 timer_res_t;

typedef struct
{
  timer_res_t msec;
  void *pCallback;
} jtimer_t, *jvtimer;

void jtimer_init(void);
void jtimer_settimer(jtimer_name_t name, timer_res_t msec, void *pCallback);
void jtimer_killtimer(jtimer_name_t name);
uint8_t jtimer_timerelapsed(jtimer_name_t name);

#endif
