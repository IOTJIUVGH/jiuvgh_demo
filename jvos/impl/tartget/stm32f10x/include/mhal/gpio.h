/**
 ******************************************************************************
 * @file    gpio.h
 * @author  snow yang
 * @version v1.0.0
 * @date    2019/06/21
 * @brief   This file provides mxchip GPIO APIs.
 ******************************************************************************
 */

#pragma once

#include "stdint.h"
#include "stdbool.h"
#include "jerr.h"

#ifdef __cplusplus
extern "C" {
#endif

/** 
* @addtogroup hal
* @{
*/

/**
* @defgroup gpio GPIO
* @brief GPIO driver APIs
* @{
*/

/**
 * @brief UART handle type
 */
typedef int8_t mhal_gpio_t;

/**
 * @brief Pin configuration
 */
typedef enum
{
    INPUT_PULL_UP,             /**< Input with an internal pull-up resistor - use with devices that actively drive the signal low - e.g. button connected to ground */
    INPUT_PULL_DOWN,           /**< Input with an internal pull-down resistor - use with devices that actively drive the signal high - e.g. button connected to a power rail */
    INPUT_HIGH_IMPEDANCE,      /**< Input - must always be driven, either actively or by an external pullup resistor */
    OUTPUT_PUSH_PULL,          /**< Output actively driven high and actively driven low - must not be connected to other active outputs - e.g. LED output */
    OUTPUT_OPEN_DRAIN_NO_PULL, /**< Output actively driven low but is high-impedance when set high - can be connected to other open-drain/open-collector outputs. Needs an external pull-up resistor */
    OUTPUT_OPEN_DRAIN_PULL_UP, /**< Output actively driven low and is pulled high with an internal resistor when set high - can be connected to other open-drain/open-collector outputs. */
} mhal_gpio_config_t;

/**
 * @brief GPIO interrupt trigger
 */
typedef enum
{
    IRQ_TRIGGER_RISING_EDGE  = 0x1, /**< Interrupt triggered at input signal's rising edge */
    IRQ_TRIGGER_FALLING_EDGE = 0x2, /**< Interrupt triggered at input signal's falling edge */
    IRQ_TRIGGER_BOTH_EDGES   = IRQ_TRIGGER_RISING_EDGE | IRQ_TRIGGER_FALLING_EDGE, /**< Interrupt triggered at input signal's falling edge or signal's rising edge*/
} mhal_gpio_irq_trigger_t;

/**
 * @brief GPIO interrupt callback handler
 */
typedef void (*mhal_gpio_irq_handler_t)( void* arg );

/**
 * @brief Open a GPIO pin
 * 
 * @note Prepares an I2C hardware interface for communication as a master
 * @param  gpio           the gpio pin which should be initialised
 * @param configuration  A structure containing the required
 *                        gpio configuration
 * 
 * @retval    kNoErr            on success.
 * @retval    kGeneralErr       if an error occurred with any step
 */
merr_t mhal_gpio_open( mhal_gpio_t gpio, mhal_gpio_config_t configuration );

/**
 * @brief Close a GPIO pin
 * 
 * @param  gpio           the gpio pin which should be deinitialised
 * 
 * @retval    kNoErr            on success.
 * @retval    kGeneralErr       if an error occurred with any step
 */
merr_t mhal_gpio_close( mhal_gpio_t gpio );

/**
 * @brief Sets an output GPIO pin high
 * 
 * @param  gpio           the gpio pin which should be set high
 * 
 * @retval    kNoErr            on success.
 * @retval    kGeneralErr       if an error occurred with any step
 */
merr_t mhal_gpio_high( mhal_gpio_t gpio );

/**
 * @brief Sets an output GPIO pin high
 * 
 * @param  gpio           the gpio pin which should be set low
 * 
 * @retval    kNoErr            on success.
 * @retval    kGeneralErr       if an error occurred with any step
 */
merr_t mhal_gpio_low( mhal_gpio_t gpio );

/**
 * @brief Trigger an output GPIO pin
 * 
 * @param  gpio           the gpio pin which should be toggle
 * 
 * @retval    kNoErr            on success.
 * @retval    kGeneralErr       if an error occurred with any step
 */
merr_t mhal_gpio_toggle( mhal_gpio_t gpio );



/**@brief Get the state of an input GPIO pin
 *
 * @note Get the state of an input GPIO pin. Using this function on a
 * gpio pin which is set to output mode will return an undefined value.
 *
 * @param gpio           the gpio pin which should be read
 *
 * @retval    true   if high
 * @retval    fasle  if low
 */
bool mhal_gpio_value( mhal_gpio_t gpio );


/**@brief Enables an interrupt trigger for an input GPIO pin
 *
 * @note Enables an interrupt trigger for an input GPIO pin.
 * Using this function on a gpio pin which is set to
 * output mode is undefined.
 *
 * @param gpio     the gpio pin which will provide the interrupt trigger
 * @param trigger  the type of trigger (rising/falling edge)
 * @param handler  a function pointer to the interrupt handler
 * @param arg      an argument that will be passed to the
 *                  interrupt handler
 *
 * @retval    kNoErr         on success.
 * @retval    kGeneralErr    if an error occurred with any step
 */
merr_t mhal_gpio_int_on( mhal_gpio_t gpio, mhal_gpio_irq_trigger_t trigger, mhal_gpio_irq_handler_t handler, void* arg );


/**@brief Disables an interrupt trigger for an input GPIO pin
 *
 * @note Disables an interrupt trigger for an input GPIO pin.
 * Using this function on a gpio pin which has not been set up
 * using mhal_gpio_input_irq_enable is undefined.
 *
 * @param gpio     the gpio pin which provided the interrupt trigger
 *
 * @retval    kNoErr         on success.
 * @retval    kGeneralErr    if an error occurred with any step
 */
merr_t mhal_gpio_int_off( mhal_gpio_t gpio );

/** 
 * @} 
 */
/** 
 * @} 
 */

#ifdef __cplusplus
} /*extern "C" */
#endif



