#ifndef NETWORK_80211_H
#define NETWORK_80211_H
#include "c_types.h"

#define FRAME_TYPE_MANAGEMENT 0
#define FRAME_TYPE_CONTROL 1
#define FRAME_TYPE_DATA 2
#define FRAME_SUBTYPE_PROBE_REQUEST 0x04
#define FRAME_SUBTYPE_PROBE_RESPONSE 0x05
#define FRAME_SUBTYPE_BEACON 0x08
#define FRAME_SUBTYPE_AUTH 0x0b
#define FRAME_SUBTYPE_DEAUTH 0x0c
#define FRAME_SUBTYPE_DATA 0x14

typedef struct framectrl_80211
{
    //buf[0]
    uint8_t Protocol:2;
    uint8_t Type:2;
    uint8_t Subtype:4;
    //buf[1]
    uint8_t ToDS:1;
    uint8_t FromDS:1;
    uint8_t MoreFlag:1;
    uint8_t Retry:1;
    uint8_t PwrMgmt:1;
    uint8_t MoreData:1;
    uint8_t Protectedframe:1;
    uint8_t Order:1;
} framectrl_80211,*lpframectrl_80211;

typedef struct probe_request_80211
{
	struct framectrl_80211 framectrl;
	uint16_t duration;
	uint8_t rdaddr[6];
	uint8_t tsaddr[6];
	uint8_t bssid[6];
	uint16_t number;
} probe_request, *pprobe_request;

typedef struct tagged_parameter
{
	/* SSID parameter */
	uint8_t tag_number;
	uint8_t tag_length;
} tagged_parameter, *ptagged_parameter;

struct data_frame_80211 {
  struct framectrl_80211 framectrl;
  uint16_t duration;
  uint8_t da[6]; // destination addr
  uint8_t sa[6]; // source addr
  uint8_t bssid[6];
  uint16_t seq_ctrl;
};

struct RxControl {
    signed rssi:8;
    unsigned rate:4;
    unsigned is_group:1;
    unsigned:1;
    unsigned sig_mode:2;
    unsigned legacy_length:12;
    unsigned damatch0:1;
    unsigned damatch1:1;
    unsigned bssidmatch0:1;
    unsigned bssidmatch1:1;
    unsigned MCS:7;
    unsigned CWB:1;
    unsigned HT_length:16;
    unsigned Smoothing:1;
    unsigned Not_Sounding:1;
    unsigned:1;
    unsigned Aggregation:1;
    unsigned STBC:2;
    unsigned FEC_CODING:1;
    unsigned SGI:1;
    unsigned rxend_state:8;
    unsigned ampdu_cnt:8;
    unsigned channel:4;
    unsigned:12;
};

struct sniffer_buf{
	struct RxControl rx_ctrl;
	uint8_t buf[48];
	uint16_t cnt;
	uint16_t len[1];
};

#endif // NETWORK_80211_H
