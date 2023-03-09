/**
 ******************************************************************************
 * @file    mtls.h
 * @author  snow yang
 * @version v1.0.0
 * @date    2019/06/18
 * @brief   This file provides mxchip TLS APIs.
 ******************************************************************************
 */

#ifndef __MTLS_H__
#define __MTLS_H__

#include <stdint.h>

/** 
 * @addtogroup tls
 * @{
 */

/**
 * @brief mxchip TLS handler type
 */
typedef void *mtls_t;

enum
{
  SSL_V3_MODE = 1,   /**< SSL V3 */
  TLS_V1_0_MODE = 2, /**< TLS V1.0 */
  TLS_V1_1_MODE = 3, /**< TLS V1.1 */
  TLS_V1_2_MODE = 4, /**< TLS V1.2 */
};
/**
 * @brief mxchip TLS version
 */
typedef uint8_t mtls_ver_t;

/** 
 * @defgroup tls_socket Socket
 * @brief TLS socket functions
 * @{
 */

/** 
 * @brief      Used to set the TLS protocol version for both TLS client and TLS server 
 * @note       This function should be called before TLS is ready to function (before
 *              mtls_connect and mtls_accept is called by application ).
 * @param      version TLS protocol version.
 */
void mtls_set_ver(mtls_ver_t version);

/** 
 * @brief      Get the internal socket fire description
 * @note       This function should be called after TLS connection is established.
 * @param      tls TLS handler.
 *
 * @return     File descriptor for the TLS connection.
 */
int mtls_socket(mtls_t tls);

/** 
 * @brief      Used by the TLS server. Set the certificate and private key for a TLS server. 
 * @details    This function is called on the server side to set it's certifact and private key.
 *             It must be called before mtls_accept. These two arguments must be global
 *             string buffer, library will not create a copy for them.
 * @param      cert_pem Point to the certificate string in PEM format.
 * @param      private_key_pem Point to the private key string in PEM format.
 * @param      verify_ca verify client certificate.
 */
void mtls_set_cert(const char *cert_pem, const char *private_key_pem, const char *verify_ca);

/** 
 * @brief      TLS client create a TLS connection.
 * @details    This function is called on the client side and initiates an TLS/TLS handshake with a 
 *              server.  When this function is called, the underlying communication channel has already 
 *              been set up. This function is called after TCP 3-way handshak finished. 
 * @param      fd The fd number for a connected TCP socket.
 * @param      calen the string length of ca. 0=do not verify server's certificate.
 * @param      ca pointer to the CA certificate string, used to verify server's certificate.
 * @param      errno return the errno if mtls_connect return NULL.

 * @return     return the TLS context pointer on success or NULL for fail.
 */
mtls_t mtls_connect(int fd, int calen, char *ca, int *errno);

/** 
 * @brief      TLS client create a TLS connection.
 *
 * @details    This function is called on the client side and initiates an TLS/TLS handshake with a 
 *              server.  When this function is called, the underlying communication channel has already 
 *              been set up. This function is called after TCP 3-way handshak finished. 
 *
 * @param      fd The fd number for a connected TCP socket.
 * @param      calen the string length of ca. 0=do not verify server's certificate.
 * @param      ca pointer to the CA certificate string, used to verify server's certificate.
 * @param      sni_servername sni server name.
 * @param      errno return the errno if mtls_connect return NULL.
 *
 * @return     return the TLS context pointer on success or NULL for fail.
 */
mtls_t mtls_connect_sni(int fd, int calen, char *ca, char *sni_servername, int *errno);

/** 
 * @brief      TLS Server accept a TLS connection
 * @param      fd The fd number for a connected TCP socket.
 * 
 * @return     return the TLS context pointer on success or NULL for fail.
 */
mtls_t mtls_accept(int fd);

/** 
 * @brief      TLS send data
 * @param      tls Point to the TLS context.
 * @param      data Point to send data.
 * @param      len data length.
 *
 * @return     On success, these calls return the number of bytes sent.  On error,
 *             -1 is returned,
 */
int mtls_send(mtls_t tls, void *data, size_t len);

/** 
 * @brief      TLS receive data
 * @param      tls Point to the TLS context.
 * @param      data Point to buffer to receive TLS data.
 * @param      len max receive buffer length.
 *
 * @return     On success, these calls return the number of bytes received.  On error,
 *             -1 is returned,
 */
int mtls_recv(mtls_t tls, void *data, size_t len);

/** 
 * @brief      Close the TLS session, release resource.
 * @param      tls Point to the TLS context.
 *
 * @return     kNoerr or kGeneralErr
 */
int mtls_close(mtls_t tls);
/** 
 * @}
 */

/** 
 * @defgroup tls_utils Utils
 * @brief TLS utils functions
 * @{
 */

/** 
 * @brief      Get the size of data that is pending for read
 * @param      tls Point to the TLS context.
 *
 * @return     size of data that is pending for read 
 */
int mtls_pending(mtls_t tls);

/** 
 * @brief      Get last error code
 * @param      tls Point to the TLS context.
 * @param      ret return code
 *
 * @return     error code
 */
int mtls_get_error(mtls_t tls, int ret);

/** 
 * @brief      Set TLS nonblock mode
 * @param      tls Point to the TLS context.
 * @param      nonblock true - nonblock, flase - block
 */
void mtls_set_set_nonblock(mtls_t tls, int nonblock);
/** 
 * @} 
 */
/** 
 * @} 
 */

#endif