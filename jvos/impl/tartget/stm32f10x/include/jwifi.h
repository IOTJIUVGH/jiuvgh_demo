/**
 ******************************************************************************
 * @file    mwifi.h
 * @author  snow yang
 * @version v1.0.0
 * @date    2019/06/18
 * @brief   this file provides mxchip wifi apis.
 ******************************************************************************
 */

#ifndef __MWIFI_H__
#define __MWIFI_H__

#include <stdint.h>
#include <stdbool.h>

#include "merr.h"

/** 
 * @addtogroup wifi
 * @{
 */

enum
{
  SOFTAP_INTERFACE,
  STATION_INTERFACE,
};
/**
 * @brief Wi-Fi interface
 */
typedef uint8_t mwifi_if_t;

/**
 * @brief Wi-Fi configure mode
 */
typedef enum
{
  CONFIG_BY_NONE,           /**< Default value */
  CONFIG_BY_EASYLINK_V2,    /**< Wlan configured by EasyLink revision 2.0 */
  CONFIG_BY_EASYLINK_PLUS,  /**< Wlan configured by EasyLink Plus */
  CONFIG_BY_EASYLINK_MINUS, /**< Wlan configured by EasyLink Minus */
  CONFIG_BY_MONITOR,        /**< Wlan configured by airkiss from wechat Tencent inc. */
  CONFIG_BY_SOFT_AP,        /**< Wlan configured by EasyLink soft ap mode */
  CONFIG_BY_WAC,            /**< Wlan configured by wireless accessory configuration from Apple inc. */
  CONFIG_BY_WPS,            /**< Wlan configured by Wi-Fi protected setup(WPS). */
  CONFIG_BY_AWS,            /**< Wlan configured by EasyLink AWS */
  CONFIG_BY_USER,           /**< Wlan configured by user defined functions. */
} mxos_config_source_t;

enum
{
  SECURITY_TYPE_NONE,       /**< Open system. */
  SECURITY_TYPE_WEP,        /**< Wired Equivalent Privacy. WEP security. */
  SECURITY_TYPE_WPA_TKIP,   /**< WPA /w TKIP */
  SECURITY_TYPE_WPA_AES,    /**< WPA /w AES */
  SECURITY_TYPE_WPA2_TKIP,  /**< WPA2 /w TKIP */
  SECURITY_TYPE_WPA2_AES,   /**< WPA2 /w AES */
  SECURITY_TYPE_WPA2_MIXED, /**< WPA2 /w AES or TKIP */
  SECURITY_TYPE_AUTO,       /**< It is used when calling @ref mwifi_connect, MXOS read security type from scan result. */
};
/**
 * @brief Wi-Fi security type
 */
typedef uint8_t mwifi_security_t;

/** 
 * @brief IP address attributes
 */
typedef struct
{
  char localip[16];  /**< lcoal ip address */
  char netmask[16];  /**< netmask */
  char gateway[16];  /**< gateway ip address */
  char dnserver[16]; /**< dns server ip address */
} mwifi_ip_attr_t;

/** 
 * @brief Advanced Access-Point attributes
 */
typedef struct
{
  uint8_t bssid[6];          /**< bssid of access-point */
  uint8_t channel;           /**< channel of access-point */
  mwifi_security_t security; /**< security of access-point */
} mwifi_connect_attr_t;

/** 
 * @brief Soft-AP attributes
 */
typedef struct
{
  uint8_t channel;          /**< chanel of soft-ap */
  mwifi_ip_attr_t *ip_attr; /**< ip attributes of soft-ap */
} mwifi_softap_attr_t;

/** 
 * @brief Linker layer information
 */
typedef struct
{
  int is_connected;          /**< The link to wlan is established or not, 0: disconnected, 1: connected. */
  int rssi;                  /**< Signal strength of the current connected AP */
  char ssid[33];             /**< SSID of the current connected wlan */
  uint8_t bssid[6];          /**< BSSID of the current connected wlan */
  char key[65];              /**< The passphrase/PSK of the connected AP */
  int channel;               /**< Channel of the current connected wlan */
  mwifi_security_t security; /**< security of access-point */
} mwifi_link_info_t;

/** 
 * @brief Access-Point information
 */
typedef struct
{
  int rssi;                  /**< Signal strength of the AP */
  char ssid[33];             /**< SSID of the AP */
  uint8_t bssid[6];          /**< BSSID of the AP */
  int channel;               /**< Channel of the AP */
  mwifi_security_t security; /**< security of the AP */
} mwifi_ap_info_t;
/** 
 * @brief Wi-Fi monitor callback function type
 */
