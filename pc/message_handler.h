#ifndef MESSAGE_HANDLER_H
#define MESSAGE_HANDLER_H
#include "../common/messages.h"
#include <map>
#include <string>
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

extern uint32_t my_chipid;
extern std::map<uint32_t, Node*> node_map;
extern std::map<std::string, Device*> device_map;

extern float myx, myy, myz;
void handle_message(message m);
#endif
