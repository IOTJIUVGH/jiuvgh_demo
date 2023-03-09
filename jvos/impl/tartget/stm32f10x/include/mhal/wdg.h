/**
 ******************************************************************************
 * @file    wdg.h
 * @author  snow yang
 * @version v1.0.0
 * @date    2019/06/21
 * @brief   This file provides mxchip WDG APIs.
 ******************************************************************************
 */
#pragma once
     
#include <stdio.h>
#include "jerr.h"

/** 
 * @addtogroup hal
 * @{
 */

/**
 * @defgroup wag WAG
 * @brief WAG driver APIs
 * @{
 */

/**
 * @brief This function will initialize the on board CPU hardware watch dog
 *
 * @param timeout         Watchdag timeout, application should call mhal_wdg_feed befor timeout.
 *
 * @retval    kNoErr         on success.
 * @retval    kGeneralErr    if an error occurred with any step
 */
merr_t mhal_wdg_open( uint32_t timeout );

/**
 * @brief Reload watchdog counter.
 *
 * 
 * @retval    none
 */
void mhal_wdg_feed( void );

/**
 * @brief This function performs any platform-specific cleanup needed for hardware watch dog
 *
 *
 * @retval    kNoErr         on success.
 * @retval    kGeneralErr    if an error occurred with any step
 */
merr_t mhal_wdg_close( void );

/** @} */
/** @} */

