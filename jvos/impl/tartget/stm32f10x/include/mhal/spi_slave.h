/**
 ******************************************************************************
 * @file    spi_slave.h
 * @author  snow yang
 * @version v1.0.0
 * @date    2019/07/24
 * @brief   This file provides mxchip SPI APIs.
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
* @defgroup spi SPI
* @brief SPI driver APIs
* @{
*/

/**
 * @brief SPI handle type
 */
typedef int8_t mhal_spi_t;

/**@brief Open the SPI interface for a given SPI device
 *
 * @note  Prepares a SPI hardware interface for communication as a slave
 *
 * @param   spi     the SPI device to be initialised
 * @param   bufsize rx buffer size in bytes
 *
 * @retval  kNoErr  on success.
 * @retval  others  if an error occurred with any step
 */
merr_t mhal_spi_slave_open(mhal_spi_t spi, uint32_t bufsize);

/**@brief Format the SPI interface for a given SPI device
 *
 * @param   spi     the SPI device to be initialised
 * @param   speed   speed of SPI in Hz
 * @param   mode    SPI mode
 *              @arg 0 : [Polarity,Phase]=[0,0]
 *              @arg 1 : [Polarity,Phase]=[0,1]
 *              @arg 2 : [Polarity,Phase]=[1,0]
 *              @arg 3 : [Polarity,Phase]=[1,1]
 * @param   bits    SPI bits
 *
 * @retval  kNoErr  on success.
 * @retval  others  if an error occurred with any step
 */
merr_t mhal_spi_slave_format(mhal_spi_t spi, uint8_t mode, uint8_t bits);

/**@brief Send data from a SPI device
 *
 * @param   spi     the SPI device to be initialised
 * @param   buf     ponter to the buffer of data to be sent
 * @param   len     length of data
 *
 * @retval  kNoErr  on success
 * @retval  others  if an error occurred with any step
 */
merr_t mhal_spi_slave_write(mhal_spi_t spi, uint8_t *buf, uint32_t len);

/**@brief Receive data from a SPI device
 *
 * @param   spi     the SPI device to be initialised
 * @param   buf     ponter to the buffer to store received data
 * @param   len     number of bytes to receive
 * @param   timeout timeout in ms
 *
 * @retval  kNoErr  on success
 * @retval  others  if an error occurred with any step
 */
merr_t mhal_spi_slave_read(mhal_spi_t spi, uint8_t *buf, uint32_t *len, uint32_t timeout);

/**@brief Close a SPI interface
 *
 * @note Turns off a SPI hardware interface
 *
 * @param   spi     the SPI device to be de-initialised
 *
 * @retval  kNoErr  on success.
 * @retval  others  if an error occurred with any step
 */
merr_t mhal_spi_slave_close(mhal_spi_t spi);

/** @} */
/** @} */
