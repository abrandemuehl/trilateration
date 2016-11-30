#include "messages.h"

#ifdef __xtensa__
#include "osapi.h"
#else
#include <string.h>
#endif



void ICACHE_FLASH_ATTR message_create(message *msg, uint32_t id, uint8_t *mac, uint8_t rssi) {
  msg->head.PREFIX0 = 'f';
  msg->head.PREFIX1 = 'd';
  msg->head.PREFIX2 = 'a';
  msg->src_id = id;
#ifdef __xtensa__
  os_memcpy(msg->mac, mac, sizeof(msg->mac));
#else
  memcpy(msg->mac, mac, sizeof(msg->mac));
#endif
  msg->rssi = rssi;

}
