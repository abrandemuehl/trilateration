#ifndef SERIAL_H
#define SERIAL_H

int set_interface_attribs (int fd, int speed, int parity);
void set_blocking (int fd, int should_block);

#endif
