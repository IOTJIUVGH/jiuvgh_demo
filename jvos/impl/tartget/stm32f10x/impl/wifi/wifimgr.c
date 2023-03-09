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
#include <osdep_service.h>
#include <device_lock.h>
#include "mwifi.h"
#include "mos.h"
#include "mxchip_debug.h"

static uint8_t log_enable = 0;

#define wifi_log(M, ...) do { \
                       if (log_enable == 1) { \
                            dbg_printf("[wifimgr %d] " M "\r\n", __LINE__, ##__VA_ARGS__); \
                       } \
                   } while(0)

#ifndef WLAN0_NAME
  #define WLAN0_NAME		"wlan0"
#endif
#ifndef WLAN1_NAME
  #define WLAN1_NAME		"wlan1"
#endif

#ifndef WEAK
#define WEAK             __attribute__((weak))
#endif

typedef struct _scan_rst_t_ {
	rtw_scan_result_t result;
	
	struct _scan_rst_t_ *next;
}mxchip_scan_rst_t;

#define CMP_MAC( a, b )  (((a[0])==(b[0]))&& \
                          ((a[1])==(b[1]))&& \
                          ((a[2])==(b[2]))&& \
                          ((a[3])==(b[3]))&& \
                          ((a[4])==(b[4]))&& \
                          ((a[5])==(b[5])))

#define NULL_MAC( a )  (((a[0])==0)&& \
                        ((a[1])==0)&& \
                        ((a[2])==0)&& \
                        ((a[3])==0)&& \
                        ((a[4])==0)&& \
                        ((a[5])==0))

#define DUAL_MODE_RETRY_INTERVAL 10*1000
#define STA_MODE_RETRY_INTERVAL 100
#define STA_MODE_MAX_SCANTIME 5*1000
#define NEVER_TIMEOUT     0xFFFFFFFF

#define CONFIG_AP_LIST_SIZE (6)
#define MAX_SCAN_NUM    (32)
#define WLC_EVENT_MSG_LINK      (0x01)    /** link is up */

typedef enum {
    STOPPED,
	SLEEPING,
	SCANING,
	JOINING,
	CONNECTED,
	DISCONNECTED,
	ADVTRY,
}STA_STATE;

enum {
    WIFI_ERROR_INIT = 1,
    WIFI_ERROR_NOGW,
    WIFI_ERROR_NOBUS_CREDIT,
};

#define USER_SCAN_FLAG      0x01
#define MF_SCAN_FLAG        0x04
#define JOIN_SCAN_FLAG      0x08
#define CMD_SCAN_FLAG       0x20

typedef struct _static_ip
{
    uint8_t  dhcp;
    uint32_t ip;
    uint32_t mask;
    uint32_t gw;
    uint32_t dns1;
} static_ip_t;

typedef struct _connect_ap_list
{
    char ssid[33];
    char key[64];
    uint8_t key_len;
    static_ip_t ip;
    struct _connect_ap_list *next;
} connect_ap_list_t;


struct direct_conn_ap {
    char         ssid[33];
    char         key[64];
    uint8_t      bssid[6];
    uint8_t      key_len;
    uint8_t      channel;
    uint8_t      security;
    uint8_t      valid;
    static_ip_t  ip;
};

/******************************************************
 *               Variables Definitions
 ******************************************************/
static uint8_t sta_connected = 0, uap_connected = 0;
static uint32_t sta_retry_interval = STA_MODE_RETRY_INTERVAL;
static STA_STATE sta_state = STOPPED;

static connect_ap_list_t * p_connect_ap = NULL, *try_ap=NULL;

static mxchip_scan_rst_t *p_result_head = NULL;

static int sta_dhcp_mode = 0;

static mos_semphr_id_t wifimgr_sem, *scan_sem, *softap_disassoc_semphr;
static mos_mutex_id_t ap_list_lock, scanlock;
static mos_thread_id_t wifimgr_thread_id;
static int user_req_scan = 0;
static int user_stop_sta = 0;

static int softap_channel = 6, softap_enabled=0;

static uint8_t assoc_ap_bssid[6];
static char softap_ssid[33], softap_key[64];
static int uap_client_num = 0;
static int ps_enabled = 0;
static char active_scan_ssid[33];
int tdma_on = 0;
int tdma_resume = 0;

extern struct netif xnetif[NET_IF_NUM]; 

extern unsigned char psk_essid[NET_IF_NUM][32+4];
extern unsigned char psk_passphrase[NET_IF_NUM][64 + 1];
extern unsigned char wpa_global_PSK[NET_IF_NUM][20 * 2];
extern unsigned char psk_passphrase64[64 + 1];
void wlan_set_softap_tdma(int value);

uint8_t *winc_get_mac_addr(void);
static void insert_result_by_rssi(rtw_scan_result_t* result);
static void set_sta_connection(uint8_t connection);
static void wifi_reset(void);
static int sectype_rtl2mx(rtw_security_t sec);

#define MAX_NO_REPORT_TIME 6

static int no_ap_report_times = 0;

extern rtw_mode_t wifi_mode;

void mwifi_get_mac(uint8_t mac[6])
{
    wifi_mac_get(mac);
}

merr_t	get_ip_by_idx(uint8_t idx, mwifi_ip_attr_t *attr)
{
	struct in_addr in_addr;
	uint8_t *ip;

    ip = (uint8_t *)LwIP_GetIP(&xnetif[idx]);
    memcpy(&in_addr.s_addr, ip, 4);
	strcpy(attr->localip, inet_ntoa(in_addr));
    ip = (uint8_t *)LwIP_GetMASK(&xnetif[idx]);
	memcpy(&in_addr.s_addr, ip, 4);
	strcpy(attr->netmask, inet_ntoa(in_addr));
    ip = (uint8_t *)LwIP_GetGW(&xnetif[idx]);
	memcpy(&in_addr.s_addr, ip, 4);
	strcpy(attr->gateway, inet_ntoa(in_addr));
    LwIP_GetDNS((struct ip_addr*)&in_addr);
	strcpy(attr->dnserver, inet_ntoa(in_addr));

    return 0;
}

static void set_netif_static_ip(uint8_t idx, static_ip_t *attr)
{
	struct ip_addr ipaddr;
	struct ip_addr netmask;
	struct ip_addr gw, dns;
    struct netif *pnetif = &xnetif[idx];
    
    ipaddr.addr = attr->ip;
    netmask.addr = attr->mask;
    gw.addr = attr->gw;
    dns.addr = attr->dns1;
    
    LwIP_SetDNS(&dns);
#if LWIP_VERSION_MAJOR >= 2
	netif_set_addr(pnetif, ip_2_ip4(&ipaddr), ip_2_ip4(&netmask),ip_2_ip4(&gw));
#else
	netif_set_addr(pnetif, &ipaddr , &netmask, &gw);
#endif
    netif_set_up(pnetif); 
	if (idx == 0)
		netif_set_default(pnetif);
    wifi_log("ip: %p, mask %p, gw %p, dns %p\r\n",
        ipaddr.addr, netmask.addr, gw.addr, dns.addr);
}

