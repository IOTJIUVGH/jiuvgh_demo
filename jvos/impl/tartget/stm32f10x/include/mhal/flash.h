/**
 ******************************************************************************
 * @file    flash.h
 * @author  snow yang
 * @version v1.0.0
 * @date    2019/06/21
 * @brief   This file provides mxchip FLASH APIs.
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
* @defgroup flash FLASH
* @brief FLASH driver APIs
* @{
*/

#define PAR_OPT_READ_POS (0)  /**< partition read pos */
#define PAR_OPT_WRITE_POS (1) /**< partition write pos */

#define PAR_OPT_READ_MASK (0x1u << PAR_OPT_READ_POS)   /**< partition read mask */
#define PAR_OPT_WRITE_MASK (0x1u << PAR_OPT_WRITE_POS) /**< partition write mask */

#define PAR_OPT_READ_DIS (0x0u << PAR_OPT_READ_POS)   /**< partition read disable */
#define PAR_OPT_READ_EN (0x1u << PAR_OPT_READ_POS)    /**< partition read enable */
#define PAR_OPT_WRITE_DIS (0x0u << PAR_OPT_WRITE_POS) /**< partition write disable  */
#define PAR_OPT_WRITE_EN (0x1u << PAR_OPT_WRITE_POS)  /**< partition write enable */

/**
 * @brief FLASH handle type
 */
typedef int8_t mhal_flash_part_name_t;

/**
 * @brief flash mhal_flash_part_t
 */
typedef struct
{
    int32_t owner;    /**< partition owner */
    const char *desc; /**< partition description */
    uint32_t addr;    /**< partition start_addr */
    uint32_t len;     /**< partition length */
    uint32_t rw;      /**< partition options */
} mhal_flash_part_t;

/**
 * @brief   Get the infomation of the specified flash area 
 * 
 * @param   name  The target flash logical partition which should be erased
 * 
 * @retval  mhal_flash_part_t  struct
 */
mhal_flash_part_t *mhal_flash_get_info(mhal_flash_part_name_t name);

/**@brief   Erase an area on a Flash logical partition
 *
 * @note    Erase on an address will erase all data on a sector that the 
 *          address is belonged to, this function does not save data that
 *          beyond the address area but in the affected sector, the data
 *          will be lost.
 *
 * @param  name      The target flash logical partition which should be erased
 * @param  offset          Start address of the erased flash area
 * @param  len    	    Size of the erased flash area
 *
 * @retval  kNoErr         On success.
 * @retval  kGeneralErr    If an error occurred with any step
 */
merr_t mhal_flash_erase(mhal_flash_part_name_t name, uint32_t offset, uint32_t len);

/**@brief  Write data to an area on a Flash logical partition
 *
 * @param  name     The target flash logical partition which should be read which should be written
 * @param  offset         Point to the start address that the data is written to, and
 *                          point to the last unwritten address after this function is 
 *                          returned, so you can call this function serval times without
 *                          update this start address.
 * @param  buffer        point to the data buffer that will be written to flash
 * @param  len  The length of the buffer
 *
 * @retval  kNoErr         On success.
 * @retval  kGeneralErr    If an error occurred with any step
 */
merr_t mhal_flash_write(mhal_flash_part_name_t name, uint32_t *offset, uint8_t *buffer, uint32_t len);

/**@brief    Read data from an area on a Flash to data buffer in RAM
 *
 * @param    name     The target flash logical partition which should be read
 * @param    offset         Point to the start address that the data is read, and
 *                          point to the last unread address after this function is 
 *                          returned, so you can call this function serval times without
 *                          update this start address.
 * @param    buffer       Point to the data buffer that stores the data read from flash
 * @param    len  The length of the buffer
 *
 * @retval    kNoErr         On success.
 * @retval    kGeneralErr    If an error occurred with any step
 */
merr_t mhal_flash_read(mhal_flash_part_name_t name, uint32_t *offset, uint8_t *buffer, uint32_t len);

/** @} */
/** @} */
