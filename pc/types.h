#ifndef TYPES_H
#define TYPES_H
#include "../common/messages.h"

struct bcast_message {
  message m;
  float x, y, z;
};
#endif
