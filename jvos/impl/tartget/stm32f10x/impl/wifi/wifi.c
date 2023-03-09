#include "main.h"
#include <lwip_netconf.h>
#include <lwip/sockets.h>
#include "lwip/netdb.h"
#include <dhcp/dhcps.h>
#include "tcpip.h"
#include <platform/platform_stdlib.h>
#include <wifi/wifi_conf.h>
#include <wifi/wifi_util.h>
#include <wifi/wifi_ind.h>
#include "wifi_structures.h"
#include <osdep_service.h>
#include <device_lock.h>
#include "mwifi.h"
#include "mos.h"
#include "wifimgr.h"
#include "chipid.h"


#ifndef WEAK
#define WEAK             __attribute__((weak))
#endif

WEAK void    mwifi_scan_results_cb(int num, mwifi_ap_info_t *ap_list)
{
    int i;
    
    printf("Got %d APs\r\n", num);
    for(i=0; i<num; i++) {
        printf("\t ssid:%s, bssid[%02x-%02x-%02x-%02x-%02x-%02x], channel %d, security %d, rssi %d\r\n",
            ap_list->ssid, ap_list->bssid[0],ap_list->bssid[1],ap_list->bssid[2],
            ap_list->bssid[3],ap_list->bssid[4],ap_list->bssid[5],
            ap_list->channel, ap_list->security, ap_list->rssi);
        ap_list++;
    }
}
WEAK void    mwifi_status_cb(uint8_t status)
{
    if (status == NOTIFY_STATION_UP)
        printf("station up\r\n");
    else
        printf("station down\r\n");
}

WEAK void    mwifi_connected_ap_info_cb(mwifi_link_info_t *info, char *key, int key_len)
{
    printf("connected AP: %s rssi=%d, channel=%d, security=%d, key %s, key_len %d\r\n",
        info->ssid, info->rssi, info->channel, info->security,
        key, key_len);
}

WEAK void join_fail(merr_t err)
{
    printf("join fail %d\r\n", err);
}

static int id_pass = 0;

void chipid_assert(void)
{
    if (id_pass != 1)
    {
        printf("\r\nWIFI IS NOT READY FOR USE !!!\r\n\r\n");
        while (1)
            ;
    }
}

merr_t	mwifi_disconnect(void)
{
    return sta_disconnect();
}


void	mwifi_scan(const char *ssid)
{
    chipid_assert();
    user_active_scan(ssid);
}

merr_t	mwifi_on(void)
{
    chipid_assert();
    wifi_on(RTW_MODE_STA);
}	
merr_t	mwifi_off(void)
{
    wifi_off();
}
void	mwifi_ps_on(void)
{
    ps_enable();
}
void	mwifi_ps_off(void)
{
    ps_disable();
}

merr_t	mwifi_get_ip(mwifi_ip_attr_t *attr, mwifi_if_t interface)
{
    if (interface == STATION_INTERFACE)
        return get_ip_by_idx(0, attr);
    else
        return get_ip_by_idx(1, attr);
}

#define WLAN_OUI_MICROSOFT		(0x0050F2)
#define WLAN_OUI_WPS			(0x0050F2)
#define WLAN_OUI_TYPE_MICROSOFT_WPA	(1)
#define WLAN_OUI_TYPE_WPS		(4)
#define WLAN_EID_RSN 48
#define WLAN_EID_VENDOR_SPECIFIC    (221)

struct ieee80211_vendor_ie {
    u8 element_id;
    u8 len;
    u8 oui[3];
    u8 oui_type;
};
/* yhb added to support softap mode in easylink */
#define SOFTAP_CHANNEL 6
static int softap_enabled=0, easylink_pause=0;
static char softap_ssid[33]="", softap_key[65];
static uint32_t last_channel_time=0;
static uint8_t mimo_enable=1;
static mwifi_monitor_cb_t sniffer_callback = NULL;
static mwifi_monitor_cb_t rx_mgmt_callback = NULL;
static uint8_t current_channel=0;

static u8 *wlan_find_ie(u8 eid, u8 *ies, int len)
{
        while (len > 2 && ies[0] != eid) {
                len -= ies[1] + 2;
                ies += ies[1] + 2;
        }
        if (len < 2)
                return NULL;
        if (len < 2 + ies[1])
                return NULL;
        return ies;
}

