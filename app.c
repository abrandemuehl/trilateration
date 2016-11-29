#include "osapi.h"
#include "ets_sys.h"
#include "user_config.h"
#include "user_interface.h"
#include "network_80211.h"
#include "mem.h"

#include "uart.h"
#include "messages.h"
#include "ringbuffer.h"

#define CHANNEL 8

#define MAC_CMP(a, b) \
  (a[0] == b[0] && a[1] == b[1] && a[2] == b[2] && \
   a[3] == b[3] && a[4] == b[4] && a[5] == b[5])




#define message_procTaskPrio        2
#define message_procTaskQueueLen    2
os_event_t message_procTaskQueue[message_procTaskQueueLen];
static void message_procTask(os_event_t *events);



struct entry {
  uint8_t mac[6];
  int8_t rssi;
  uint32_t time;
};

struct entry entries[100];
size_t end = 0;

RingBuffer updates;

void ICACHE_FLASH_ATTR
promisc_cb(uint8_t *buf, uint16_t len)
{
  if (len < 64) {
    return;
  }
  struct sniffer_buf *sniffer = (struct sniffer_buf*)buf;
  struct framectrl_80211 *fc = (struct framectrl_80211*)sniffer->buf;
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
    }

    // Put the new position into the ringbuffer as been updated
    rb_push(&updates, ptr);
  }
}



/* message messages[255]; */
/* RingBuffer message_q; */
/* int message_index = 0; */

/* message msg; */
/* uint8_t *msg_next = &msg; */
/* uint8_t *msg_end = (&msg)+1; */
/* int msg_done = 0; */


// Send updates over uart
void ICACHE_FLASH_ATTR
message_procTask(os_event_t *event) {
  os_printf("Running loop\n");
  message m;
  int idx = rb_get(&updates);
  while(idx != -1) {
    message_create(&m, system_get_chip_id(), entries[idx].mac, entries[idx].rssi);
    // uart0_tx_buffer((uint8_t *)&m, sizeof(m));
  }
  os_delay_us(1000);
  system_os_post(message_procTaskPrio, 0, 0);
}

void ICACHE_FLASH_ATTR
sniffer_init(void)
{
  // Set up promiscuous callback
  wifi_set_channel(CHANNEL);
  wifi_promiscuous_enable(0);
  wifi_set_promiscuous_rx_cb(promisc_cb);
  wifi_promiscuous_enable(1);
  os_memset(entries, 0, sizeof(entries));
  rb_init(&updates);
}


  void ICACHE_FLASH_ATTR
user_init()
{
  uart_div_modify(0, UART_CLK_FREQ / 115200);
  /* uart_init(BIT_RATE_115200, BIT_RATE_115200); */

  // Promiscuous works only with station mode
  wifi_set_opmode(STATION_MODE);

  system_init_done_cb(sniffer_init);

  // rb_init(&messages);

  uint8_t comm_start[100] = {0xFF};
  comm_start[99] = 0;

  system_os_task(message_procTask, message_procTaskPrio, message_procTaskQueue, message_procTaskQueueLen);
  system_os_post(message_procTaskPrio, 0, 0);
}
