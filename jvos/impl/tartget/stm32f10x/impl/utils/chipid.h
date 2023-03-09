/*
 * Copyright (C) 2017-2019 MXCHIP
 */

#ifndef _CHIPID_H_
#define _CHIPID_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * generate chip id with mac address.
 *
 * @param[in]   mac     mac address, 6 bytes.
 * @param[out]  chipid  pointer to of chip id storage, 8 bytes.
 *
 * @return  0: success, -1: invalid mac address.
 */
int chipid_calc(uint8_t mac[6], uint8_t chipid[8]);

#ifdef __cplusplus
}
#endif

#endif /* _CHIPID_H_ */