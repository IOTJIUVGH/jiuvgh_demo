/**
 ******************************************************************************
 * @file    ringbuffer.h
 * @author  swyang
 * @version V1.0.0
 * @date    18-July-2017
 * @brief   Header file for ringbuffer.c
 ******************************************************************************
 *
 *  UNPUBLISHED PROPRIETARY SOURCE CODE
 *  Copyright (c) 2017 MXCHIP Inc.
 *
 *  The contents of this file may not be disclosed to third parties, copied or
 *  duplicated in any form, in whole or in part, without the prior written
 *  permission of MXCHIP Corporation.
 ******************************************************************************
 */

#ifndef __RingBufferUtils_h__
#define __RingBufferUtils_h__

#include <stdint.h>

/** @addtogroup MICO_Middleware_Interface
  * @{
  */

/** @defgroup MICO_RingBuffer MiCO Ring Buffer
  * @brief Provide APIs for Ring Buffer
  * @{
  */

typedef struct
{
  uint8_t *buffer;
  uint32_t size;
  volatile uint32_t head; /* Read from */
  volatile uint32_t tail; /* Write to */
} ringbuffer_t;

#ifndef MIN
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#endif /* ifndef MIN */

/**
 * @brief ?
 *
 * @param ringbuffer:   ?
 * @param      buffer:   ?
 * @param        size:   ?
 *
 * @return   kNoErr        : on success.
 * @return   kGeneralErr   : if an error occurred
 */
int ringbuffer_init(ringbuffer_t *ringbuffer, uint8_t *buffer, uint32_t size);

/**
 * @brief ?
 *
 * @param ringbuffer:   ?
 *
 * @return   kNoErr        : on success.
 * @return   kGeneralErr   : if an error occurred
 */
int ringbuffer_deinit(ringbuffer_t *ringbuffer);

/**
 * @brief ?
 *
 * @param ringbuffer:   ?
 *
 * @return   
 */
uint32_t ringbuffer_free_space(ringbuffer_t *ringbuffer);

/**
 * @brief ?
 *
 * @param ringbuffer:   ?
 *
 * @return   
 */
uint32_t ringbuffer_used_space(ringbuffer_t *ringbuffer);

/**
 * @brief ?
 *
 * @param ringbuffer:   ?
 * @param data:   ?
 * @param contiguous_bytes: ?
 *
 * @return   
 */
int ringbuffer_get_data(ringbuffer_t *ringbuffer, uint8_t **data, uint32_t *contiguous_bytes);

/**
 * @brief ?
 *
 * @param ringbuffer:      ?
 * @param bytes_consumed:   ?
 *
 * @return   
 */
int ringbuffer_consume(ringbuffer_t *ringbuffer, uint32_t bytes_consumed);

/**
 * @brief ?
 *
 * @param ringbuffer:   ?
 * @param data:   ?
 * @param data_length: ?
 *
 * @return   
 */
uint32_t ringbuffer_write(ringbuffer_t *ringbuffer, const uint8_t *data, uint32_t data_length);

uint8_t ringbuffer_is_full(ringbuffer_t *ringbuffer);

int ringbuffer_read(ringbuffer_t *ringbuffer, uint8_t *data, uint32_t data_length, uint32_t *number_of_bytes_read);

uint32_t ringbuffer_total_size(ringbuffer_t *ringbuffer);
/**
  * @}
  */

/**
  * @}
  */

#endif // __RingBufferUtils_h__