int StopScan(void)
{
    return kNoErr;
}

merr_t	mwifi_get_link_info(mwifi_link_info_t *info	)
{
    rtw_wifi_setting_t setting;
	int rssi;

	
	rtw_memset(info, 0, sizeof(mwifi_link_info_t));
	if(wifi_get_setting(WLAN0_NAME,&setting)) {
		return 0;
	}

	if (setting.mode == RTW_MODE_AP)
		return 0;

	wifi_get_rssi(&rssi);
    memcpy(info->bssid, assoc_ap_bssid, 6);
	info->is_connected = 1;
	info->channel = setting.channel;
	info->rssi = rssi;
	rtw_memcpy(info->ssid, setting.ssid, 32);
    info->security = sectype_rtl2mx(setting.security_type);
    return 0;
}

int is_uap_mode(void)
{
	return 0;// TODO: uap_enabled;
}

int is_ps_enabled(void)
{
	return ps_enabled;
}

void wifi_ps_mode_set(void)
{
	if (ps_enabled == 1)
		wifi_enable_powersave();
	else
		wifi_disable_powersave();
}

void ps_enable(void)
{
    ps_enabled = 1;
	wifi_enable_powersave();
}

void ps_disable(void)
{
    wifi_disable_powersave();
	ps_enabled = 0;
}

void enable_ps_mode(int unit_type, int unitcast_ms, int multicast_ms)
{
	ps_enable();
}

void disable_ps_mode(void)
{
    ps_disable();
}

void wifimgr_debug_enable(bool enable)
{
    if (enable)
        log_enable = 1;
    else
        log_enable = 0;
}

static int sectype_rtl2mx(rtw_security_t sec)
{
    int ret;

    switch (sec) {
    case RTW_SECURITY_OPEN:
        ret = SECURITY_TYPE_NONE;
        break;
    case RTW_SECURITY_WEP_PSK:
        ret = SECURITY_TYPE_WEP;
        break;
    case RTW_SECURITY_WPA_TKIP_PSK:
        ret = SECURITY_TYPE_WPA_TKIP;
        break;
    case RTW_SECURITY_WPA_AES_PSK:
        ret = SECURITY_TYPE_WPA_AES;
        break;
    case RTW_SECURITY_WPA2_AES_PSK:
        ret = SECURITY_TYPE_WPA2_AES;
        break;
    case RTW_SECURITY_WPA2_TKIP_PSK:
        ret = SECURITY_TYPE_WPA2_TKIP;
        break;
    case RTW_SECURITY_WPA2_MIXED_PSK:
        ret = SECURITY_TYPE_WPA2_AES;
        break;
	case RTW_SECURITY_WPA_WPA2_MIXED:
		ret = SECURITY_TYPE_WPA2_AES;
		break;
    default:
        ret = SECURITY_TYPE_WPA2_AES;
        break;
    }

    return ret;
}

static int sectype_mx2rtl(int sec)
{
    int ret;

    switch (sec) {
    case SECURITY_TYPE_NONE:
        ret = RTW_SECURITY_OPEN;
        break;
    case SECURITY_TYPE_WEP:
        ret = RTW_SECURITY_WEP_PSK;
        break;
    case SECURITY_TYPE_WPA_TKIP:
        ret = RTW_SECURITY_WPA_TKIP_PSK;
        break;
    case SECURITY_TYPE_WPA_AES:
        ret = RTW_SECURITY_WPA_AES_PSK;
        break;
    case SECURITY_TYPE_WPA2_AES:
        ret = RTW_SECURITY_WPA2_AES_PSK;
        break;
    case SECURITY_TYPE_WPA2_TKIP:
        ret = RTW_SECURITY_WPA2_TKIP_PSK;
        break;
    case SECURITY_TYPE_WPA2_MIXED:
        ret = RTW_SECURITY_WPA2_MIXED_PSK;
        break;
    default:
        ret = RTW_SECURITY_UNKNOWN;
        break;
    }

    return ret;
}


/******************************************************
 *               Function Definitions
 ******************************************************/
static void free_ap_list(void);
static void mxchip_timer_tick(void);
static void health_monitor_thread(void* arg);
static void softap_tdma_resume_therad(void* arg);
static void wifimgr_thread(void* arg);

extern int user_scan(int advance_scan);

extern void wifi_reboot_event(int type);

void wifi_status_callback(int connection)
{
    uint32_t curtime = sys_now();

    srand(curtime);
    tcp_init();
	udp_init();
    mwifi_status_cb(connection);
}

static void set_sta_connection(uint8_t connection)
{
	if (sta_connected == connection)
		return;
    sta_connected = connection;

	if (connection == 1) {
        wifi_log("Connected\r\n");
		wifi_status_callback(NOTIFY_STATION_UP);
	} else {
		if (sta_state == STOPPED)
			return;
	    wifi_log( "Disconnected\r\n");
		wifi_status_callback(NOTIFY_STATION_DOWN);
        if (sta_retry_interval == 0xFFFFFFFF) {

            sta_state = DISCONNECTED;
        } else {
            sta_state = SLEEPING;
            mos_semphr_release(wifimgr_sem);
        }
    }

    
	if (connection == 1) {
        if (is_ps_enabled()) {
            wifi_log("ps enable\r\n");
            wifi_enable_powersave();
        } else {
            wifi_log("ps disable\r\n");
            wifi_disable_powersave();
        }
    }
}


static void stopscan(void)
{
    mos_msleep(10);
}

static void print_ap_list(void)
{
    connect_ap_list_t * ap_info = p_connect_ap;
    int i=0;
    
    if (ap_info == NULL) {
        wifi_log("No AP\r\n");
        return;
    }
    wifi_log("Connect AP info: \r\n");
    while(ap_info) {
        wifi_log("\tSSID(%d): %s\r\n\tkey: %s(%d)\r\n\tretry interval %d\r\n\r\n", 
            i++, ap_info->ssid, ap_info->key, ap_info->key_len, sta_retry_interval);
        ap_info = ap_info->next;
    }
}

int get_sta_connection(void)
{
	return sta_connected;
}

void set_uap_connection(uint8_t connection)
{
    if (connection == 0)
        wifi_status_callback(NOTIFY_AP_DOWN);
    else
        wifi_status_callback(NOTIFY_AP_UP);
}


