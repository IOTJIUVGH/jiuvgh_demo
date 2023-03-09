/**
 ******************************************************************************
 * @file    gtimer.h
 * @author  snow yang
 * @version v1.0.0
 * @date    2019/06/21
 * @brief   This file provides mxchip TIMER APIs.
 ******************************************************************************
 */
#pragma once

#include "stdint.h"
#include "jerr.h"


/** 
* @addtogroup hal
* @{
*/

/**
* @defgroup gtimer TIMER
* @brief TIMER driver APIs
* @{
*/

/**
 * @brief TIMER handle type
 */
typedef int8_t mhal_gtimer_t;

/**
 * @brief Timer mode
 */
typedef enum
{
 ONE_SHOT,
 PERIOIC,
} mhal_gtimer_mode_t;

/**
 * @brief timer interrupt callback handler
 */
typedef void (*mhal_gtimer_irq_callback_t)( void* arg );

/**
 * @brief Init a hardware timer
 * 
 * @param  timer the timer for which the hardware timer should be initialised
 * @param  mode  the mode for which the timer reload
 * @param  time  us
 * @param  function  the callback function which the timer
 * @param  arg  
 * 
 * @retval    kNoErr            on success.
 * @retval    kGeneralErr       if an error occurred with any step
 */
merr_t mhal_gtimer_open(mhal_gtimer_t timer,mhal_gtimer_mode_t mode, uint32_t time, mhal_gtimer_irq_callback_t function, void *arg);

/**
 * @brief Start a hardware timer
 * 
 * @param  timer the timer for which the hardware timer should be start
 * 
 * @retval    kNoErr            on success.
 * @retval    kGeneralErr       if an error occurred with any step
 */
merr_t mhal_gtimer_start(mhal_gtimer_t timer);

/**
 * @brief Stop a hardware timer
 * 
 * @param  timer the timer for which the hardware timer should be stop
 * 
 * @retval    kNoErr            on success.
 * @retval    kGeneralErr       if an error occurred with any step
 */
merr_t mhal_gtimer_stop(mhal_gtimer_t timer);

/**
 * @brief Close a hardware timer
 * 
 * @param  timer the timer for which the hardware timer should be closed
 * 
 * @retval    kNoErr            on success.
 * @retval    kGeneralErr       if an error occurred with any step
 */
merr_t mhal_gtimer_close(mhal_gtimer_t timer);

/** @} */
/** @} */

