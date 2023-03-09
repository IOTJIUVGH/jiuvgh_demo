/**
 ******************************************************************************
 * @file    uart.h
 * @author  snow yang
 * @version v1.0.0
 * @date    2019/06/21
 * @brief   This file provides mxchip UART APIs.
 ******************************************************************************
 */

#ifndef __UART_H__
#define __UART_H__

#include <stdio.h>
#include "jerr.h"

/** 
* @addtogroup hal
* @{
*/

/**
* @defgroup uart UART
* @brief UART driver APIs
* @{
*/

enum
{
    DATA_WIDTH_5BIT, /**< 5 bit data width */
    DATA_WIDTH_6BIT, /**< 6 bit data width */
    DATA_WIDTH_7BIT, /**< 7 bit data width */
    DATA_WIDTH_8BIT, /**< 8 bit data width */
    DATA_WIDTH_9BIT  /**< 9 bit data width */
};
/**
 * @brief UART data width
 */
typedef uint8_t mhal_uart_data_width_t;

enum
{
    STOP_BITS_1, /**< 1 stop bits */
    STOP_BITS_2, /**< 2 stop bits */
};
/**
 * @brief UART stop bits
 */
typedef uint8_t mhal_uart_stop_bits_t;

enum
{
    FLOW_CONTROL_DISABLED, /**< Flow control disabled */
    FLOW_CONTROL_CTS,      /**< Flow contrl CTS */
    FLOW_CONTROL_RTS,      /**< Flow contrl RTS */
    FLOW_CONTROL_CTS_RTS   /**< Flow contrl CTS and RTS */
};
/**
 * @brief UART flow control
 */
typedef uint8_t mhal_uart_flow_control_t;

enum
{
    NO_PARITY,   /**< No parity */
    ODD_PARITY,  /**< Odd parity */
    EVEN_PARITY, /**< Even parity */
};
/**
 * @brief UART parity
 */
typedef uint8_t mhal_uart_parity_t;

/**
 * @brief UART configuration
 */
typedef struct
{
    uint32_t baudrate;                     /**< Baudrate */
    mhal_uart_data_width_t data_width;     /**< Date width */
    mhal_uart_parity_t parity;             /**< Parity */
    mhal_uart_stop_bits_t stop_bits;       /**< Stop bits */
    mhal_uart_flow_control_t flow_control; /**< Flow control */
    uint32_t buffersize;                   /**< Receive buffer size */
} mhal_uart_config_t;

typedef struct
{
    int tx;  /**< TXD pin */
    int rx;  /**< TXD pin */
    int rts; /**< RTS pin */
    int cts; /**< CTS pin */
} mhal_uart_pinmux_t;

/**
 * @brief Callback function to receive an UART byte.
 * 
 * @note register this callback function at mhal_uart_open_irq_mode
 * @param  data       The received uart data
 * 
 */
typedef void (*uart_input_byte_t)(uint8_t data);

/**
 * @brief Open a UART interface
 * 
 * @note Prepares an UART hardware interface for communications
 * @param  uart       the interface which should be initialised
 * @param  baudrate   Baudrate
 * @param  rx_cb      The user callback function to receive a byte from hardware.
 * @param  config     UART configuration structure
 * 
 * @retval    kNoErr         on success.
 * @retval    kGeneralErr    if an error occurred with any step
 */
merr_t mhal_uart_open_irq_mode(int uart, uart_input_byte_t rx_cb, const mhal_uart_config_t *config, mhal_uart_pinmux_t *pinmux);

/**
 * @brief Open a UART interface
 * 
 * @note Prepares an UART hardware interface for communications
 * @param  uart       the interface which should be initialised
 * @param  baudrate   Baudrate
 * @param  buffersize Rx buffer size in bytes
 * @param  config     UART configuration structure
 * 
 * @retval    kNoErr         on success.
 * @retval    kGeneralErr    if an error occurred with any step
 */
merr_t mhal_uart_open(int uart, const mhal_uart_config_t *config, mhal_uart_pinmux_t *pinmux);

/**
 * @brief Initialises a STDIO UART interface, internal use only
 * @note Prepares an UART hardware interface for stdio communications
 *
 * @param  baudrate  Baudrate
 *
 * @retval    kNoErr         on success.
 * @retval    kGeneralErr    if an error occurred with any step
 */
merr_t mhal_stdio_uart_open(uint32_t baudrate);

/**
 * @brief Deinitialises a UART interface
 *
 * @param  uart  the interface which should be deinitialised
 *
 * @retval    kNoErr         on success.
 * @retval    kGeneralErr    if an error occurred with any step
 */
merr_t mhal_uart_close(int uart);

/**
 * @brief Transmit data on a UART interface
 *
 * @param  uart      the UART interface
 * @param  data      pointer to the start of data
 * @param  size      number of bytes to transmit
 *
 * @retval    kNoErr         on success.
 * @retval    kGeneralErr    if an error occurred with any step
 */
merr_t mhal_uart_write(int uart, const void *buf, uint32_t n);

/**
 * @brief Receive data on a UART interface
 *
 * @param  uart      the UART interface
 * @param  data      pointer to the buffer which will store incoming data
 * @param  size      number of bytes to receive
 * @param  timeout   timeout in millisecond
 *
 * @retval    kNoErr         on success.
 * @retval    kGeneralErr    if an error occurred with any step
 */
merr_t mhal_uart_read(int uart, void *buf, uint32_t *n, uint32_t timeout);

/**
 * @brief Receive data on a UART interface
 *
 * @param  uart      the UART interface
 * @param  data      pointer to the buffer which will store incoming data
 * @param  size      number of bytes to receive
 * @param  timeout   timeout in millisecond
 *
 * @retval    kNoErr         on success.
 * @retval    kGeneralErr    if an error occurred with any step
 */
merr_t mhal_uart_read_buf(int uart, void *buf, uint32_t n, uint32_t timeout);

/**
 * @brief Read the length of the data that is already recived by uart driver and stored in buffer
 * 
 * @param uart      the UART interface
 *
 * @return    Data length
 */
uint32_t mhal_uart_recved_len(int uart);
/** 
 * @} 
 */
/** 
 * @} 
 */

#endif
