/**
 ******************************************************************************
 * @file    rng.h
 * @author  snow yang
 * @version v1.0.0
 * @date    2019/06/21
 * @brief   This file provides mxchip RNG APIs.
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
* @defgroup rng RNG
* @brief RNG driver APIs
* @{
*/


#define PlatformRandomBytes mhal_rng_read   /**< funtion define */


/**@brief Fill in a memory buffer with random data
 *
 * @param inBuffer        : Point to a valid memory buffer, this function will fill 
                            in this memory with random numbers after executed
 * @param inByteCount     : Length of the memory buffer (bytes)
 *
 * @return    kNoErr        : on success.
 * @return    kGeneralErr   : if an error occurred with any step
 */
merr_t mhal_rng_read( void *inBuffer, int inByteCount );

/** @} */
/** @} */

