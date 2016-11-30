#ifndef CLIENTS_H
#define CLIENTS_H
#include <stdint.h>

extern "C" {
#include <user_interface.h>
}

#define MAX_SOFTAP_CLIENTS 5


// Keeps track of status of every item
struct Client {
  Client *next;

  uint8_t mac[6];
  struct ip_addr ip;
  int rssi;
};

class ClientsManager {
  Client _clients[MAX_SOFTAP_CLIENTS];
  Client *clients;
};

#endif // CLIENTS_H