int get_uap_connection(void)
{
	return uap_connected;
}

int get_connection(void)
{
	if (sta_connected == 1)
        return 1;
    if (uap_connected == 1)
        return 1;
    return 0;
}

void    mwifi_set_reconnect_interval(uint32_t ms)
{
    if (ms == sta_retry_interval)
        return;
    
    if (ms < 100)
        ms = 100;

    if (sta_retry_interval == 0xFFFFFFFF) {

        sta_retry_interval = ms;
        wifi_log("change retry interval from NEVER to %d", ms);
        if (sta_state == DISCONNECTED) {
            sta_state = SLEEPING;
            mos_semphr_release(wifimgr_sem);
        }
    }
    sta_retry_interval = ms;
}

static void _uap_up(void)
{
}

static void uap_down()
{
}

void uap_dns_redirector(int enable)
{

    
}

static void set_static_ip(static_ip_t *cfg, mwifi_ip_attr_t *attr)
{
    uint32_t tmp;

    if (attr == NULL) {
        cfg->dhcp = 1;
        return;
    }
    cfg->dhcp = 0;
    
    cfg->ip = inet_addr((char*)attr->localip);
    tmp = inet_addr((char*)attr->netmask);
    if (tmp == 0xFFFFFFFF)
        tmp = 0x00FFFFFF;// if not set valid netmask, set as 255.255.255.0
    cfg->mask = (tmp);
    cfg->gw = inet_addr((char*)attr->gateway);

    cfg->dns1 = inet_addr((char*)attr->dnserver);
}

/* insert an AP to connect without scan */
static int insert_ap_manual(const char *ssid,	char *key, mwifi_connect_attr_t *attr)
{
	/* New BSSID - add it to the list */
    mxchip_scan_rst_t *p;
	rtw_scan_result_t *result;
	
	if (NULL_MAC(attr->bssid)) // not a valid AP scan results
		return 0;
	
	p = (mxchip_scan_rst_t*)calloc(sizeof(mxchip_scan_rst_t), 1);
	if (p == NULL)
        return 0;
	
	result = &p->result;
	free_ap_list();
	memcpy(result->BSSID.octet, attr->bssid, 6);
	result->SSID.len = strlen(ssid);
	if (result->SSID.len > 32)
		result->SSID.len = 32;
	memcpy(result->SSID.val, ssid, strlen(ssid));
	result->SSID.val[result->SSID.len] = 0;
	result->channel = attr->channel;
	result->security = sectype_mx2rtl(attr->security);
	p->next = p_result_head;
	p_result_head = p;

	return 1;
}

merr_t	mwifi_connect(const char *ssid,	char *key, int key_len, mwifi_connect_attr_t *attr, mwifi_ip_attr_t *ip)
{
    int ret;
    connect_ap_list_t *apinfo;
    
	//sta_disconnect();
  
    apinfo = (connect_ap_list_t*)malloc(sizeof(connect_ap_list_t));
    if (apinfo == NULL)
        return kNoMemoryErr;

    strncpy(apinfo->ssid, ssid, 32);
    if (key) {
        memcpy(apinfo->key, key, 64);
        apinfo->key_len = key_len;
    } else {
        apinfo->key_len = 0;
        memset(apinfo->key, 0, 64);
    }
 
    set_static_ip(&apinfo->ip, ip);
    mos_mutex_lock(ap_list_lock);
    apinfo->next = p_connect_ap;
    p_connect_ap = apinfo;
    try_ap = NULL;
    mos_mutex_unlock(ap_list_lock);
 
    print_ap_list();
    if (attr == NULL) {
        sta_state = SLEEPING;
        mos_semphr_release(wifimgr_sem);
        return kNoErr;
    }
 
    ret = insert_ap_manual(ssid, key, attr);
    if (ret == 1)
        sta_state = ADVTRY;
    else
        sta_state = SLEEPING;
  
    mos_semphr_release(wifimgr_sem);

    return kNoErr;
}

merr_t	mwifi_ap_add(const char *ssid,	char *key, int key_len, mwifi_ip_attr_t *attr)
{
    connect_ap_list_t *apinfo;

    apinfo = (connect_ap_list_t*)malloc(sizeof(connect_ap_list_t));
    if (apinfo == NULL)
        return -1;

    strncpy(apinfo->ssid, ssid, 32);
    memcpy(apinfo->key, key, 64);
    apinfo->key_len = key_len;
    set_static_ip(&apinfo->ip, attr);
    mos_mutex_lock(ap_list_lock);
    apinfo->next = p_connect_ap;
    p_connect_ap = apinfo;
    try_ap = NULL;
    mos_mutex_unlock(ap_list_lock);
    return 0;
}

static void remove_ap_list(void)
{
    connect_ap_list_t *p = p_connect_ap, *q;

    wifi_log("remove extra network\r\n");
    mos_mutex_lock(ap_list_lock);
    p_connect_ap = NULL;
    try_ap = NULL;
    mos_mutex_unlock(ap_list_lock);
    while(p != NULL) {
        q = p;
        p = p->next;
        free(q);
    }
}

void wlan_set_channel(int channel)
{
    softap_channel = channel;
}

void wlan_set_softap_tdma(int value)
{
    if ((value < 0) || (value > 100)) {
        value = 0;
        wifi_log( "Invalid TDMA value\r\n");
    } 
    if (value == 0) {
        if (tdma_on == 0)
            return;
    } 

    tdma_on = value;
    wifi_log("Set TDMA %d\r\n", value);
    wifi_set_tdma_param(value, 0,0,0);
}

int get_softap_channel(void)
{
	return softap_channel;
}

int sta_disconnect(void)
{
    wifi_log( "STA disconnect, state %d\r\n", sta_state);
	int i = 0;
	if (sta_state == STOPPED) 
		return kNoErr;

	user_stop_sta = 1;
    mos_semphr_release(wifimgr_sem);
    while(sta_state != STOPPED) {
		mos_msleep(100);
		i++;
		if (i == 200) {// must bigger than  RTW_JOIN_TIMEOUT 15000 ms
			user_stop_sta = 0;
			wifi_log( "STA disconnect timeout, state %d\r\n", sta_state);
			break;
		}
	}
    wifi_log( "STA disconnected, state %d\r\n", sta_state);
    return kNoErr;
}

int uap_stop(void)
{
	softap_enabled = 0;
    // if(rltk_wlan_running(WLAN1_IDX) == 0) {
	// 	return RTW_SUCCESS;
	// }

    wifi_set_mode(RTW_MODE_STA);
    set_uap_connection(0);
	// wifi_suspend_softap(WLAN0_NAME);

    return RTW_SUCCESS;
}

