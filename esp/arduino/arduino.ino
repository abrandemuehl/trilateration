#include <Arduino.h>
#include "sniffer.h"

extern "C" {
#include <user_interface.h>
}

#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "password"
#define   MESH_PORT       5555

void setup() {
  Serial.begin(115200);
  wifi_set_opmode(STATION_MODE);
  sniffer_init();
}

void loop() {
}