static const u8 *wlan_find_vendor_ie(
    unsigned int oui, u8 oui_type, const u8 *ies, int len)
{
    struct ieee80211_vendor_ie *ie;
    const u8 *pos = ies, *end = ies + len;
    int ie_oui;

    while (pos < end) {
        pos = wlan_find_ie(WLAN_EID_VENDOR_SPECIFIC, pos,
                               end - pos);
        if (!pos) {
            return NULL;
        }

        ie = (struct ieee80211_vendor_ie *)pos;

        if (ie->len < sizeof(*ie)) {
            goto cont;
        }

        ie_oui = ie->oui[0] << 16 | ie->oui[1] << 8 | ie->oui[2];
        if (ie_oui == oui && ie->oui_type == oui_type) {
            return pos;
        }
cont:
        pos += 2 + ie->len;
    }
    return NULL;
}

static inline int get_cipher_info(uint8_t *frame, int frame_len,
		uint8_t *pairwise_cipher_type)
{
	uint8_t cap = frame[24+10]; // 24 is mac header; 8 timestamp, 2 beacon interval; 
	u8 is_privacy = !!(cap & 0x10); // bit 4 = privacy
	const u8 *ie = frame + 36; // 24 + 12
	u16 ielen = frame_len - 36;
	const u8 *tmp;
	int ret = 0;

	tmp = wlan_find_ie(WLAN_EID_RSN, ie, ielen);
	if (tmp && tmp[1]) {
		*pairwise_cipher_type = ENC_CCMP;// TODO: maybe it is a CCMP&TKIP, set to CCMP to try try
	} else {
		tmp = wlan_find_vendor_ie(WLAN_OUI_MICROSOFT, WLAN_OUI_TYPE_MICROSOFT_WPA, ie, ielen);
		if (tmp) {
			*pairwise_cipher_type = ENC_TKIP;
		} else {
			if (is_privacy) {
				*pairwise_cipher_type = ENC_WEP;
			} else {
				*pairwise_cipher_type = ENC_OPEN;
			}
		}
	}

	return ret;
}

static void sniffer_rx_cb(unsigned char *buf, unsigned int len, void* userdata)
{
    signed char rssi;
    uint8_t enc_type;

#if defined(CONFIG_UNSUPPORT_PLCPHDR_RPT) && CONFIG_UNSUPPORT_PLCPHDR_RPT
    if (((ieee80211_frame_info_t *)userdata)->type == RTW_RX_UNSUPPORT ) {
        rtw_rx_info_t *lsig = (rtw_rx_info_t*)buf;

        lsig_input(lsig->length, lsig->rssi, mos_time());

        return ;
    }

    rssi = ((ieee80211_frame_info_t *)userdata)->rssi;

    /* check the RTS packet */
    if ((buf[0] == 0xB4) && (len == 16)) { // RTS
        rts_update(buf, rssi, mos_time());
        return;
    }
    /* check beacon/probe response packet */
    if ((buf[0] == 0x80) || (buf[0] == 0x50)) {
        get_cipher_info(buf, len, &enc_type);
        beacon_update(&buf[16], enc_type);
    } 
#endif    
	if (sniffer_callback == NULL)
		return;

	sniffer_callback(buf, len);
}

merr_t	mwifi_monitor_start(void)
{
    int ret;

    chipid_assert();
    
    scan_lock();
	wifi_enter_promisc_mode();
    scan_unlock();
#if defined(CONFIG_UNSUPPORT_PLCPHDR_RPT) && CONFIG_UNSUPPORT_PLCPHDR_RPT
    if (mimo_enable == 1) {
        lsig_init();
        ret = wifi_set_promisc(RTW_PROMISC_ENABLE_4, sniffer_rx_cb, 0);
        wifi_promisc_ctrl_packet_rpt(1);  // enable rx RTS/CTS
    } else {
        ret = wifi_set_promisc(RTW_PROMISC_ENABLE_2, sniffer_rx_cb, 0);
    }
#else
    ret = wifi_set_promisc(RTW_PROMISC_ENABLE_2, sniffer_rx_cb, 0);
#endif

    return ret;
}

merr_t mwifi_monitor_start_with_softap(char *ssid, char *key,int channel, mwifi_ip_attr_t *attr,asso_event_handler_t fn)
{
    int ret;
    
    chipid_assert();

	if ((ssid != NULL) && (strlen(ssid) > 0)) {
		mwifi_softap_start(ssid, key, channel, attr);
        wifi_reg_event_handler(WIFI_EVENT_STA_ASSOC, fn, NULL);
	}else
	{
		wifi_enter_promisc_mode();
	}
#if defined(CONFIG_UNSUPPORT_PLCPHDR_RPT) && CONFIG_UNSUPPORT_PLCPHDR_RPT
    if (mimo_enable == 1) {
        ret = wifi_set_promisc(RTW_PROMISC_ENABLE_4, sniffer_rx_cb, 0);
        wifi_promisc_ctrl_packet_rpt(1);  // enable rx RTS/CTS
    } else {
        ret = wifi_set_promisc(RTW_PROMISC_ENABLE_2, sniffer_rx_cb, 0);
    }
#else
    ret = wifi_set_promisc(RTW_PROMISC_ENABLE_2, sniffer_rx_cb, 0);
#endif
    return ret;
}

