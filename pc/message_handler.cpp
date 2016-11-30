#include "message_handler.h"
#include <map>
#include <cstdint>
#include <string>
#include "types.h"
extern "C" {
#include "../common/trilateration.h"
}

#define NUM_NODES 3
struct Device {
  // Maps node id to rssi value
  std::map<uint32_t, uint8_t> rssi;
};

struct Node {
  Node(uint32_t _id, float _x, float _y, float _z): id(_id) {
    pos.x = _x;
    pos.y = _y;
    pos.z = _z;
  }
  uint32_t id;
  Vector pos;
};
std::map<uint32_t, Node*> node_map;
std::map<std::string, Device*> device_map;

float rssi_to_radius(int8_t rssi) {
  return ((float)rssi / 2) - 100;
}

void handle_message(bcast_message msg) {
  uint32_t chip_id = msg.m.src_id;

  // Make sure node is accounted for
  if(node_map.count(chip_id) == 0) {
    // Doesn't exist. Create
    node_map[chip_id] = new Node(chip_id, msg.x, msg.y, msg.z);
  }

  // Make sure device exists
  std::string mac((char *)msg.m.mac, 6);
  if(device_map.count(mac) == 0) {
    // Add to map
    device_map[mac] = new Device();
  }
  Device *device = device_map[mac];

  device->rssi[chip_id] = msg.m.rssi;
  if(device->rssi.size() >= 3) {
    iter =
  }
}
