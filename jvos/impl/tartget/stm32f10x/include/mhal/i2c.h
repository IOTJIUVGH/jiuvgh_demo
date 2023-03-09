/**
 ******************************************************************************
 * @file    i2c.h
 * @author  snow yang
 * @version v1.0.0
 * @date    2019/06/21
 * @brief   This file provides mxchip I2C APIs.
 ******************************************************************************
 */
#ifndef __I2C_H__
#define __I2C_H__

#include "stdint.h"
#include "merr.h"

/** 
 * @addtogroup hal
 * @{
 */

/**
 * @defgroup i2c I2C
 * @brief I2C driver APIs
 * @{
 */

/**
 * @brief I2C address width
 */
typedef enum
{
    I2C_ADDR_WIDTH_7BIT,  /**< 7  bit address width */
    I2C_ADDR_WIDTH_10BIT, /**< 10 bit address width */
    I2C_ADDR_WIDTH_16BIT  /**< 16 bit address width */
} mhal_i2c_addr_width_t;

typedef struct
{
    int sda; /**< MISO pin */
    int scl; /**< Clock pin */
} mhal_i2c_pinmux_t;

/**
 * @brief Open an I2C interface
 * 
 * @note Prepares an I2C hardware interface for communication as a master
 * @param  device  the device for which the i2c should be initialised
 * 
 * @retval    kNoErr            on success.
 * @retval    kGeneralErr       if an error occurred with any step
 */
merr_t mhal_i2c_open(int i2c, mhal_i2c_addr_width_t addrwidth, int frequency, mhal_i2c_pinmux_t *pinmux);

merr_t mhal_i2c_write(int i2c, uint8_t addr, uint8_t *buf, uint32_t n);

merr_t mhal_i2c_read(int i2c, uint8_t addr, uint8_t *buf, uint32_t n);

/**
 * @brief Close an I2C device
 * 
 * @param  device  the device for which the i2c i2c should be deinitialised
 * 
 * @retval    kNoErr            on success.
 * @retval    kGeneralErr       if an error occurred with any step
 */
merr_t mhal_i2c_close(int i2c);
/** 
 * @} 
 */
/** 
 * @} 
 */

#endif
