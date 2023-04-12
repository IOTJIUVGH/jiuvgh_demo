#include <stddef.h>
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_exti.h"
#include "misc.h"
#include "gpio.h"
#include "peripheral_remap.h"

#define GPIO_Pin_Max 16 

typedef struct
{
    uint8_t pin;
    uint8_t port;
} gpio_dev_t;

typedef struct 
{
   mhal_gpio_irq_handler_t irq_handler;
   uint32_t* arg;
} irq_fun_t;

static const uint8_t gpio_mode_remap[] =
    {
        GPIO_Mode_IPU,
        GPIO_Mode_IPD,
        GPIO_Mode_IN_FLOATING,
        0,
        GPIO_Mode_Out_OD, // 开漏
        GPIO_Mode_Out_PP,
};

static const uint32_t gpio_rcc_remap[] =
    {
       RCC_APB2Periph_GPIOA,
       RCC_APB2Periph_GPIOB,
       RCC_APB2Periph_GPIOC,
       RCC_APB2Periph_GPIOD,
       RCC_APB2Periph_GPIOE,
       RCC_APB2Periph_GPIOF,
       RCC_APB2Periph_GPIOG
};

static const uint32_t gpio_pin_remap[] =
    {
       GPIO_Pin_0,
       GPIO_Pin_1,
       GPIO_Pin_2,
       GPIO_Pin_3,
       GPIO_Pin_4,
       GPIO_Pin_5,
       GPIO_Pin_6,
       GPIO_Pin_7,
       GPIO_Pin_8,
       GPIO_Pin_9,
       GPIO_Pin_10,
       GPIO_Pin_11,
       GPIO_Pin_12,
       GPIO_Pin_13,
       GPIO_Pin_14,
       GPIO_Pin_15
};

static const uint8_t gpio_pin_source[] =
    {
        GPIO_PinSource0,
        GPIO_PinSource1,
        GPIO_PinSource2,
        GPIO_PinSource3,
        GPIO_PinSource4,
        GPIO_PinSource5,
        GPIO_PinSource6,
        GPIO_PinSource7,
        GPIO_PinSource8,
        GPIO_PinSource9,
        GPIO_PinSource10,
        GPIO_PinSource11,
        GPIO_PinSource12,
        GPIO_PinSource13,
        GPIO_PinSource14,
        GPIO_PinSource15};

static const uint8_t gpio_port_source[] =
    {
        GPIO_PortSourceGPIOA,
        GPIO_PortSourceGPIOB,
        GPIO_PortSourceGPIOC,
        GPIO_PortSourceGPIOD,
        GPIO_PortSourceGPIOE,
        GPIO_PortSourceGPIOF,
        GPIO_PortSourceGPIOG};

static const uint32_t gpio_exit_line[] =
    {
        EXTI_Line0,
        EXTI_Line1,
        EXTI_Line2,
        EXTI_Line3,
        EXTI_Line4,
        EXTI_Line5,
        EXTI_Line6,
        EXTI_Line7,
        EXTI_Line8,
        EXTI_Line9,
        EXTI_Line10,
        EXTI_Line11,
        EXTI_Line12,
        EXTI_Line13,
        EXTI_Line14,
        EXTI_Line15,
};

static const uint32_t gpio_exit_irqn[] =
    {
        EXTI0_IRQn,
        EXTI1_IRQn,
        EXTI2_IRQn,
        EXTI3_IRQn,
        EXTI4_IRQn,
        EXTI9_5_IRQn,
        EXTI15_10_IRQn};

enum
{
    exit0,
    exit1,
    exit2,
    exit3,
    exit4,
    exit9_5,
    exit15_10,
    exitmax
};

enum
{
    Pin_0,
    Pin_1,
    Pin_2,
    Pin_3,
    Pin_4,
    Pin_5,
    Pin_6,
    Pin_7,
    Pin_8,
    Pin_9,
    Pin_10,
    Pin_11,
    Pin_12,
    Pin_13,
    Pin_14,
    Pin_15,
    Pin_max
};

