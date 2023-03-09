/**
 ******************************************************************************
 * @file    i2s.h
 * @author  snow yang
 * @version v1.0.0
 * @date    2019/06/21
 * @brief   This file provides mxchip I2S APIs.
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
 * @defgroup i2s I2S
 * @brief I2S driver APIs
 * @{
 */


/* Legacy definitions */
#define MxosIISInitialize mhal_i2s_open         /**< funtion define */
#define MxosIISTransfer mhal_i2s_send_and_recv  /**< funtion define */
#define MxosIISFinalize mhal_i2s_close          /**< funtion define */
#define MxosIISWrite mhal_i2s_write             /**< funtion define */
#define MxosIISRead mhal_i2s_read               /**< funtion define */

/**
 * @brief I2C handle type
 */
typedef int8_t mhal_i2s_t;   

/**
 * @brief I2S message
 */
typedef struct
{
    const void* tx_buffer;  /**< pointer to a tx buffer */
    void*       rx_buffer;  /**< pointer to a rx buffer */
    uint32_t    length;     /**< number of bytes to transmit */
} mhal_i2s_message_segment_t;

/**
 * @brief I2S dev
 */
typedef struct
{
    mhal_i2s_t   port;              /**< Platform I2S port that is connected to the target I2S device, - e.g. MXOS_I2S_1 */
    uint32_t     bits;              /**< 1=master mode; 0=slave mode */
    uint8_t      master_enable;     /**< 1=master mode; 0=slave mode */
    uint32_t     clk_freq;          /**< 1=master mode; 0=slave mode */
    bool         is_mono_channel;   /**< 1=master mode; 0=slave mode */
} mhal_i2s_dev_t;

/**@brief Initialises the iis interface for a given iis device
 *
 * @note  Prepares a iis hardware interface for communication as a master
 *
 * @param  iis : the iis device to be initialised
 *
 * @retval    kNoErr         on success.
 * @retval    kGeneralErr    if the iis device could not be initialised
 */
merr_t mhal_i2s_open( const mhal_i2s_dev_t* iis );


/**@brief Transmits and/or receives data from a IIS device
 *
 * @param  iis      : the IIS device to be initialised
 * @param  segments : a pointer to an array of segments
 * @param  number_of_segments : the number of segments to transfer
 *
 * @retval    kNoErr        : on success.
 * @retval    kGeneralErr   : if an error occurred
 */
merr_t mhal_i2s_send_and_recv( const mhal_i2s_dev_t* iis, const mhal_i2s_message_segment_t* segments, uint16_t number_of_segments );


/**@brief De-initialises a IIS interface
 *
 * @note Turns off a IIS hardware interface
 *
 * @param  iis : the IIS device to be de-initialised
 *
 * @retval    kNoErr        : on success.
 * @retval    kGeneralErr   : if an error occurred
 */
merr_t mhal_i2s_close( const mhal_i2s_dev_t* iis );

/**@brief Write through IIS interface
 *
 * @note Write through IIS hardware interface
 *
 * @param  iis : the IIS device to be de-initialised
 * @param  p_buf : buf to be transfer
 * @param  size: size of buf
 *
 * @retval    kNoErr        : on success.
 * @retval    kGeneralErr   : if an error occurred
 */
merr_t mhal_i2s_write( const mhal_i2s_dev_t* iis, uint8_t *p_buf, uint32_t size );

/**@brief Read through IIS interface
 *
 * @note Read through IIS hardware interface
 *
 * @param  iis : the IIS device to be de-initialised
 * @param  p_buf : buf to be receive
 * @param  size: size of buf
 *
 * @retval    kNoErr        : on success.
 * @retval    kGeneralErr   : if an error occurred
 */
merr_t mhal_i2s_read( const mhal_i2s_dev_t* iis, uint8_t *p_buf, uint32_t size );

/** @} */
/** @} */