int wlan_disconnect( void )
{
    wifi_log( "wlan disconnect\r\n");
    sta_disconnect();
	uap_stop();
    return RTW_SUCCESS;
}

static void insert_result_by_rssi(rtw_scan_result_t* result)
{
    mxchip_scan_rst_t *p, *next, *prev;
  
    /* New BSSID - add it to the list */
    p = (mxchip_scan_rst_t*)malloc(sizeof(mxchip_scan_rst_t));
    if (p == NULL) {
		wifi_log( "insert malloc fail\r\n");
		return;
    }
	wifi_log( "insert %s %d\r\n",
		result->SSID.val, result->signal_strength);
    memcpy(&p->result, result, sizeof(rtw_scan_result_t));
    p->next = NULL;
    if (p_result_head == NULL) {
        p->next = p_result_head;
        p_result_head = p;
        goto DONE;
    }
    
    prev = p_result_head;
    while(prev->next != NULL) {
        prev = prev->next;
    }
    prev->next = p;
DONE:
    return;
}

static void append_ap_list(rtw_scan_result_t *result, mwifi_ap_info_t *list, int num)
{
	int i, j;
    int16_t rssi = result->signal_strength;
    
	for(i=0;i<num;i++) {
		if(rssi > list[i].rssi)
			break;
	}
    
	for (j=num; j>i; j--) {
        memcpy(&list[j], &list[j-1], sizeof(mwifi_ap_info_t));
	}
    
	strncpy(list[i].ssid, (char*)result->SSID.val, 32);
    list[i].rssi = rssi;
    list[i].channel = result->channel;
    memcpy(list[i].bssid, result->BSSID.octet, 6);
    list[i].security = sectype_rtl2mx(result->security);
}


static void get_scan_result_list()
{
    mxchip_scan_rst_t* p;
    int i;
    int apnum;
    mwifi_ap_info_t *aplist = NULL;
    
    p = p_result_head;
    apnum = 0;
    while(p != NULL) {
        apnum++;
        p = p->next;
    }
    if (apnum > MAX_SCAN_NUM)
        apnum = MAX_SCAN_NUM;

    if (apnum > 0) {
        aplist = (mwifi_ap_info_t *)malloc(sizeof(mwifi_ap_info_t) * apnum);
    }
    
    if (aplist == NULL) {
        apnum = 0;
        mwifi_scan_results_cb(0, NULL);
        return;
    }

    p = p_result_head;
    i = 0;
    while(p != NULL) {
        append_ap_list(&p->result, aplist, i++);
        p = p->next;
        if (i == apnum)
            break;
    }
    mwifi_scan_results_cb(apnum, aplist);
    free(aplist);
}


/**
 *  Scan result callback
 *  Called whenever a scan result is available
 *
 *  @param result_ptr : pointer to pointer for location where result is stored. The inner pointer
 *                      can be updated to cause the next result to be put in a new location.
 *  @param user_data : unused
 */
static rtw_result_t app_scan_result_handler( rtw_scan_handler_result_t* malloced_scan_result )
{
    rtw_scan_result_t* record = &malloced_scan_result->ap_details;
    int i;
    mxchip_scan_rst_t* p;
    rtw_scan_result_t* result;
    
    if (malloced_scan_result->scan_complete == RTW_TRUE){
		wifi_log( "scan done, set scan sem\r\n");
		if (p_result_head == NULL) { // no AP result
			no_ap_report_times++;
			wifi_log( "NO AP found, times %d\r\n", no_ap_report_times);
            
		}
		mos_semphr_release(scan_sem);

        return RTW_SUCCESS;
    }

	if (record->SSID.len > 32)
		record->SSID.len = 32;
	record->SSID.val[record->SSID.len] = 0;

	no_ap_report_times = 0;
    /* New BSSID - add it to the list */
    insert_result_by_rssi(record);
	return RTW_SUCCESS;
}

static connect_ap_list_t *get_connecting_ap(rtw_scan_result_t* result, char *ssid)
{
    connect_ap_list_t * ap_info = p_connect_ap;
    
    while(ap_info!=NULL) {
        if (strncmp(result->SSID.val, ap_info->ssid, 32) == 0) {
            wifi_log("Got ssid(%d): %s, key(%d):%s\r\n",
                result->SSID.len, result->SSID.val,
                ap_info->key_len, ap_info->key);
            return ap_info;
        }
        ap_info = ap_info->next;
    }

    return NULL;
}

