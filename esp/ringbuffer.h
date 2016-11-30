#ifndef RINGBUFFER_H
#define RINGBUFFER_H



#define RB_SIZE 255

typedef struct {
  int *start, *end;
  int data[RB_SIZE];
} RingBuffer;



void rb_init(RingBuffer *b);
/* Gets an item from the ringbuffer */
int rb_get(RingBuffer *b);
/* Puts an item into the ringbuffer */
void rb_push(RingBuffer *b, int val);

#endif // RINGBUFFER_H
