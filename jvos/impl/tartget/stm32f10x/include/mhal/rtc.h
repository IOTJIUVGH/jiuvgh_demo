/**
 ******************************************************************************
 * @file    rtc.h
 * @author  snow yang
 * @version v1.0.0
 * @date    2019/06/21
 * @brief   This file provides mxchip RTC APIs.
 ******************************************************************************
 */
#pragma once
     
#include "stdint.h"
#include "merr.h"

/** 
* @addtogroup hal
* @{
*/

/**
* @defgroup rtc RTC
* @brief RTC driver APIs
* @{
*/

/**
 * @brief Initialize the RTC peripheral
 *
 * @retval    kNoErr            on success.
 * @retval    kGeneralErr       if an error occurred with any step
 */
merr_t mhal_rtc_open(void);

/**
 * @brief Get time from the RTC peripheral
 *  
 * @param t The RTC time to be get
 *
 * @retval    kNoErr            on success.
 * @retval    kGeneralErr       if an error occurred with any step
 */
merr_t mhal_rtc_get_time(time_t *t);

/**
 * @brief Set the current time to the RTC peripheral
 *  
 * @param t The RTC time to be set
 *
 * @retval    kNoErr            on success.
 * @retval    kGeneralErr       if an error occurred with any step
 */
merr_t mhal_rtc_set_time(time_t t);

/**
 * @brief De-initialises the RTC peripheral
 *
 * @retval    kNoErr            on success.
 * @retval    kGeneralErr       if an error occurred with any step
 */
merr_t mhal_rtc_close(void);

/** @} */
/** @} */
