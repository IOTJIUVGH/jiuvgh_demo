#include "jvos.h"
#include "peripheral_remap.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_rcc.h"
#include "misc.h"
#include "jerr.h"
#include "gtimer.h"

#define CLK_10KHZ      7199

static TIM_TypeDef *
    timer_remap[] =
        {
            TIM2,
            TIM3,
            TIM4};

static const uint32_t timer_rcc_remap[] =
    {
        RCC_APB1Periph_TIM2,
        RCC_APB1Periph_TIM3,
        RCC_APB1Periph_TIM4};

static const uint32_t timer_exit_irqn[] =
    {
        TIM2_IRQn,
        TIM3_IRQn,
        TIM4_IRQn};

typedef void (*hal_timer_cb_t)(void *arg);


typedef struct 
{
   hal_timer_cb_t irq_handler;
   uint32_t* arg;
} timer_irq_fun_t;


static timer_irq_fun_t irq_fun[ TIM_NUM ] = {0};

merr_t mhal_gtimer_open(mhal_gtimer_t timer,mhal_gtimer_mode_t mode, uint32_t time, mhal_gtimer_irq_callback_t function, void *arg)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;


	RCC_APB1PeriphClockCmd(timer_rcc_remap[ timer ], ENABLE); //时钟使能

    irq_fun[ timer ].irq_handler = function;
    irq_fun[ timer ].arg = (uint32_t *)arg;

	TIM_TimeBaseStructure.TIM_Period = 10 * time -1; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	 计数到5000为500ms
	TIM_TimeBaseStructure.TIM_Prescaler =CLK_10KHZ; //设置用来作为TIMx时钟频率除数的预分频值  10Khz的计数频率  
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(timer_remap[ timer ], &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位

    NVIC_InitStructure.NVIC_IRQChannel = timer_exit_irqn[timer];  
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 15;  
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
	NVIC_Init(&NVIC_InitStructure);  

    return kNoErr;
}

merr_t mhal_gtimer_start(mhal_gtimer_t timer)
{
    TIM_Cmd(timer_remap[timer], ENABLE);

    return kNoErr;
}

merr_t mhal_gtimer_stop(mhal_gtimer_t timer)
{
   return kGenericErrorEnd;
}

merr_t mhal_gtimer_close(mhal_gtimer_t timer)
{
    TIM_DeInit(timer_remap[timer]);

    return kNoErr;
}

void TIM2_IRQHandler(void)   //TIM3中断
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) //检查指定的TIM中断发生与否:TIM 中断源 
		{
		    TIM_ClearITPendingBit(TIM3, TIM_IT_Update  );  //清除TIMx的中断待处理位:TIM 中断源 
            irq_fun[ TIM_2 ].irq_handler(irq_fun[ TIM_2 ].arg);
		}
}


void TIM3_IRQHandler(void)   //TIM3中断
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) //检查指定的TIM中断发生与否:TIM 中断源 
		{
		    TIM_ClearITPendingBit(TIM3, TIM_IT_Update  );  //清除TIMx的中断待处理位:TIM 中断源 
            irq_fun[ TIM_3 ].irq_handler(irq_fun[ TIM_3 ].arg);
		}
}

void TIM4_IRQHandler(void)   //TIM3中断
{
	if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET) //检查指定的TIM中断发生与否:TIM 中断源 
		{
		    TIM_ClearITPendingBit(TIM4, TIM_IT_Update  );  //清除TIMx的中断待处理位:TIM 中断源 
            irq_fun[ TIM_4 ].irq_handler(irq_fun[ TIM_4 ].arg);
		}
}

