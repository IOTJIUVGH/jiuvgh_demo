#ifndef __WIFI_MGR_H
#define __WIFI_MGR_H

enum {
    WIFI_STATE_STA_UP = 1,
    WIFI_STATE_STA_DOWN,
    WIFI_STATE_UAP_UP,
    WIFI_STATE_UAP_DOWN,
};

enum {
    ENC_OPEN,
    ENC_WEP,
    ENC_CCMP,
    ENC_TKIP,
};

#endif

