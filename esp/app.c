#include "osapi.h"
#include "ets_sys.h"
#include "user_config.h"
#include "user_interface.h"
#include "network_80211.h"
#include "mem.h"

#include "uart.h"
#include "../common/messages.h"
#include "ringbuffer.h"

#define CHANNEL 1

#define MAC_CMP(a, b) \
  (a[0] == b[0] && a[1] == b[1] && a[2] == b[2] && \
   a[3] == b[3] && a[4] == b[4] && a[5] == b[5])

char martin[6] = {0x40, 0x78, 0x6a, 0x6d, 0x12, 0x4d};




#define message_procTaskPrio        2
#define message_procTaskQueueLen    2
os_event_t message_procTaskQueue[message_procTaskQueueLen];
static void message_procTask(os_event_t *events);

#define uart_procTaskPrio        3
#define uart_procTaskQueueLen    64
os_event_t uart_procTaskQueue[uart_procTaskQueueLen];
static void uart_procTask(os_event_t *events);

#define PRINT_MAC(mac) \
    os_printf("%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);


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
  os_printf("Recv\n");
  if(fc->Type == FRAME_TYPE_DATA) {
    struct data_frame_80211 *data_frame = (struct data_frame_80211*)sniffer->buf;

    int8_t rssi = sniffer->rx_ctrl.rssi;
    uint32_t time = system_get_time();
    if(MAC_CMP(martin, data_frame->sa)) {
      // Found. Update value
      os_printf("%d\n", rssi);
    }
  }
}



// Send updates over uart
void ICACHE_FLASH_ATTR
message_procTask(os_event_t *event) {
  message m;

  int idx = rb_get(&updates);
  while(idx != -1) {
    /* message_create(&m, system_get_chip_id(), entries[idx].mac, entries[idx].rssi); */
    /* uart0_tx_buffer((uint8_t *)&m, sizeof(m)); */

    idx = rb_get(&updates);

    os_printf("%d\n", entries[idx].rssi);
    //uart0_tx_buffer((uint8_t *)&m, sizeof(m));
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
}


message msg;
static void uart_procTask(os_event_t *events) {
}


  void ICACHE_FLASH_ATTR
user_init()
{
  uart_div_modify(0, UART_CLK_FREQ / 115200);
  /* uart_init(BIT_RATE_115200, 0); */

  // Promiscuous works only with station mode
  wifi_set_opmode(STATION_MODE);

  os_memset(entries, 0, sizeof(entries));
  rb_init(&updates);
  system_init_done_cb(sniffer_init);



  // rb_init(&messages);

  /* system_os_task(message_procTask, message_procTaskPrio, message_procTaskQueue, message_procTaskQueueLen); */
  /* system_os_post(message_procTaskPrio, 0, 0); */
  /* system_os_task(uart_procTask, uart_procTaskPrio, uart_procTaskQueue, uart_procTaskQueueLen); */
}
