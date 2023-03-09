/**
 ******************************************************************************
 * @file    ringbuffer.c
 * @author  swyang
 * @version V1.0.0
 * @date    18-July-2017
 * @brief   This file provide the ringbufferfer functions.
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

#include <string.h>
#include "ringbuffer.h"

int ringbuffer_init(ringbuffer_t *ringbuffer, uint8_t *buffer, uint32_t size)
{
  if (ringbuffer)
  {
    ringbuffer->buffer = (uint8_t *)buffer;
    ringbuffer->size = size;
    ringbuffer->head = 0;
    ringbuffer->tail = 0;
    return 0;
  }
  else
    return -1;
}

int ringbuffer_deinit(ringbuffer_t *ringbuffer)
{
  return 0;
}

uint32_t ringbuffer_free_space(ringbuffer_t *ringbuffer)
{
  uint32_t tail_to_end = ringbuffer->size - 1 - ringbuffer->tail;
  return ((tail_to_end + ringbuffer->head) % ringbuffer->size);
}

uint32_t ringbuffer_used_space(ringbuffer_t *ringbuffer)
{
  uint32_t head_to_end = ringbuffer->size - ringbuffer->head;
  return ((head_to_end + ringbuffer->tail) % ringbuffer->size);
}

int ringbuffer_get_data(ringbuffer_t *ringbuffer, uint8_t **data, uint32_t *contiguous_bytes)
{
  uint32_t head_to_end = ringbuffer->size - ringbuffer->head;

  *data = &ringbuffer->buffer[ringbuffer->head];
  *contiguous_bytes = MIN(head_to_end, (head_to_end + ringbuffer->tail) % ringbuffer->size);

  return 0;
}

int ringbuffer_consume(ringbuffer_t *ringbuffer, uint32_t bytes_consumed)
{
  ringbuffer->head = (ringbuffer->head + bytes_consumed) % ringbuffer->size;
  return 0;
}

uint32_t ringbuffer_write(ringbuffer_t *ringbuffer, const uint8_t *data, uint32_t data_length)
{
  uint32_t tail_to_end = ringbuffer->size - 1 - ringbuffer->tail;

  /* Calculate the maximum amount we can copy */
  uint32_t amount_to_copy = MIN(data_length, (tail_to_end + ringbuffer->head) % ringbuffer->size);

  /* Fix the bug when tail is at the end of buffer */
  tail_to_end++;

  /* Copy as much as we can until we fall off the end of the buffer */
  memcpy(&ringbuffer->buffer[ringbuffer->tail], data, MIN(amount_to_copy, tail_to_end));

  /* Check if we have more to copy to the front of the buffer */
  if (tail_to_end < amount_to_copy)
  {
    memcpy(&ringbuffer->buffer[0], data + tail_to_end, amount_to_copy - tail_to_end);
  }

  /* Update the tail */
  ringbuffer->tail = (ringbuffer->tail + amount_to_copy) % ringbuffer->size;

  return amount_to_copy;
}

int ringbuffer_read(ringbuffer_t *ringbuffer, uint8_t *data, uint32_t data_length, uint32_t *number_of_bytes_read)
{
  uint32_t max_bytes_to_read;
  uint32_t i;
  uint32_t head;
  uint32_t used_bytes;

  head = ringbuffer->head;

  used_bytes = ringbuffer_used_space(ringbuffer);

  max_bytes_to_read = MIN(data_length, used_bytes);

  if (max_bytes_to_read != 0)
  {
    for (i = 0; i != max_bytes_to_read; i++, (head = (head + 1) % ringbuffer->size))
    {
      data[i] = ringbuffer->buffer[head];
    }

    ringbuffer_consume(ringbuffer, max_bytes_to_read);
  }

  *number_of_bytes_read = max_bytes_to_read;

  return 0;
}

uint8_t ringbuffer_is_full(ringbuffer_t *ringbuffer)
{
  if (ringbuffer_used_space(ringbuffer) >= ringbuffer->size - 1)
    return 1;
  else
    return 0;
}

uint32_t ringbuffer_total_size(ringbuffer_t *ringbuffer)
{
  return ringbuffer->size - 1;
}
