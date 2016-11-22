#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "mem.h"
#include "user_config.h"
#include "user_interface.h"

#define user_procTaskPrio        0
#define user_procTaskQueueLen    1
os_event_t    user_procTaskQueue[user_procTaskQueueLen];
static volatile os_timer_t deauth_timer;

// Channel to perform deauth
uint8_t channel = 1;

// Access point MAC to deauth
uint8_t ap[6] = {0x00,0x01,0x02,0x03,0x04,0x05};

// Client MAC to deauth
uint8_t client[6] = {0x06,0x07,0x08,0x09,0x0A,0x0B};

// Sequence number of a packet from AP to client
uint16_t seq_n = 0;

// Packet buffer
uint8_t packet_buffer[64];

/* ==============================================
 * Promiscous callback structures, see ESP manual
 * ============================================== */

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


int rssi = -1000;
/* Listens communication between AP and client */
  static void ICACHE_FLASH_ATTR
promisc_cb(uint8_t *buf, uint16_t len)
{
  if (len == 12){
    struct RxControl *sniffer = (struct RxControl*) buf;
    if(sniffer->rssi > rssi) rssi = sniffer->rssi;
  } else if (len == 128) {
    struct sniffer_buf2 *sniffer = (struct sniffer_buf2*) buf;
    if(sniffer->rx_ctrl.rssi > rssi) rssi = sniffer->rx_ctrl.rssi;
  } else {
    struct sniffer_buf *sniffer = (struct sniffer_buf*) buf;
    if(sniffer->rx_ctrl.rssi > rssi) rssi = sniffer->rx_ctrl.rssi;
    int i=0;
    // Check MACs
    for (i=0; i<6; i++) if (sniffer->buf[i+4] != client[i]) return;
    for (i=0; i<6; i++) if (sniffer->buf[i+10] != ap[i]) return;
    // Update sequence number
    seq_n = sniffer->buf[23] * 0xFF + sniffer->buf[22];
  }
  os_printf("Max rssi: %d\n", rssi);
}

  void ICACHE_FLASH_ATTR
sniffer_system_init_done(void)
{
  // Set up promiscuous callback
  wifi_set_channel(channel);
  wifi_promiscuous_enable(0);
  wifi_set_promiscuous_rx_cb(promisc_cb);
  wifi_promiscuous_enable(1);
}

  void ICACHE_FLASH_ATTR
user_init()
{
  uart_div_modify(0, UART_CLK_FREQ / 115200);
  os_printf("\n\nSDK version:%s\n", system_get_sdk_version());

  // Promiscuous works only with station mode
  wifi_set_opmode(STATION_MODE);

  // Continue to 'sniffer_system_init_done'
  system_init_done_cb(sniffer_system_init_done);
  os_printf("Done with init\n");
}