typedef void (*mwifi_monitor_cb_t)(uint8_t *data, int len);

enum
{
  CUSTOM_IE_DELETE_ALL = 0,    /**< Remove all custom IE */
  CUSTOM_IE_DELETE_BY_OUI = 1, /**< Remove custom IE by OUI */
};
/** 
 * @brief Wi-Fi custom IE remove type
 */
typedef uint8_t mwifi_custom_ie_remove_type_t;

enum
{
  NOTIFY_STATION_UP = 1,
  NOTIFY_STATION_DOWN,

  NOTIFY_AP_UP,
  NOTIFY_AP_DOWN,

  NOTIFY_ETH_UP,
  NOTIFY_ETH_DOWN,
};
/** 
 * @brief Wi-Fi notify type
 */
typedef uint8_t mwifi_notify_t;
/** 
 * @brief SoftAP association event handler type
*/
typedef void (*asso_event_handler_t)(char *buf, int buf_len, int flags, void *handler_user_data);

/** 
 * @defgroup station Station
 * @brief Station funcitons
 * @{
 */
/**
 * @brief Connect to an Access-Point
 * @param ssid SSID of the Access-Point
 * @param key Passphrase or PSK of the Access-Point
 * @param key_len length of the Passphrase or PSK
 * @param attr extral atrributes of Access-Point
 * @param ip Station IP settings
 * 
 * @return result
 * @retval kNoErr sucess
 * @retval others failure
 * 
 * Connect to an AP normally
 * @code
 * #define SSID "snowyang"
 * #define PASSPHRASE "12345678"
 * 
 * mwifi_connect(SSID, PASSPHRASE, sizeof(PASSPHRASE) - 1, NULL, NULL);
 * @endcode
 */
merr_t mwifi_connect(const char *ssid, char *key, int key_len, mwifi_connect_attr_t *attr, mwifi_ip_attr_t *ip);
/**
 * @brief Add extra Access-Point to connect list
 * @param ssid SSID of the Access-Point
 * @param key Passphrase or PSK of the Access-Point
 * @param key_len length of the Passphrase or PSK
 * @param attr extral atrributes of Access-Point
 * 
 * @return result
 * @retval kNoErr sucess
 * @retval others failure
 */
merr_t mwifi_ap_add(const char *ssid, char *key, int key_len, mwifi_ip_attr_t *attr);
/**
 * @brief Disconnect from current Access-Point
 * 
 * @return result
 * @retval kNoErr sucess
 * @retval others failure
 */
merr_t mwifi_disconnect(void);
/**
 * @brief Set the reconnect interval
 * @param ms reconnect interval in ms
 */
void mwifi_set_reconnect_interval(uint32_t ms); /* 0xFFFFFFFF=don't reconnect */
/** 
 * @}
 */

/** 
 * @defgroup softap SoftAP
 * @brief Soft-AP funcitons
 * @{
 */
/**
 * @brief start soft-ap
 * @param ssid ssid of the soft-ap
 * @param key passphrase of the soft-ap
 * @param channel channel of the soft-ap
 * @param attr ip address atrributes of soft-ap
 * 
 * @return result
 * @retval kNoErr sucess
 * @retval others failure
 */
merr_t mwifi_softap_start(const char *ssid, char *key, int channel, mwifi_ip_attr_t *attr);
/**
 * @brief stop soft-ap
 * 
 * @return result
 * @retval kNoErr sucess
 * @retval others failure
 */
merr_t mwifi_softap_stop(void);
/** 
 * @}
 */

/** 
 * @defgroup wifi_utils Utils
 * @brief Utils funcitons
 * @{
 */
/**
 * @brief scan access-points
 * @param ssid ssid of access-point
 */
void mwifi_scan(const char *ssid);
/**
 * @brief get mac address of us
 * @param mac mac address buffer pointer
 */
void mwifi_get_mac(uint8_t mac[6]);
/**
 * @brief get ip address attributes
 * @param attr ip address attributes structure pointer
 * @param iface wifi interface
 * 
 * @return result
 * @retval kNoErr sucess
 * @retval others failure
 */
merr_t mwifi_get_ip(mwifi_ip_attr_t *attr, mwifi_if_t iface);
/**
 * @brief get linker layer information
 * @param info linker layer information structure pointer
 * 
 * @return result
 * @retval kNoErr sucess
 * @retval others failure
 */
merr_t mwifi_get_link_info(mwifi_link_info_t *info);
/** 
 * @}
 */

/** 
 * @defgroup wifi_power Power Manger
 * @brief Power Manager
 * @{
 */
