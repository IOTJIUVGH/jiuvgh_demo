/**
 ******************************************************************************
 * @file    mfi_auth.h
 * @author  snow yang
 * @version v1.0.0
 * @date    2019/06/21
 * @brief   This file provides mxchip mfi_auth APIs.
 ******************************************************************************
 */

#ifndef __MXOSDRIVERMFIAUTH_H__
#define __MXOSDRIVERMFIAUTH_H__

#pragma once
#include "mxos_common.h"

/** 
 * @addtogroup hal
 * @{
 */

/**
 * @defgroup mfi_auth MFI_AUTH
 * @brief mfi_auth driver APIs
 * @{
 */


#define MxosMFiAuthInitialize mhal_mfi_auth_open              /**< funtion define */
#define MxosMFiAuthFinalize mhal_mfi_auth_close               /**< funtion define */
#define MxosMFiAuthCreateSignature mhal_mfi_auth_create_sign  /**< funtion define */
#define MxosMFiAuthCopyCertificate mhal_mfi_auth_copy_cert    /**< funtion define */

/** @brief     PlatformMFiAuthInitialize
 *
 *  @param   i2c              mhal_i2c_t context    
 *
 *  @retval    kNoErr         on success.
 *  @retval    kGeneralErr    if an error occurred with any step
 */
merr_t mhal_mfi_auth_open( mhal_i2c_t i2c );


/** @brief    PlatformMFiAuthFinalize 
 *
 *  @retval   none
 */
void mhal_mfi_auth_close( void );

/** @brief    PlatformMFiAuthCreateSignature
 *
 *  @param    inDigestPtr         Pointer to 20-byte SHA-1 digest.
 *  @param    inDigestLen         Number of bytes in the digest. Must be 20.
 *  @param    outSignaturePtr     Receives malloc()'d ptr to RSA signature. Caller must free() on success.
 *  @param    outSignatureLen     Receives number of bytes in RSA signature.  
 *
 *  @retval    kNoErr             on success.
 *  @retval    kGeneralErr        if an error occurred with any step
 */
merr_t mhal_mfi_auth_create_sign( const  void      *inDigestPtr,
                                            size_t     inDigestLen,
                                            uint8_t  **outSignaturePtr,
                                            size_t    *outSignatureLen );

/** @brief    PlatformMFiAuthCopyCertificate
 *
 *  @param    outCertificatePtr   Receives malloc()'d ptr to a DER-encoded PKCS#7 message containing the certificate.
                                    Caller must free() on success.
 *  @param    outCertificateLen   Number of bytes in the DER-encoded certificate. 
 *
 *  @retval    kNoErr             on success.
 *  @retval    kGeneralErr        if an error occurred with any step
 */
merr_t mhal_mfi_auth_copy_cert( uint8_t **outCertificatePtr, size_t *outCertificateLen );

/** @} */
/** @} */
#endif 


