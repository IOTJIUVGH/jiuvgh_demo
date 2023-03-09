/**
 ******************************************************************************
 * @file    pwm.h
 * @author  snow yang
 * @version v1.0.0
 * @date    2019/06/21
 * @brief   This file provides mxchip PWM APIs.
 ******************************************************************************
 */
#ifndef __PWM_H__
#define __PWM_H__

#include "stdint.h"
#include "merr.h"

/** 
* @addtogroup hal
* @{
*/

/**
* @defgroup pwm PWM
* @brief PWM driver APIs
* @{
*/

typedef float (*dutycycle_get_t)(void);

/**
 * @brief Open a PWM pin
 * 
 * @note  Prepares a Pulse-Width Modulation pin for use.
 * Does not start the PWM output (use @ref mhal_pwm_start).
 *
 * @param pwm         the PWM interface which should be initialised
 * @param frequency   Output signal frequency in Hertz
 * @param dutycycle  Duty cycle of signal as a floating-point percentage (0.0 to 100.0)
 *
 * @retval    kNoErr            on success.
 * @retval    kGeneralErr       if an error occurred with any step
 */
merr_t mhal_pwm_open(int pwm, int polarity, int pin);

/**
 * @brief Starts PWM output on a PWM interface
 *
 * @note  Starts Pulse-Width Modulation signal output on a PWM pin
 *
 * @param pwm         the PWM interface which should be started
 *
 * @retval    kNoErr            on success.
 * @retval    kGeneralErr       if an error occurred with any step
 */
merr_t mhal_pwm_set_freq(int pwm, uint32_t frequency);

/**
 * @brief Starts PWM output on a PWM interface
 *
 * @note  Starts Pulse-Width Modulation signal output on a PWM pin
 *
 * @param pwm         the PWM interface which should be started
 *
 * @retval    kNoErr            on success.
 * @retval    kGeneralErr       if an error occurred with any step
 */
merr_t mhal_pwm_set_duty(int pwm, float dutycycle);

/**
 * @brief Close a PWM pin
 *
 * @param pwm         the PWM interface which should be initialised
 *
 * @retval    kNoErr            on success.
 * @retval    kGeneralErr       if an error occurred with any step
 */
merr_t mhal_pwm_close(int pwm);

/**
 * @brief Open a PWM pin to play PCM data
 * 
 *
 * @param pwm         the PWM interface which should be initialised
 * @param sample_rate the output PCM samplerate, only support 16000 and 8000. 
 * @param cb          callback function to get the duty cycle of signal as a floating-point percentage (0.0 to 100.0)
 *
 * @retval    kNoErr            on success.
 * @retval    kGeneralErr       if an error occurred with any step
 */
merr_t mhal_pwm_audio_start(int pwm, int sample_rate, dutycycle_get_t cb);

/**
 * @brief Stops PWM audio output
 *
 * @note  Stops Pulse-Width Modulation signal output on a PWM pin
 *
 * @param pwm         the PWM interface which should be stopped
 *
 * @retval    kNoErr            on success.
 * @retval    kGeneralErr       if an error occurred with any step
 */
merr_t mhal_pwm_audio_stop(int pwm);

/** @} */
/** @} */
#endif
