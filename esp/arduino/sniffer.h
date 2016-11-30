#ifndef SNIFFER_H
#define SNIFFER_H
#include "c_types.h"



void promisc_cb(uint8_t *buf, uint16_t len);
// Must be called in the system_init_done_cb
void sniffer_init(void);

#endif // SNIFFER_H
