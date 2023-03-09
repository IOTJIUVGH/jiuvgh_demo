#include <stdint.h>
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "misc.h"
#include "jerr.h"
#include "uart.h"
#include "peripheral_remap.h"
#include "stm32f10x_gpio.h"
#include "jtimer.h"

#define GPIO_PIN_MAX      16


typedef struct 
{
   uint16_t uart1_rx_total_cnt;
   uint8_t*  uart1_rx_buf;
   uint16_t uart1_rx_max;
} uart1_data_t;

static uint16_t uart1_rx_cnt = 0;

static uart1_data_t uart1_data ={ 0 };

static  USART_TypeDef *
    uart_port_remap[] =
        {
            USART1,
            USART2,
            USART3
};

static const uint32_t uart_rcc_remap[] =
    {
       RCC_APB2Periph_USART1,
       RCC_APB1Periph_USART2,
       RCC_APB1Periph_USART3
};

static const uint32_t uart_irqn[] =
    {
        USART1_IRQn,
        USART2_IRQn,
        USART3_IRQn};


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

static const mhal_uart_config_t default_config = {
    .data_width = USART_WordLength_8b,
    .parity = USART_Parity_No,
    .stop_bits = USART_StopBits_1,
    .flow_control = USART_HardwareFlowControl_None,
};

merr_t mhal_uart_open(int uart, const mhal_uart_config_t *config, mhal_uart_pinmux_t *pinmux)
{
    GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd(uart_rcc_remap[ uart ]|RCC_APB2Periph_GPIOA, ENABLE);
  
    GPIO_InitStructure.GPIO_Pin = gpio_pin_remap [ pinmux->tx / GPIO_PIN_MAX ] ; 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = gpio_pin_remap [pinmux->rx / GPIO_PIN_MAX ] ;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = uart_irqn[ uart ];
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=15 ;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			
	NVIC_Init(&NVIC_InitStructure);	

    uart1_data.uart1_rx_max = config ->buffersize;
	USART_InitStructure.USART_BaudRate = config->baudrate;
	USART_InitStructure.USART_WordLength = config->data_width;
	USART_InitStructure.USART_StopBits = config->stop_bits;
	USART_InitStructure.USART_Parity = config->parity;
	USART_InitStructure.USART_HardwareFlowControl = config->flow_control;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    USART_Init(uart_port_remap [uart], &USART_InitStructure);
    USART_ITConfig(uart_port_remap [uart], USART_IT_RXNE, ENABLE);
    USART_Cmd(uart_port_remap [uart], ENABLE);                    

    return kNoErr;
}

merr_t mhal_stdio_uart_open(uint32_t baudrate)
{
    return kNoErr;
}

merr_t mhal_uart_close(int uart)
{
    USART_DeInit(uart_port_remap [uart]);

    return kNoErr;
}

merr_t mhal_uart_write(int uart, const void *data, uint32_t size)
{
    for( uint16_t i = 0; i < size; i++ )
    {
        while((uart_port_remap [uart]->SR & 0x80)==0x00);
        uart_port_remap [uart]->DR=*(data+i);
    }

    return kNoErr;
}

merr_t mhal_uart_read(int uart, void *data, uint32_t *size, uint32_t timeout)
{
    merr_t err = kParamErr;

    if(uart_port_remap [uart] !=  USART1)
        return kParamErr;
   
    if( data != NULL)
        uart1_data.uart1_rx_buf = data;


    *size = uart1_data.uart1_rx_total_cnt;

    return kNoErr;
}

static uart1_rxtimeout()
{
    uart1_data.uart1_rx_total_cnt = uart1_rx_cnt;
	uart1_rx_cnt = 0;
}


void USART1_IRQHandler(void)                
{
    if (USART1->SR & 0x20)
    {
        if (uart1_data.uart1_rx_total_cnt == 0)
        {
            Vtimer_SetTimer(JTIM0, 3, uart1_rxtimeout);
            uart1_data.uart1_rx_buf[uart1_rx_cnt] = USART1->DR;
            if (uart1_rx_cnt < (uart1_data.uart1_rx_max - 1))
                uart1_rx_cnt++;
            else
                uart1_rx_cnt = 0;
        }
        else if (USART1->DR)
        {
        }
    }
} 


//////////////////////////////////////////////////////////////////
//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
_sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{      
	while((USART2->SR&0X40)==0);//循环发送,直到发送完毕   
    USART2->DR = (u8) ch;      
	return ch;
}
#endif 