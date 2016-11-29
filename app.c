#include "osapi.h"
#include "ets_sys.h"
#include "user_config.h"
#include "user_interface.h"
#include "network_80211.h"

#define MAC_CMP(a, b) \
  (a[0] == b[0] && a[1] == b[1] && a[2] == b[2] && \
   a[3] == b[3] && a[4] == b[4] && a[5] == b[5])


struct entry {
  uint8_t mac[6];
  int8_t rssi;
  uint32_t time;
};

struct entry entries[100];
size_t end = 0;

int updates[255];

void ICACHE_FLASH_ATTR
promisc_cb(uint8_t *buf, uint16_t len)
{
  if (len < 64) {
    return;
  }
  struct sniffer_buf *sniffer = (struct sniffer_buf*)buf;
  buf += sizeof(struct RxControl);
  struct framectrl_80211 *fc = (struct framectrl_80211*)buf;
  if(fc->Type == FRAME_TYPE_DATA) {
    struct data_frame_80211 *data_frame = (struct data_frame_80211*)buf;
    int8_t rssi = sniffer->rx_ctrl.rssi;
    uint32_t time = system_get_time();
    // Search for the mac entry
    int ptr;
    for(ptr = 0; ptr < end; ptr++) {
      if(MAC_CMP(entries[ptr].mac, data_frame->sa)) {
        // Found. Update value
        entries[ptr].time = time;
        if(entries[ptr].rssi != rssi) {
          entries[ptr].rssi = rssi;
          os_printf("Updating %02x:%02x:%02x:%02x:%02x:%02x -> ",
              data_frame->sa[0], data_frame->sa[1],
              data_frame->sa[2], data_frame->sa[3],
              data_frame->sa[4], data_frame->sa[5]);
          os_printf("RSSI: %x %d\n", rssi, rssi);
        }
        break;
      }
    }
    if(ptr == end) {
      // Insert a new entry
      os_memcpy(entries[end].mac, data_frame->sa, sizeof(entries[end].mac));
      entries[end].rssi = rssi;
      entries[ptr].time = time;
      end++;

      os_printf("Adding %02x:%02x:%02x:%02x:%02x:%02x -> ",
          data_frame->sa[0], data_frame->sa[1],
          data_frame->sa[2], data_frame->sa[3],
          data_frame->sa[4], data_frame->sa[5]);
      os_printf("RSSI: %x %d\n", rssi, rssi);
    }
  }
}


void loop() {

}

void ICACHE_FLASH_ATTR
sniffer_init(void)
{
  // Set up promiscuous callback
  wifi_set_channel(1);
  wifi_promiscuous_enable(0);
  wifi_set_promiscuous_rx_cb(promisc_cb);
  wifi_promiscuous_enable(1);
  os_printf("Done initializing\n");
  os_memset(entries, 0, sizeof(entries));
}


  void ICACHE_FLASH_ATTR
user_init()
{
  uart_div_modify(0, UART_CLK_FREQ / 115200);

  // Promiscuous works only with station mode
  wifi_set_opmode(STATION_MODE);

  system_init_done_cb(sniffer_init);

  loop();
}
