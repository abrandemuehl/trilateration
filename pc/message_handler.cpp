#include "message_handler.h"
#include <map>
#include <cstdint>
#include <string>
#include "types.h"
#include <math.h>
#include <string.h>
extern "C" {
#include "../common/trilateration.h"
}


std::map<uint32_t, Node*> node_map;
std::map<std::string, Device*> device_map;
float myx, myy, myz;

float rssi_to_radius(int8_t rssi) {
  float dbm = ((float)rssi / 2) - 100;
  return -dbm;
}

uint32_t my_chipid;
void handle_message(bcast_message msg) {
  uint32_t chip_id = msg.m.src_id;

  // Make sure node is accounted for
  if(node_map.count(chip_id) == 0) {
    // Doesn't exist. Create
    node_map[chip_id] = new Node(chip_id, msg.x, msg.y, msg.z);
  }


  Vector points[3];
  Node *node1, *node2;
  points[0].x = myx;
  points[0].y = myy;
  points[0].z = myz;
  auto iter = node_map.begin();
  points[1] = iter->second->pos;
  node1 = iter->second;
  iter++;
  points[2] = iter->second->pos;
  node2 = iter->second;

  // Add device
  std::string mac((char *)msg.m.mac, 6);
  if(device_map.count(mac) == 0) {
    // Add to map
    device_map[mac] = new Device();
  }
  Device *device = device_map[mac];

  // Calculate the radii values
  device->rssi[chip_id] = msg.m.rssi;
  float r0, r1, r2;
  r0 = rssi_to_radius(device->rssi[my_chipid]);
  r1 = rssi_to_radius(device->rssi[node1->id]);
  r2 = rssi_to_radius(device->rssi[node2->id]);

  Vector pos = trilaterate3D(points[0], r0, points[1], r1, points[2], r2);
  printf("{%f %f %f}", pos.x, pos.y, pos.z);
}
