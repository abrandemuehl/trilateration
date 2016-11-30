#include "ringbuffer.h"




void rb_init(RingBuffer *rb) {
  rb->start = &rb->data[0];
  rb->end = &rb->data[0];
}


int rb_get(RingBuffer *rb) {
  int val;
  if(rb->start == rb->end) {
    return -1;
  }
  val = *rb->start;
  rb->start++;
  if(rb->start > &rb->data[RB_SIZE-1]) {
    rb->start = &rb->data[0];
  }
  return val;
}

void rb_push(RingBuffer *rb, int val) {
  int *next = rb->end + 1;
  if(next > &rb->data[RB_SIZE-1]) {
    next = &rb->data[0];
  }
  if(next == rb->start) {
    return;
  }

  *rb->end = val;
  rb->end = next;
}