static int select_ap_to_join(void)
{
    mxchip_scan_rst_t* p;
    rtw_scan_result_t* result;
    int i;
    int ret = -30;
    int match_ap_num = 0;
	uint8_t     pscan_config;
    char *key;
    uint8_t key_len;
	int err_unknow_num = 0;
    char tmp_key[8];
    connect_ap_list_t * ap_info;
        
    p = p_result_head;
    while(p!= NULL) {
        result = &p->result;
        wifi_log( 
               "ssid(%d) %s, %02x-%02x-%02x-%02x-%02x-%02x, rssi %d, channel %d\r\n",
               result->SSID.len, result->SSID.val,
               result->BSSID.octet[0],result->BSSID.octet[1],
               result->BSSID.octet[2],result->BSSID.octet[3],
               result->BSSID.octet[4],result->BSSID.octet[5],
               result->signal_strength, result->channel);
        p = p->next;
    }
    p = p_result_head;
    while(p != NULL) {
        result = &p->result;
        mos_mutex_lock(ap_list_lock);
        ap_info = get_connecting_ap(result, result->SSID.val);
        mos_mutex_unlock(ap_list_lock);
        if (ap_info != NULL) { // find the sepcific AP, connect it
            key = ap_info->key;
            key_len = ap_info->key_len;
            if (ap_info->key_len == 64) {
                memcpy(wpa_global_PSK[0], key, 40);
                strcpy(psk_essid[0], result->SSID.val);
                memset(tmp_key, 0, 8);
                key = tmp_key;
        		key_len = 8;
                wifi_log( 
                    "connect %s - %02x%02x(64)\r\n", result->SSID.val, (uint8_t)wpa_global_PSK[0][0], (uint8_t)wpa_global_PSK[0][1]);
            } else {
                wifi_log( 
                    "connect %s - %02x%02x(%d)\r\n", result->SSID.val, (uint8_t)key[0], (uint8_t)key[1], key_len);
            }
            match_ap_num++;
            wifi_log( 
               "find ap: %02x-%02x-%02x-%02x-%02x-%02x\r\n",
               result->BSSID.octet[0],result->BSSID.octet[1],
               result->BSSID.octet[2],result->BSSID.octet[3],
               result->BSSID.octet[4],result->BSSID.octet[5]);
			if ((result->channel > 0) && (result->channel < 14)) {
				pscan_config = PSCAN_ENABLE | PSCAN_FAST_SURVEY;
				wifi_set_pscan_chan((uint8_t *)&result->channel, &pscan_config, 1);
			}
			if (result->security == RTW_SECURITY_UNKNOWN)
				return -1;

			ret = wifi_connect_bssid(result->BSSID.octet, result->SSID.val, result->security,  
		               key, 6, result->SSID.len, key_len, 0, NULL);

			if (ret != RTW_SUCCESS)
				ret = wifi_connect(result->SSID.val, result->security,  
		               key, result->SSID.len, key_len, 0, NULL);
            if (ret == RTW_SUCCESS)
            {
                wifi_log( "join succes\r\n");

                if (ap_info->ip.dhcp != 1) {
                    set_netif_static_ip(0, &ap_info->ip);
                    ret = RTW_SUCCESS;
                } else {
                	wifi_log( "DHCP start <%d>\r\n", sys_now());
                    ret = LwIP_DHCP(0, DHCP_START);
                    if (DHCP_ADDRESS_ASSIGNED == ret) {
                        wifi_log( "DHCP success, STA connected\r\n");
                    	ret = RTW_SUCCESS;
                    } else {
                        ret = RTW_ERROR;
                        wifi_log( "DHCP fail<%d> return %d\r\n", sys_now(), ret);
                    }
                }

                if (ret == RTW_SUCCESS) {
                    mwifi_link_info_t info;
                    
                    strncpy(info.ssid, result->SSID.val, 32);
                    memcpy(info.bssid, result->BSSID.octet, 6);
                    memcpy(assoc_ap_bssid, result->BSSID.octet, 6);
                    info.channel = result->channel;
                    info.security = sectype_rtl2mx(result->security);
                    info.is_connected = 1;
                    wifi_get_rssi(&info.rssi);
					if (result->security & (WPA_SECURITY|WPA2_SECURITY)) {
                    	mwifi_connected_ap_info_cb(&info, wpa_global_PSK[0], 64);
					} else {
						mwifi_connected_ap_info_cb(&info, key, key_len);
					}
                }
                return ret;
            }else {
                wifi_log( "join fail ret %d, err %d\r\n", ret, wifi_get_last_error());
                join_fail(ret);
				if (wifi_get_last_error() == RTW_UNKNOWN) {
					err_unknow_num++;
					if (err_unknow_num > 1) {
						wifi_log( "wifi reset, delay 1s\r\n");
                        mos_msleep(1000);
						wifi_reset();
                        err_unknow_num = 0;
					}
				}
            }
        }
                
        p = p->next;
    }
    
    if (match_ap_num == 0)
        join_fail(-30);
    wifi_log( "retry later\r\n");
    
    return -1;
}

/*!
 ******************************************************************************
 * Scans for access points and prints out results
 *
 * @return  0 for success, otherwise error
 */
int user_active_scan(char *ssid)
{
	if (user_req_scan != 0)
		return 0;
	
    user_req_scan |= USER_SCAN_FLAG;

    if (ssid && strlen(ssid)>0)
        strncpy(active_scan_ssid, ssid, 33);
    else
        memset(active_scan_ssid, 0, sizeof(active_scan_ssid));
    mos_semphr_release(wifimgr_sem);
    return 0;
}

void dns_server_remove_all(void);
static void sta_iface_down(void *arg)
{
	struct netif *iface = &xnetif[0];
	struct ip_addr ipaddr;
	struct ip_addr netmask;
	struct ip_addr gw;
    
    ipaddr.addr = 0;
    netmask.addr = 0;
    gw.addr = 0;
    
	dhcp_stop( iface );
    //dns_server_remove_all();
    netif_set_addr(iface, &ipaddr, &netmask, &gw); // it's in the tcpip thread
}


void wifi_disconnected(int err)
{
	wifi_log( "wifi disconnected %d, task name %s, caller %p\r\n", err,
        pcTaskGetTaskName(NULL), __builtin_return_address(0));
	tcpip_callback_with_block(sta_iface_down, 0, 1);
    set_sta_connection(0);
}

static void free_ap_list(void)
{
    mxchip_scan_rst_t* p, *q;

    p = p_result_head;
    p_result_head = NULL;
    
    while(p != NULL) {
        q = p->next;
        free(p);
        p = q;
    }   
}


int mfg_scan(int mode)
{
	mxchip_scan_rst_t *p;
	rtw_scan_result_t* result;
	char str[64];
	
    user_req_scan = MF_SCAN_FLAG;
    free_ap_list();

    if(wifi_scan_networks(app_scan_result_handler, NULL ) != RTW_SUCCESS){
		return 1;
	}

	mos_semphr_acquire(scan_sem, STA_MODE_MAX_SCANTIME);

    if (p_result_head != NULL) {
		mf_printf("Scan AP Success:\r\n");
		p = p_result_head;
	    while (p!=NULL) {
	        result = &p->result;
	        sprintf(str, "  SSID: %s, RSSI: %d\r\n", result->SSID.val, result->signal_strength);
        	mf_printf(str);
			p = p->next;
	    }
	} else {
		mf_printf("Scan AP Fail\r\n");
	}
	
    return 0;
}

extern void ping_Command( int argc, char **argv );
int mfg_connect(char *ssid) // return 0=join success, -1=fail
{
	char str[400];
    char *pcmd[2];
	int ret;
	mwifi_ip_attr_t ip;
    
    ret = wifi_connect(ssid, RTW_SECURITY_OPEN, NULL, strlen(ssid), 
					   0, 0, NULL);
    if (ret != RTW_SUCCESS) {
        mf_printf("AP Connect Fail\r\n");
        return -1;
    } else {
        mf_printf("AP Connect Success\r\n");
    }

	ret = LwIP_DHCP(0, DHCP_START);
    if (DHCP_ADDRESS_ASSIGNED != ret) {
        mf_printf("DHCP Fail\r\n");
		return -1;
    }
	mos_msleep(10);
    get_ip_by_idx(0, &ip);
	sprintf(str, "DHCP Get IP Success, IP address: %s\r\n", ip.localip);
	mf_printf(str);

    pcmd[0] = "ping";
    pcmd[1] = ip.gateway;
    ping_Command( 2, pcmd);
    mf_printf(str);
	return 0;
}

