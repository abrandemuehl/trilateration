#include "sniffer.h"
#include <Arduino.h>

extern "C" {
#include "user_interface.h"
}

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

struct LenSeq {
  uint16_t length;
  uint16_t seq;
  uint8_t  address3[6];
};

struct sniffer_buf {
  struct RxControl rx_ctrl;
  uint8_t buf[36];
  uint16_t cnt;
  struct LenSeq lenseq[1];
};

struct sniffer_buf2{
  struct RxControl rx_ctrl;
  uint8_t buf[112];
  uint16_t cnt;
  uint16_t len;
};



/* Listens communication between AP and client */
void promisc_cb(uint8_t *buf, uint16_t len)
{
  int rssi;
  if (len == 12){
    struct RxControl *sniffer = (struct RxControl*) buf;
    rssi = sniffer->rssi;
  } else if (len == 128) {
    struct sniffer_buf2 *sniffer = (struct sniffer_buf2*) buf;
    rssi = sniffer->rx_ctrl.rssi;
  } else {
    struct sniffer_buf *sniffer = (struct sniffer_buf*) buf;
    rssi = sniffer->rx_ctrl.rssi;
  }
  Serial.print("Len:");
  Serial.print(len,DEC);
  Serial.print(" RSSI:");
  Serial.print(rssi, DEC);
  Serial.print("\n");
}

void sniffer_init(void)
{
  // Set up promiscuous callback
  wifi_set_channel(1);
  wifi_promiscuous_enable(0);
  wifi_set_promiscuous_rx_cb(promisc_cb);
  wifi_promiscuous_enable(1);
  Serial.println("Enabled Sniffer");
}
