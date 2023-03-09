/**
 ******************************************************************************
 * @file    adc.h
 * @author  snow yang
 * @version v1.0.0
 * @date    2019/06/21
 * @brief   This file provides mxchip ADC APIs.
 ******************************************************************************
 */

#ifndef __ADC_H__
#define __ADC_H__

#include "stdint.h"
#include "merr.h"

/** 
* @addtogroup hal
* @{
*/

/**
* @defgroup adc ADC
* @brief ADC driver APIs
* @{
*/

/**@brief Initialises an ADC interface
 *
 * Prepares an ADC hardware interface for sampling
 *
 * @param adc            : the interface which should be initialised
 * @param sampling_cycle : sampling period in number of ADC clock cycles. If the
 *                         MCU does not support the value provided, the closest
 *                         supported value is used.
 *
 * @retval    kNoErr        : on success.
 * @retval    kGeneralErr   : if an error occurred with any step
 */
merr_t mhal_adc_open(int adc, int pin);

/**@brief Takes a single sample from an ADC interface
 *
 * Takes a single sample from an ADC interface
 *
 * @param adc    : the interface which should be sampled
 * @param output : pointer to a variable which will receive the sample
 *
 * @retval    kNoErr        : on success.
 * @retval    kGeneralErr   : if an error occurred with any step
 */
float mhal_adc_read(int adc);

/**@brief     De-initialises an ADC interface
 *
 * @param  adc : the interface which should be de-initialised
 *
 * @retval    kNoErr        : on success.
 * @retval    kGeneralErr   : if an error occurred with any step
 */
merr_t mhal_adc_close(int adc);

/** @} */
/** @} */

#endif