/* start scan and wait scan report. */
static int start_scan(u8 *ssid) 
{
    int ret;

	if (ssid && (ssid[0] == '\0'))
		ssid = NULL;
    if (ssid)
        wifi_log( "start scan(%d) %s\r\n", strlen(ssid), ssid);
    else
        wifi_log("start broadcast scan\r\n");
    free_ap_list();
    ret = mxwifi_scan_networks(app_scan_result_handler, ssid );

	if (ret != RTW_SUCCESS) {
		mos_msleep(sta_retry_interval);
		wifi_log( "!!!! SCAN fail %d!!!!\r\n", ret);
		return ret;
	}
    if (mos_semphr_acquire(scan_sem, STA_MODE_MAX_SCANTIME) != kNoErr) {
		wifi_log( "!!!! SCAN timeout!!!!\r\n");
    }

    wifi_log( "STA state %d, scan ret %d\r\n", sta_state, ret);
    return ret;
}


void mxchip_thread_init(void)
{
    wifimgr_sem = mos_semphr_new(1);
    scan_sem = mos_semphr_new(1);
    ap_list_lock = mos_mutex_new();
    scanlock = mos_mutex_new();
    wifimgr_thread_id = mos_thread_new(5, "wifimgr", wifimgr_thread, 2048, NULL);
}

void scan_lock(void)
{
    mos_mutex_lock(scanlock);
}

void scan_unlock(void)
{
    mos_mutex_unlock(scanlock);
}

static void wifi_reboot_do(void)
{
    wifi_reboot_event(WIFI_ERROR_INIT);
}

void wifi_reboot(void)
{
    wifi_reboot_do();
}

static void wifi_reset(void)
{
	wifi_off();
    mos_msleep(20);
	if (softap_enabled == 1) {
		int keylen, ret;
		
		wifi_on(RTW_MODE_STA_AP);
		keylen = strlen(softap_key);
        if (keylen >= 8 && keylen < 64) {
            ret = wifi_start_ap(softap_ssid, RTW_SECURITY_WPA2_AES_PSK,
						 softap_key, strlen(softap_ssid),
						 keylen,softap_channel);
        } else {
            ret = wifi_start_ap(softap_ssid, RTW_SECURITY_OPEN,
						 NULL, strlen(softap_ssid),
						 0, softap_channel);
        }
        
        if(ret != RTW_SUCCESS) {
            wifi_log( "Start AP Fail return %d\r\n", ret);
        }
	} else {
		wifi_on(RTW_MODE_STA);
	}
}

static char *next_scan_ssid(void)
{
    mos_mutex_lock(ap_list_lock);
    if (try_ap == NULL) {
        try_ap = p_connect_ap;
    } else {
        try_ap = try_ap->next;
    }
    mos_mutex_unlock(ap_list_lock);
    if (try_ap == NULL) {
        wifi_log("next_scan_ssid NULL\r\n");
        return NULL;
    } else {
        wifi_log("next_scan_ssid %p\r\n", try_ap);
        wifi_log("ssid %s\r\n", try_ap->ssid);
        return try_ap->ssid;
    }
}

static void wifimgr_thread(void* arg)
{
    uint32_t timeout = NEVER_TIMEOUT;
    int scan_state;
	
    
    memset(active_scan_ssid, 0, sizeof(active_scan_ssid));
    while(1) {
		scan_unlock();
        scan_state = 0;
		if (uap_connected == 1) { // if softap is started, the minmal retry interval is 60 seconds.
			if (timeout < DUAL_MODE_RETRY_INTERVAL) {
				timeout = DUAL_MODE_RETRY_INTERVAL;
			}
		}
        wifi_log("take sem %p, timeout %d\r\n", wifimgr_sem, timeout);
        mos_semphr_acquire(wifimgr_sem, timeout);
        wifi_log("take sem %p done\r\n", wifimgr_sem);
		scan_lock();
		if ( !wifi_is_up(RTW_STA_INTERFACE) ) {
			if (user_stop_sta == 1) {
                remove_ap_list();
				user_stop_sta = 0;
				sta_state = STOPPED;
			}
			timeout = sta_retry_interval;
			continue;
		}
		
		if (sta_state == ADVTRY) {
			if (select_ap_to_join() != RTW_SUCCESS) { // join fail, retry later.
                sta_state = SLEEPING;
            } else {
                timeout = NEVER_TIMEOUT;
                sta_state = CONNECTED;
                set_sta_connection(1);
				continue;
            }
		}
        if (user_stop_sta == 1) { // user request to disconnect AP
            user_stop_sta = 0;
            wifi_log( "wifimgr stop sta\r\n");
            remove_ap_list();
			wifi_disconnected(0);
			wifi_disconnect();
			sta_state = STOPPED;
            continue;
        }
        if (sta_state == SLEEPING)
            scan_state |= 1;
        if (user_req_scan & USER_SCAN_FLAG)
            scan_state |= 2;
        if (user_req_scan & CMD_SCAN_FLAG)
            scan_state |= 8;

        if (scan_state == 0)
            continue;
        wifi_log("scan_state %d\r\n", scan_state);
        user_req_scan = 0;

        if (scan_state > 1) {
            if (((scan_state & 0x20)==0) && (strlen(active_scan_ssid) > 0)) {
                start_scan(active_scan_ssid);
                memset(active_scan_ssid, 0, sizeof(active_scan_ssid));
            } else
                start_scan(NULL);
        } else {
            wifi_log("no_ap_report_times %d\r\n", no_ap_report_times);
        	if (no_ap_report_times >= MAX_NO_REPORT_TIME) {
				wifi_log( "NOT found any AP too long, reset wifi\r\n");
				wifi_reset();
				no_ap_report_times = 0;
			}
        	if ((no_ap_report_times % 2) == 0) {
            	start_scan(next_scan_ssid());
			} else {
				start_scan(NULL);
			}
        }
       
        if (scan_state & 2) {
            get_scan_result_list();
        } 
        
        wifi_log( "state %d\r\n", sta_state);
        if (sta_state == SLEEPING) {
            sta_state = JOINING;
            if (select_ap_to_join() != RTW_SUCCESS) { // join fail, retry later.
                timeout = sta_retry_interval;
                sta_state = SLEEPING;
            } else {
                timeout = NEVER_TIMEOUT;
                sta_state = CONNECTED;
                set_sta_connection(1);
            }
        }
        free_ap_list();
    }
}

#if 0
static int arp_fail_times = 0;

static void arp_addr_check(void *arg)
{
	ip_addr_t gwip, retgw;
	struct eth_addr *eth;
	struct netif *netif;
	struct ipv4_config myaddr;
	
	net_get_if_addr(&myaddr, NET_IF_STA);
    gwip.addr = (myaddr.gw);
	netif = net_get_interface(NET_IF_STA);
	if (etharp_find_addr(netif, &gwip, &eth, &retgw) < 0) {
        wifi_log( "healMon query gateway %x %d\r\n", gwip.addr, arp_fail_times);
        etharp_query(netif, &gwip, NULL);
        arp_fail_times++;
    } else {
		arp_fail_times = 0;
	}
}

