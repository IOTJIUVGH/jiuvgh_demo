/**
 ******************************************************************************
 * @file    chipid_calc.c
 * @author  swyang
 * @version V1.0.0
 * @date    17-July-2017
 * @brief   This file provide the chip ID calculation for EMW3060.
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
#include <stdint.h>

enum
{
    MD5_BLOCK_SIZE = 64,
    MD5_DIGEST_SIZE = 16,
    MD5_PAD_SIZE = 56
};

typedef struct
{
    uint32_t buffLen; /* length in bytes          */
    uint32_t loLen;   /* length in bytes   */
    uint32_t hiLen;   /* length in bytes   */
    uint32_t buffer[MD5_BLOCK_SIZE / sizeof(uint32_t)];
    uint32_t digest[MD5_DIGEST_SIZE / sizeof(uint32_t)];
} Md5;

enum
{
    AES_ENC_TYPE = 1, /* cipher unique type */
    AES_ENCRYPTION = 0,
    AES_DECRYPTION = 1,
    AES_BLOCK_SIZE = 16
};

typedef struct
{
    uint32_t key[60];
    uint32_t rounds;
    uint32_t reg[AES_BLOCK_SIZE / sizeof(uint32_t)]; /* for CBC mode */
    uint32_t tmp[AES_BLOCK_SIZE / sizeof(uint32_t)]; /* same         */
    uint32_t left;                                   /* unsued bytes left from last call */
} Aes;

#define InitMd5 wc_InitMd5
#define Md5Update wc_Md5Update
#define Md5Final wc_Md5Final

#define AesSetKey wc_AesSetKey
#define AesCbcEncrypt wc_AesCbcEncrypt

void InitMd5(Md5 *md5);
void Md5Update(Md5 *md5, const uint8_t *data, uint32_t len);
void Md5Final(Md5 *md5, uint8_t *hash);

int AesSetKey(Aes *aes, const uint8_t *key, uint32_t len, const uint8_t *iv, int dir);
int AesCbcEncrypt(Aes *aes, uint8_t *out, const uint8_t *in, uint32_t sz);


void md5(uint8_t *in, uint32_t size, uint8_t *out)
{
    Md5 md5_ctx;
    InitMd5(&md5_ctx);
    Md5Update(&md5_ctx, in, size);
    Md5Final(&md5_ctx, out);
}

#define AES_SZ 16
uint8_t VALID_MAC[][3] = {{0x04 + 1, 0x78 + 2, 0x63 + 3},
                          {0x80 + 4, 0xA0 + 5, 0x36 + 6},
                          {0xB0 + 7, 0xF8 + 8, 0x93 + 9},
                          {0xC8 + 10, 0x93 + 11, 0x46 + 12},
                          {0xD0 + 13, 0xBA + 14, 0xE4 + 15}};

int chipid_calc(uint8_t mac[6], uint8_t chipid[8])
{
    int i;
    uint8_t cipher[AES_SZ];
    uint8_t macmd5[AES_SZ];
    Aes enc;
    
    for (i = 0; i < sizeof(VALID_MAC) / 3; i++)
    {
        if ((uint8_t)(mac[0] + i * 3 + 1) == VALID_MAC[i][0] &&
            (uint8_t)(mac[1] + i * 3 + 2) == VALID_MAC[i][1] &&
            (uint8_t)(mac[2] + i * 3 + 3) == VALID_MAC[i][2])
        {
            break;
        }
    }

    if (i >= sizeof(VALID_MAC) / 3)
    {
        return -1;
    }

    
    // input, key, iv = md5(mac)
    md5(mac, 6, macmd5);

    AesSetKey(&enc, macmd5, AES_SZ, macmd5, 0);
    AesCbcEncrypt(&enc, cipher, macmd5, AES_SZ);
    // pick the odd bytes as chipid
    for (int i = 0; i < AES_SZ; i += 2)
    {
        chipid[i / 2] = cipher[i];
    }

    return 0;
}