static irq_fun_t irq_fun[exitmax] = {0};

static const uint8_t gpio_exit_trigger[] =
{
  EXTI_Trigger_Rising ,
  EXTI_Trigger_Falling ,  
  EXTI_Trigger_Rising_Falling
};


static GPIO_TypeDef *
    gpio_port_remap[] =
        {
            GPIOA,
            GPIOB,
            GPIOC,
            GPIOD,
            GPIOE,
            GPIOF,
            GPIOG,
};

merr_t mhal_gpio_info(gpio_dev_t* gpio_dev,mhal_gpio_t gpio)
{
    if (gpio >= GPIO_NUM)
        return kParamErr;

    gpio_dev->port = gpio % GPIO_Pin_Max;
    gpio_dev->pin = gpio / GPIO_Pin_Max;

    return kNoErr;
}

merr_t mhal_gpio_open(mhal_gpio_t gpio, mhal_gpio_config_t config)
{
    if (gpio >= GPIO_NUM)
        return kParamErr;

    GPIO_InitTypeDef  GPIO_InitStructure;
    gpio_dev_t gpio_dev;

    mhal_gpio_info(&gpio_dev, gpio);
    RCC_APB2PeriphClockCmd(gpio_rcc_remap[gpio_dev.port], ENABLE);

    GPIO_InitStructure.GPIO_Pin = gpio_pin_remap[gpio_dev.pin];				
    GPIO_InitStructure.GPIO_Mode = gpio_mode_remap[config]; 		
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 
    GPIO_Init(gpio_port_remap[gpio_dev.port], &GPIO_InitStructure);					

    return kNoErr;
}

merr_t mhal_gpio_close(mhal_gpio_t gpio)
{
    if (gpio >= GPIO_NUM)
        return kParamErr;

    gpio_dev_t gpio_dev;

    mhal_gpio_info(&gpio_dev, gpio);

    GPIO_DeInit(gpio_port_remap[gpio_dev.port]);

    return kNoErr;
}

merr_t mhal_gpio_high(mhal_gpio_t gpio)
{
    if (gpio >= GPIO_NUM)
        return kParamErr;

    gpio_dev_t gpio_dev;

    mhal_gpio_info(&gpio_dev, gpio);

    GPIO_SetBits(gpio_port_remap[gpio_dev.port], gpio_pin_remap[gpio_dev.pin]);

    return kNoErr;
}

merr_t mhal_gpio_low(mhal_gpio_t gpio)
{
    if (gpio >= GPIO_NUM)
        return kParamErr;

    gpio_dev_t gpio_dev;

    mhal_gpio_info(&gpio_dev, gpio);

    GPIO_ResetBits(gpio_port_remap[gpio_dev.port], gpio_pin_remap[gpio_dev.pin]);

    return kNoErr;
}

merr_t mhal_gpio_toggle(mhal_gpio_t gpio)
{
    if (gpio >= GPIO_NUM)
        return kParamErr;

    gpio_dev_t gpio_dev;

    mhal_gpio_info(&gpio_dev, gpio);

    int rc = GPIO_ReadOutputDataBit(gpio_port_remap[gpio_dev.port], gpio_pin_remap[gpio_dev.pin]);

    if (rc == 0)
    {
         GPIO_SetBits(gpio_port_remap[gpio_dev.port], gpio_pin_remap[gpio_dev.pin]);
    }
    else
    {
        GPIO_ResetBits(gpio_port_remap[gpio_dev.port], gpio_pin_remap[gpio_dev.pin]);
    }

    return kNoErr;
}

bool mhal_gpio_value(mhal_gpio_t gpio)
{
    if (gpio >= GPIO_NUM)
        return kParamErr;

    gpio_dev_t gpio_dev;

    mhal_gpio_info(&gpio_dev, gpio);

    return GPIO_ReadOutputDataBit(gpio_port_remap[gpio_dev.port], gpio_pin_remap[gpio_dev.pin]);
}