static void health_monitor_thread(void* arg)
{
    int sleep_time = 10;

#define MAX_RETRY_TIMES 10

    while(1) {
		os_ticker2();
        sleep(sleep_time);
        if (sta_connected == 0) {
            sleep_time = 60;
            arp_fail_times = 0;
            continue;
        }

        tcpip_callback_with_block(arp_addr_check, NULL, 1);
		if (arp_fail_times == 0) {
			sleep_time = 60;
    	} else {
			sleep_time = 10;
		}
		if (arp_fail_times >= MAX_RETRY_TIMES) {
            wifi_reboot_event(WIFI_ERROR_NOGW);
            arp_fail_times = 0;
        } 
    }
}

static void softap_tdma_resume_therad(void* arg)
{
    int client_idx = 0;
	struct {
		int    count;
		rtw_mac_t mac_list[AP_STA_NUM];
    } client_info;
    
    while(1)
    {
        mos_semphr_acquire(softap_disassoc_semphr, MICO_WAIT_FOREVER);
        // printf("--> get softap_disassoc_semphr\r\n");
        memset(&client_info, 0, sizeof(client_info));
        client_info.count = AP_STA_NUM;
        wifi_get_associated_client_list(&client_info, sizeof(client_info)); 
        // printf("--> get client_info.count = %lu\r\n", client_info.count);
        if(client_info.count == 0 && tdma_resume > 0)
        {
            // printf("--> set TDMA = %lu\r\n", tdma_resume);
            wlan_set_softap_tdma(tdma_resume);
            tdma_resume = 0;
        }
    }
}
#endif
int station_dhcp_mode (void)
{
    return sta_dhcp_mode;
}

void set_wifi_stop_state(void)
{
	sta_state = STOPPED;
}

/*-----------------------------------------------------------*/
int wifistate_Command( int argc, char **argv )
{
    int xReturn;
    int len, dBm, channel;
    mwifi_link_info_t info;
    connect_ap_list_t * ap_info = p_connect_ap;

    while(ap_info) {
        mcli_printf("Connect AP info: \r\n\tSSID: %s\r\n\tkey: %s(%d)\r\n\tretry interval %d\r\n\r\n", 
        ap_info->ssid, ap_info->key, ap_info->key_len, sta_retry_interval);
    }
    switch(sta_state) {
    case STOPPED:
        mcli_printf("Station is STOPPED\r\n");
        break;
    case SLEEPING:
        mcli_printf("Station is sleeping\r\n");
        break;
    case SCANING:
        mcli_printf("Station is scanning for AP\r\n");
        break;
    case JOINING:
        mcli_printf("Station is trying to join AP\r\n");
        break;
    case CONNECTED:
        mcli_printf("Station is CONNECTED\r\n");
		mwifi_get_link_info(&info);
        mcli_printf("Connected AP info: \r\n\tSSID: %s\r\n\tChannel: %d \r\n\tRSSI: %d\r\n", 
            info.ssid, info.channel, info.rssi);
        mcli_printf("\tBSSID: %02x-%02x-%02x-%02x-%02x-%02x\r\n", 
            info.bssid[0],info.bssid[1],info.bssid[2],
            info.bssid[3],info.bssid[4],info.bssid[5]);
        break;
    default:
        mcli_printf("Unknown WIFI state %d\r\n", sta_state);
        break;
    }
	
	return xReturn;
}

void driver_state_Command( int argc, char **argv )
{
    mcli_printf("TODO\r\n");
}

void wifidebug_Command( int argc, char **argv )
{
	if (!strcasecmp(argv[1], "on")) {
        wifimgr_debug_enable(1);
		mcli_printf("Enable wifidebug\r\n");
	} else if (!strcasecmp(argv[1], "off")) {
    	wifimgr_debug_enable(0);
		mcli_printf("Disable wifidebug\r\n");
	}
}

void wifiscan_Command( int argc, char **argv )
{
    mxchip_scan_rst_t *list;
    rtw_scan_result_t *p;
    int i = 0;
	uint32_t curtime;
    
    mcli_printf("Waiting for scan results...\r\n");
    user_req_scan |= CMD_SCAN_FLAG;
    
    mos_semphr_release(wifimgr_sem);
	curtime = sys_now();
    mos_semphr_acquire(scan_sem, STA_MODE_MAX_SCANTIME); 
	curtime = sys_now() - curtime;
	mcli_printf("wait %d ms\r\n", curtime);
    list = p_result_head;
    if (list != NULL) {
        p = &list->result;
        mcli_printf("  # Type  BSSID             RSSI  Chan Security    SSID\r\n");
        mcli_printf("------------------------------------------------------------\r\n");
    } else
        mcli_printf("Scan Failed\r\n");
    while(list != NULL) {
        p = &list->result;
        mcli_printf( "%3d %5s ", i++, "Infra" );
        mcli_printf( "%02X:%02X:%02X:%02X:%02X:%02X ", p->BSSID.octet[0], p->BSSID.octet[1], p->BSSID.octet[2], 
                                                      p->BSSID.octet[3], p->BSSID.octet[4], p->BSSID.octet[5] );
        mcli_printf( " %d ", p->signal_strength );
        mcli_printf( " %2d  ", p->channel );
        mcli_printf( "%-10s ", ( p->security== RTW_SECURITY_OPEN ) ? "Open" :
                              ( p->security & WEP_ENABLED ) ? "WEP" :
                              ( p->security & (WPA_SECURITY|WPA2_SECURITY)) ? "WPA PSK" :
                              "Unknown" );
        mcli_printf( " %-32s ", p->SSID.val );
        mcli_printf( "\r\n" );
        list = list->next;
    }
}

ip_addr_t dns_getserver(u8_t numdns);

void ifconfig_Command( int argc, char **argv )
{
    ip_addr_t addr;
    mwifi_ip_attr_t ip;

    if (sta_connected) {
        get_ip_by_idx(0, &ip);
        mcli_printf("Station Interface: \r\n    IP Addr: %s\r\n", ip.localip);
        mcli_printf("    Netmask: %s\r\n", ip.netmask);
        mcli_printf("    Gateway: %s\r\n", ip.gateway);
        mcli_printf("    DNS   : %s\r\n", ip.dnserver);
    } 

    if (uap_connected) {
        get_ip_by_idx(1, &ip);
        mcli_printf("SoftAP Interface: \r\n    IP Addr: %s\r\n", ip.localip);
        mcli_printf("    Netmask: %s\r\n", ip.netmask);
        mcli_printf("    Gateway: %s\r\n", ip.gateway);
    }
}

