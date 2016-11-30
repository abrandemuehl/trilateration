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
#define NUM_NODES 3

#define MAC_CMP(a, b) \
  (a[0] == b[0] && a[1] == b[1] && a[2] == b[2] && \
   a[3] == b[3] && a[4] == b[4] && a[5] == b[5])



// #define uart_procTaskPrio        2
#define uart_procTaskQueueLen    2
os_event_t uart_procTaskQueue[uart_procTaskQueueLen];
static void uart_procTask(os_event_t *event);

#define message_procTaskPrio        1
#define message_procTaskQueueLen    2
os_event_t message_procTaskQueue[message_procTaskQueueLen];
static void message_procTask(os_event_t *event);


#define process_procTaskPrio        0
#define process_procTaskQueueLen    64
os_event_t process_procTaskQueue[process_procTaskQueueLen];
static void process_procTask(os_event_t *event);


#define PRINT_MAC(mac) \
    os_printf("%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

// Nodes x, y and z position
float xloc, yloc, zloc;

uint32_t chip_ids[NUM_NODES];
int num_chips = 1;

struct entry {
  uint8_t mac[6];
  int8_t rssi[NUM_NODES];
  uint32_t time;
};

struct entry entries[100];
size_t entries_end = 0;

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
    struct data_frame_80211 *data_frame = (struct data_frame_80211*)sniffer->buf;

    int8_t rssi = sniffer->rx_ctrl.rssi;
    uint32_t time = system_get_time();
    // Search for the mac entry
    int ptr;
    for(ptr = 0; ptr < entries_end; ptr++) {
      if(MAC_CMP(entries[ptr].mac, data_frame->sa)) {
        // Found. Update value
        entries[ptr].time = time;
        entries[ptr].rssi[0] = rssi;
        break;
      }
    }
    if(ptr == entries_end) {
      // Insert a new entry
      os_memcpy(entries[ptr].mac, data_frame->sa, sizeof(entries[ptr].mac));
      entries[ptr].rssi[0] = rssi;
      entries[ptr].time = time;
      entries_end++;
    }

    // Put the new position into the ringbuffer as been updated
    rb_push(&updates, ptr);
    system_os_post(message_procTaskPrio, 0, 0);
  }
}



// Send updates over uart
void ICACHE_FLASH_ATTR
message_procTask(os_event_t *event) {
  message m;

  int idx = rb_get(&updates);
  message_create(&m, system_get_chip_id(), entries[idx].mac, entries[idx].rssi[0]);
  uart0_tx_buffer((uint8_t *)&m, sizeof(m));

  idx = rb_get(&updates);
}

void ICACHE_FLASH_ATTR
sniffer_init(void)
{
  // Set up promiscuous callback
  wifi_set_channel(CHANNEL);
  wifi_promiscuous_enable(0);
  wifi_set_promiscuous_rx_cb(promisc_cb);
  wifi_promiscuous_enable(1);
  os_printf("Started\n");
}


message *msg = NULL;
int uart_index = 0;
static void uart_procTask(os_event_t *event) {
  return;
  if(msg == NULL) {
    msg = (message *)os_malloc(sizeof(message));
  }
  uint8_t *buf = (uint8_t *)msg;
  buf[uart_index] = event->par;
  switch (uart_index) {
    case 0:
      if(buf[uart_index] == 'f') {
        uart_index++;
      }
      break;
    case 1:
      if(buf[uart_index] == 'd') {
        uart_index++;
      }
      break;
    case 2:
      if(buf[uart_index] == 'a') {
        uart_index++;
      }
      break;
    default:
      if(++uart_index >= sizeof(message)) {
        uart_index = 0;
        // Copy the message into the ring buffer
        system_os_post(process_procTaskPrio, 0, (os_param_t)msg);
        msg = (message *)os_malloc(sizeof(message));
      }
      break;
  }
}

static void process_procTask(os_event_t *event) {
  message *m = (message *)event->par;

  // Find chip_id
  int i;
  int chip_idx = -1;
  for(i=0; i < num_chips; i++) {
    if(chip_ids[i] == m->src_id) {
      chip_idx = i;
      break;
    }
  }
  if(chip_idx == -1) {
    if(num_chips == 3) {
      // No more chips left. Return
      return;
    }
    // Add chip to the chip list
    chip_ids[num_chips] = m->src_id;
  }

  for(i = 0; i < entries_end; i++) {
    if(MAC_CMP(entries[i].mac, m->mac)) {
      // Found. Update value
      entries[i].rssi[chip_idx] = m->rssi;
      break;
    }
  }
  if(i == entries_end) {
    // Insert a new entry
    os_memcpy(entries[i].mac, m->mac, sizeof(entries[i].mac));
    entries[i].rssi[chip_idx] = m->rssi;
    entries_end++;
  }
  os_free(m);
}

  void ICACHE_FLASH_ATTR
user_init()
{
  /* uart_div_modify(0, UART_CLK_FREQ / 115200); */
  uart_init(BIT_RATE_115200,BIT_RATE_115200);

  // Promiscuous works only with station mode
  wifi_set_opmode(STATION_MODE);

  os_memset(entries, 0, sizeof(entries));
  rb_init(&updates);
  system_init_done_cb(sniffer_init);

  chip_ids[0] = system_get_chip_id();

  // rb_init(&messages);

  uint8_t comm_start[100] = {0xFF};
  comm_start[99] = 0;

  system_os_task(message_procTask, message_procTaskPrio, message_procTaskQueue, message_procTaskQueueLen);
  system_os_task(uart_procTask, uart_procTaskPrio, uart_procTaskQueue, uart_procTaskQueueLen);


  system_os_task(process_procTask, process_procTaskPrio, process_procTaskQueue, process_procTaskQueueLen);
  os_printf("Starting\n");
}