/**
 * @brief Power on Wi-Fi
 * 
 * @return result
 * @retval kNoErr sucess
 * @retval others failure
 */
merr_t mwifi_on(void);
/**
 * @brief Power off Wi-Fi
 * 
 * @return result
 * @retval kNoErr sucess
 * @retval others failure
 */
merr_t mwifi_off(void);
/**
 * @brief Open the Wi-Fi power save mode
 */
void mwifi_ps_on(void);
/**
 * @brief Close the Wi-Fi power save mode
 */
void mwifi_ps_off(void);
/** 
 * @}
 */

/** 
 * @defgroup monitor Monitor
 * @brief Monitor
 * @{
 */
/**
 * @brief Start Wi-Fi monitor mode
 * 
 * @return result
 * @retval kNoErr sucess
 * @retval others failure
 */
merr_t mwifi_monitor_start(void);
/**
 * @brief Stop Wi-Fi monitor mode
 * 
 * @return result
 * @retval kNoErr sucess
 * @retval others failure
 */
merr_t mwifi_monitor_stop(void);
/**
 * @brief Set channel of Wi-Fi monitor mode
 * @param channel channel of Wi-Fi monitor mode
 * 
 * @return result
 * @retval kNoErr sucess
 * @retval others failure
 */
merr_t mwifi_monitor_set_channel(uint8_t channel);
/**
 * @brief Get channel of Wi-Fi monitor mode
 * 
 * @return channel of Wi-Fi monitor mode
 */
uint8_t mwifi_monitor_get_channel(void);
/**
 * @brief Register Wi-Fi monitor callback function
 * @param func Wi-Fi monitor callback function
 */
void mwifi_monitor_reg_cb(mwifi_monitor_cb_t func);
/**
 * @brief Send raw Wi-Fi monitor frame
 * @param data frame data
 * @param size size of data
 * 
 * @return result
 * @retval kNoErr sucess
 * @retval others failure
 */
merr_t mwifi_monitor_send_frame(uint8_t *data, uint32_t size);
/**
 * @brief Register Wi-Fi manager frame callback function
 * @param func Wi-Fi manager frame callback function
 * 
 * @return result
 * @retval kNoErr sucess
 * @retval others failure
 */
merr_t mwifi_reg_mgnt_cb(mwifi_monitor_cb_t func);
/**
 * @brief Add Wi-Fi custom IE
 * @param iface Wi-Fi interface
 * @param data custom IE data
 * @param size size of custom IE data
 * 
 * @return result
 * @retval kNoErr sucess
 * @retval others failure
 */
merr_t mwifi_custom_ie_add(mwifi_if_t iface, uint8_t *data, uint32_t size);
/**
 * @brief Remove Wi-Fi custom IE
 * @param iface Wi-Fi interface
 * 
 * @return result
 * @retval kNoErr sucess
 * @retval others failure
 */
merr_t mwifi_custom_ie_remove(mwifi_if_t iface);
/**
 * @brief Start monitor mode with Soft-AP coexistance
 * @param ssid ssid of the soft-ap
 * @param key passphrase of the soft-ap
 * @param channel channel of the soft-ap
 * @param attr ip address atrributes of soft-ap
 * @param fn association event handler
 * 
 * @return result
 * @retval kNoErr sucess
 * @retval others failure
 */
merr_t mwifi_monitor_start_with_softap(char *ssid, char *key, int channel, mwifi_ip_attr_t *attr, asso_event_handler_t fn);
/** 
 * @}
 */

/** 
 * @defgroup aws AWS
 * @brief AWS
 * @{
 */
/**
 * @brief Soft-AP up callback type
 */
typedef void (*notify_ap_up_callback)(void);
/**
 * @brief Start AWS Wi-Fi configure mode
 * @param timeout timeout in ms
 * 
 * @return result
 * @retval kNoErr sucess
 * @retval others failure
 */
merr_t mwifi_aws_start(int timeout);
/**
 * @brief Start AWS Wi-Fi configure mode with Soft-AP coexistance
 * @param timeout timeout in ms
 * @param ssid ssid of the soft-ap
 * @param key passphrase of the soft-ap
 * @param channel channel of the soft-ap
 * @param fn Soft-AP up callback
 * 
 * @return result
 * @retval kNoErr sucess
 * @retval others failure
 */
merr_t mwifi_aws_start_with_softap(int timeout, char *ssid, char *key, int channel, notify_ap_up_callback fn);
/**
 * @brief Stop AWS
 * 
 * @return result
 * @retval kNoErr sucess
 * @retval others failure
 */
merr_t mwifi_aws_stop(void);
/** 
 * @}
 */
/** 
 * @}
 */

#endif