void arp_display( int argc, char *argv[]);
void arp_clean(void);
void arp_Command( int argc, char **argv )
{
    if (argc != 2) {
        mcli_printf("Usage: arp show/clean.\r\n");
        return;
    }

	if (!strcasecmp(argv[1], "show")) {
		arp_display(argc, argv);
	} else if (!strcasecmp(argv[1], "clean")) {
		arp_clean();
        mcli_printf("ARP Clean Done\r\n");
	} else {
        mcli_printf("Usage: arp show/clean.\r\n");
    }
}
void ping_Command( int argc, char **argv )
{
    uint32_t  ping_target_ip;
    uint32_t ip;
    int status;
    uint32_t elapsed_ms;
    int i;
    
    if (argc != 2) {
        mcli_printf("Usage: ping <IP>.\r\n");
        return;
    }


    do_ping_test(argv[1], 64, 5, 1);
}

void dns_show(int argc, char *argv[]);
void dns_clean(void);
//dns show/get/clean
void dns_Command( int argc, char **argv )
{
    struct hostent* hostent_content = NULL;
	struct in_addr in_addr;
	
    if (argc != 2) {
        mcli_printf("Usage: dns show/clean/<domain_name>.\r\n");
        return;
    }

    if (!strcasecmp(argv[1], "show")) {
		dns_show(argc, argv);
	} else if (!strcasecmp(argv[1], "clean")) {
		dns_clean();
        mcli_printf("DNS Clean Done\r\n");
	} else {
		hostent_content = gethostbyname( argv[1] );
	    if (hostent_content == NULL) {
	        mcli_printf("Can't %s's IP address\r\n", argv[1]);
	        return;
	    }
        memcpy(&in_addr, hostent_content->h_addr, 4);
	    mcli_printf("%s's IP address is %s\r\n", argv[1], inet_ntoa(in_addr));
    }
}

void tcp_dump(int argc, char *argv[]);
void udp_dump(int argc, char *argv[]);
void socket_show(int argc, char *argv[]);
void socket_show_Command( int argc, char **argv )
{
    socket_show(argc, argv);
    tcp_dump(argc, argv);
    udp_dump(argc, argv);
}

void memory_show_Command( char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv )
{
    mcli_printf("number of chunks %d\r\n", vPortGetBlocks());
    mcli_printf("total memory %d\r\n", xPortGetTotalHeapSize());
    mcli_printf("free memory %d\r\n", xPortGetFreeHeapSize());
    mcli_printf("minmum free memory %d\r\n", xPortGetMinimumEverFreeHeapSize());
}

void memory_dump_Command( int argc, char **argv )
{
    int i;
    uint8_t *pstart;
    uint32_t start, length;
    
    if (argc != 3) {
        mcli_printf("Usage: memdump <addr> <length>.\r\n");
        return;
    }

    start = strtoul(argv[1], NULL, 0);
    length = strtoul(argv[2], NULL, 0);
    pstart = (uint8_t*)start;
    
    for(i=0; i<length;i++) {
        mcli_printf("%02x ", pstart[i]);
        if (i % 0x10 == 0x0F) {
            mcli_printf("\r\n");
            
        }
    }
}

void memory_set_Command( int argc, char **argv )
{
    uint8_t *pstart, value;
    uint32_t start;
    int i;
    
    if (argc < 3) {
        mcli_printf("Usage: memset <addr> <value 1> [<value 2> ... <value n>].\r\n");
        return;
    }
    start = strtoul(argv[1], NULL, 0);
    value = strtoul(argv[2], NULL, 0);
    pstart = (uint8_t*)start;
    *pstart = value;
    mcli_printf("Set 0x%08x %d bytes\r\n", start, argc-2);
    for(i=2;i<argc;i++) {
        value = strtoul(argv[i], NULL, 0);
        pstart[i-2] = value;
    }
}


int net_get_sock_error(int sock)
{
    int errno, optlen = 4;
    
    lwip_getsockopt(sock, 0xfff, 0x1007, &errno, &optlen);

    return errno;
}

typedef void (*mgnt_handler_t)(char *buf, int buf_len, int flags, void* handler_user_data );

int wlan_rx_mgnt_set(int enable, mgnt_handler_t cb)
{
    if ( !wifi_is_up(RTW_STA_INTERFACE) ) {
        return -1;
    }
    
	if (enable) {
		wifi_set_indicate_mgnt(1);
		wifi_reg_event_handler(WIFI_EVENT_RX_MGNT, cb,NULL);
	} else {
		wifi_set_indicate_mgnt(0);
	}

	return 0;
}

int mico_send_minus(char *ssid, int len)
{
	return 0;// TODO
}


int wlan_get_mac_address(unsigned char *dest)
{
	mwifi_get_mac(dest);
	return 0;
}


int wlan_inject_frame(const uint8_t *buff, size_t len)
{
  return wext_send_mgnt(WLAN0_NAME, (char*)buff, (__u16)len, 1);
}

/* new main code */
void wifi_driver_init(void)
{
    wlan_network();
    mxchip_thread_init();
}

merr_t	mwifi_softap_start(const char *ssid, char *key, int channel, mwifi_ip_attr_t *attr)
{
    int keylen, ret, timeout=20;
    static_ip_t ip;
    char essid[33];

    if ((channel >= 1) && (channel <= 13))
        softap_channel = channel;
    keylen = strlen(key);
    wifi_log("start softap %s-%s\r\n", ssid, key);
    if (wifi_mode != RTW_MODE_AP) {
        wifi_set_mode(RTW_MODE_AP);
    }
    scan_lock();
    if (keylen >= 8 && keylen < 64) {
        ret = wifi_start_ap(ssid, RTW_SECURITY_WPA2_AES_PSK,
					 key, strlen(ssid),
					 keylen, softap_channel);
    } else {
        ret = wifi_start_ap(ssid, RTW_SECURITY_OPEN,
					 NULL, strlen(ssid),
					 0, softap_channel);
    }
    scan_unlock();
    set_static_ip(&ip, attr);
    set_netif_static_ip(1, &ip);
    while(timeout > 0) {
		if(wext_get_ssid(WLAN0_NAME, (unsigned char *) essid) > 0) {
			if(strcmp((const char *) essid, (const char *)ssid) == 0) {
				break;
			}
		}

		mos_msleep(1);
		timeout --;
    }
    if(timeout == 0) {
		wifi_log("ERROR: Start AP timeout!\r\n");
		return -1;
    }
    softap_enabled = 1;
	strncpy(softap_ssid, ssid, 32);
	strncpy(softap_key, key, 64);
    dhcps_init(&xnetif[0]);
    set_uap_connection(1);

    return kNoErr;
}

merr_t	mwifi_softap_stop(void)
{
    uap_stop();
    return kNoErr;
}