int mwifi_monitor_rx_mimo(int enable)
{
    if (enable == 0)
        mimo_enable = 0;
    else
        mimo_enable = 1;

    return 0;
}

merr_t	mwifi_monitor_stop(void)
{
#if defined(CONFIG_UNSUPPORT_PLCPHDR_RPT) && CONFIG_UNSUPPORT_PLCPHDR_RPT
    if (mimo_enable == 1)
        wifi_promisc_ctrl_packet_rpt(0);
#endif
	wifi_set_promisc(RTW_PROMISC_DISABLE, NULL, 0);
	wifi_off();
	vTaskDelay(100);
	if (wifi_on(RTW_MODE_STA) < 0){
		printf("ERROR: Wifi on failed!\r\n");
	}

    return 0;
}	
merr_t	mwifi_monitor_set_channel(uint8_t channel)
{
    current_channel = channel;
	return wifi_set_channel(channel);
}

uint8_t mwifi_monitor_get_channel(void)
{
    return current_channel;
}

void	mwifi_monitor_reg_cb(mwifi_monitor_cb_t func)
{
    sniffer_callback = func;
}

merr_t	mwifi_monitor_send_frame(uint8_t *data,	uint32_t size)
{
    return wext_send_mgnt(WLAN0_NAME, (char*)data, (__u16)size, 1);
}

static void rtw_mgnt_rx(char *buf, int buf_len, int flags, void* handler_user_data )
{
    if (rx_mgmt_callback == NULL)
        return;
    rx_mgmt_callback(buf, buf_len);
}

merr_t	mwifi_reg_mgnt_cb(mwifi_monitor_cb_t func)
{
    if (func != NULL) {
        rx_mgmt_callback = func;
		wifi_set_indicate_mgnt(1);
		wifi_reg_event_handler(WIFI_EVENT_RX_MGNT, rtw_mgnt_rx,NULL);
	} else {
	    rx_mgmt_callback = NULL;
		wifi_set_indicate_mgnt(0);
	}
}

merr_t	mwifi_custom_ie_add(mwifi_if_t iface, uint8_t *custom_ie, uint32_t len)
{
  merr_t err = kNoErr;
  uint8_t *ie = NULL;
  uint8_t ret;
  char *name;
  
  ie = malloc(len + 2);
  if(ie == NULL) {
    err = kNoMemoryErr;
    goto exit;
  }
  ie[0] = 221;
  ie[1] = len;
  memcpy(&ie[2], custom_ie, len);
  rtw_custom_ie_t buf[1] = {{ie, PROBE_REQ|PROBE_RSP|BEACON}};

  if (iface == STATION_INTERFACE)
    name = WLAN0_NAME;
  else
    name = WLAN1_NAME;
  ret = wext_add_custom_ie(name, buf, 1);
  free(ie);
  if (ret != 0)
    err = kGeneralErr;
  
exit:
  return err;
}

merr_t	mwifi_custom_ie_remove(mwifi_if_t iface)
{
  merr_t err = kNoErr;
  uint8_t ret;
  char *name;

  if (iface == STATION_INTERFACE)
    name = WLAN0_NAME;
  else
    name = WLAN1_NAME;
  ret = wext_del_custom_ie(name);
  if (ret != 0)
    err = kGeneralErr;
  
exit:
  return err;
}

merr_t mxos_network_init(void)
{
    uint8_t mac[6];
    uint8_t chipid[8], readid[32];
    
    wifi_driver_init();

    mwifi_get_mac(mac);
    chipid_calc(mac, chipid);
    efuse_otp_read(0, 8, readid);

    if (memcmp(chipid, readid, 8) == 0) {
        id_pass=1;
    }

    return kNoErr;
}

int mxos_wlan_driver_version(char *outVersion, uint8_t inLength)
{
    if (id_pass == 1)
        snprintf(outVersion, inLength, "emc3080-1.0.0");
    else
        snprintf(outVersion, inLength, "emc3080-x");
    return 0;
}

int mxosWlanSuspend(void)
{
    sta_disconnect();
    mwifi_softap_stop();

    return 0;
}
