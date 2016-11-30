#ifndef MESSAGES_H
#define MESSAGES_H

#ifdef __xtensa__
#include "c_types.h"
#else
#include <stdint.h>
#endif


typedef struct {
  uint8_t PREFIX0; // 'f'
  uint8_t PREFIX1; // 'd'
  uint8_t PREFIX2; // 'a'
} header;
typedef struct {
  header head;
  uint32_t src_id;
  uint8_t mac[6];
  uint8_t rssi;
} message;


void message_create(message *msg, uint32_t id, uint8_t *mac, uint8_t rssi);

#endif // MESSAGES_H