merr_t mhal_gpio_int_on(mhal_gpio_t gpio, mhal_gpio_irq_trigger_t trigger, mhal_gpio_irq_handler_t handler, void *arg)
{
    if (gpio >= GPIO_NUM)
        return kParamErr;
    
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    gpio_dev_t gpio_dev;

    mhal_gpio_info(&gpio_dev, gpio);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);

    GPIO_EXTILineConfig(gpio_port_source[gpio_dev.port], gpio_pin_source[gpio_dev.pin]);

  	EXTI_InitStructure.EXTI_Line=gpio_exit_line[gpio_dev.pin];
  	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
  	EXTI_InitStructure.EXTI_Trigger = gpio_exit_trigger[trigger];
  	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  	EXTI_Init(&EXTI_InitStructure);

    if(gpio_dev.pin < Pin_5)
    {
        NVIC_InitStructure.NVIC_IRQChannel = gpio_exit_irqn[gpio_dev.pin];
        irq_fun[gpio_dev.pin].irq_handler = handler;
        irq_fun[gpio_dev.pin].arg  = (uint32_t *)arg;
    }    
    if((gpio_dev.pin > Pin_4) && ((gpio_dev.pin < Pin_10)))
    {
        NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
       
        irq_fun[exit9_5].irq_handler = handler;
        irq_fun[exit9_5].arg  = (uint32_t *)arg;
    }
       
    if(gpio_dev.pin > Pin_9)
    {
        NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
        irq_fun[exit15_10].irq_handler = handler;
        irq_fun[exit15_10].arg  = (uint32_t *)arg;
    }
       
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0f;	
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;					
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								
  	NVIC_Init(&NVIC_InitStructure); 

    return kNoErr;
}

merr_t mhal_gpio_int_off(mhal_gpio_t gpio)
{
    if (gpio >= GPIO_NUM)
        return kParamErr;

    EXTI_DeInit();

    return kNoErr;
}

void EXTI0_IRQHandler(void)
{
    if (irq_fun[exit0].irq_handler != NULL)
    {
        irq_fun[exit0].irq_handler(irq_fun[exit0].arg);
        EXTI_ClearITPendingBit(EXTI_Line0);
    }
}

void EXTI1_IRQHandler(void)
{

    if (irq_fun[exit1].irq_handler != NULL)
    {
        irq_fun[exit1].irq_handler(irq_fun[exit1].arg);
        EXTI_ClearITPendingBit(EXTI_Line1);
    }
}

void EXTI2_IRQHandler(void)
{

    if (irq_fun[exit2].irq_handler != NULL)
    {
        irq_fun[exit2].irq_handler(irq_fun[exit2].arg);
        EXTI_ClearITPendingBit(EXTI_Line2);
    }
}

void EXTI3_IRQHandler(void)
{
    if (irq_fun[exit3].irq_handler != NULL)
    {
        irq_fun[exit3].irq_handler(irq_fun[exit3].arg);
        EXTI_ClearITPendingBit(EXTI_Line3);
    }
}

void EXTI4_IRQHandler(void)
{

    if (irq_fun[exit4].irq_handler != NULL)
    {
        irq_fun[exit4].irq_handler(irq_fun[exit4].arg);
        EXTI_ClearITPendingBit(EXTI_Line4);
    }
}

void EXTI9_5_IRQHandler(void)
{
    if (irq_fun[exit9_5].irq_handler != NULL)
    {
        irq_fun[exit9_5].irq_handler(irq_fun[exit9_5].arg);
        EXTI_ClearITPendingBit(EXTI_Line5);
    }
}

void EXTI15_10_IRQHandler(void)
{
    if (irq_fun[exit15_10].irq_handler != NULL)
    {
        irq_fun[exit15_10].irq_handler(irq_fun[exit15_10].arg);
        EXTI_ClearITPendingBit(EXTI_Line15);
    }
}
