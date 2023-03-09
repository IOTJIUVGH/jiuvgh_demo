/******************************************************************************
  *
  * This module is a confidential and proprietary property of RealTek and
  * possession or use of this module requires written permission of RealTek.
  *
  * Copyright(c) 2016, Realtek Semiconductor Corporation. All rights reserved. 
  *
******************************************************************************/

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h" 

#include "main.h"
#include "main_test.h"
#if CONFIG_WLAN
#include "wifi_conf.h"
#include "wlan_intf.h"
#include "wifi_constants.h"
#endif
#include "lwip_netconf.h"
#include <platform/platform_stdlib.h>
#include "osdep_service.h"

#ifndef CONFIG_INIT_NET
#define CONFIG_INIT_NET             1
#endif
#ifndef CONFIG_INTERACTIVE_MODE
#define CONFIG_INTERACTIVE_MODE     1
#endif

#define STACKSIZE                   (512 + 768)

xSemaphoreHandle uart_rx_interrupt_sema = NULL;

#if CONFIG_MXCHIP
static int _wifi_up = 0;
static uint8_t wifi_mac[6];

void wifi_mac_get(uint8_t *mac)
{
    memcpy(mac, wifi_mac, 6);
}


static void macstr2mac(char *str, unsigned char *mac)
{
    int i;
    unsigned char hi, low;

    for(i=0;i<6;i++) {
        hi = str[3*i];
        low = str[3*i + 1];
        if (hi >= '0' && hi <= '9')
            hi -= '0';
        else if (hi >= 'a' && hi <= 'f')
            hi = hi - 'a' + 10;
        else 
            hi = hi - 'A' + 10;
        hi = (hi&0x0f) << 4;
        if (low >= '0' && low <= '9')
            low -= '0';
        else if (low >= 'a' && low <= 'f')
            low = low - 'a' + 10;
        else 
            low = low - 'A' + 10;
        low = low&0x0f;
        
        mac[i] = hi + low;
    }
}

#endif

void init_thread(void *param)
{
	/* To avoid gcc warnings */
	( void ) param;
#if CONFIG_INIT_NET
#if CONFIG_LWIP_LAYER
	/* Initilaize the LwIP stack */
	LwIP_Init();
#endif
#endif
#if CONFIG_WIFI_IND_USE_THREAD
	wifi_manager_init();
#endif
#if CONFIG_WLAN
	wifi_on(RTW_MODE_STA);
#if CONFIG_AUTO_RECONNECT
	//setup reconnection flag
	wifi_set_autoreconnect(1);
#endif
	printf("\n\r%s(%d), Available heap 0x%x", __FUNCTION__, __LINE__, xPortGetFreeHeapSize());	
#endif

#if CONFIG_INTERACTIVE_MODE
 	/* Initial uart rx swmaphore*/
	vSemaphoreCreateBinary(uart_rx_interrupt_sema);
	xSemaphoreTake(uart_rx_interrupt_sema, 1/portTICK_RATE_MS);
	start_interactive_mode();
#endif	

#if CONFIG_MXCHIP
    char mac[18];
    wifi_get_mac_address(mac);
    macstr2mac(mac, wifi_mac);
    _wifi_up = 1;
#endif

	/* Kill init thread after all init tasks done */
	vTaskDelete(NULL);
}

void wlan_network()
{
	if(xTaskCreate(init_thread, ((const char*)"init"), STACKSIZE, NULL, tskIDLE_PRIORITY + 3 + PRIORITIE_OFFSET, NULL) != pdPASS)
		printf("\n\r%s xTaskCreate(init_thread) failed", __FUNCTION__);
#if CONFIG_MXCHIP
    while(_wifi_up == 0) {
        mos_msleep(10);
    }
#endif
}
