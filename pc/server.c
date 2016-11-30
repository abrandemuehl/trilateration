#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <pthread.h>


#include "../common/messages.h"
#include "error.h"
#include "serial.h"


#define DEFAULT_PORT "/dev/ttyUSB0"


#define PORT 5555


void *readSerialThread(void *arg) {
  int fd = open((char *)arg, O_RDWR | O_NOCTTY | O_SYNC);

  if(fd < 0) {
    ERROR("%d %s", errno, strerror(errno));
  }
  set_interface_attribs(fd, B115200, 0);
  set_blocking (fd, 1);

  int sock = socket(AF_INET, SOCK_DGRAM, 0);
  if(sock < 0) {
    ERROR("Failed to open socket: %d %s", errno, strerror(errno));
  }
  int broadcastEnable=1;
  setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));

  struct sockaddr_in s;
  memset(&s, '\0', sizeof(struct sockaddr_in));
  s.sin_family = AF_INET;
  s.sin_port = htons(PORT);
  s.sin_addr.s_addr = htonl(INADDR_BROADCAST);

  message msg;
  uint8_t *buf = (uint8_t *)&msg;
  while(1) {
    // Wait for prefix
    read(fd, &buf[0], 1);
    if(buf[0] == 'f') {
      // Read next byte
      read(fd, &buf[1], 1);
      if(buf[1] == 'd') {
        read(fd, &buf[2], 1);
        if(buf[2] == 'a') {
          // Start of a message
          // Move on in the block
        } else {
          continue;
        }
      } else {
        continue;
      }
    } else {
      continue;
    }

    // Message header received
    read(fd, &buf[3], sizeof(message) - sizeof(header));
    printf("%d %02x:%02x:%02x:%02x:%02x:%02x\n", msg.rssi,
        msg.mac[0], msg.mac[1], msg.mac[2],
        msg.mac[3], msg.mac[4], msg.mac[5]);

    if(sendto(sock, (uint8_t *)&msg, sizeof(msg), 0, (struct sockaddr *)&s, sizeof(s)) < 0) {
      fprintf(stderr, "Failed to broadcast message\n");
    }
  }
  close(fd);
  return NULL;
}

void *readNetThread(void *arg) {
  (void)arg; // Unused arg

  int sock = socket(AF_INET, SOCK_DGRAM, 0);
  if(sock < 0) {
    ERROR("Failed to open socket: %d %s", errno, strerror(errno));
  }
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(PORT);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  if(bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
    ERROR("Could not bind socket %d %s", errno, strerror(errno));
  }

  message m;
  while(1) {
    int len = recv(sock, (uint8_t *)&m, sizeof(m), 0);
    if(len != sizeof(m)) {
      fprintf(stderr, "Bad message received of length %d\n", len);
      continue;
    }
  }
  return NULL;
}



int main(int argc, char **argv) {
  // Check if a serial port is specified

  char *port_name = NULL;
  switch(argc) {
    case 1:
      // No serial port specified
      port_name = DEFAULT_PORT;
      break;
    case 2:
      port_name = argv[1];
      break;
    case 3:
      ERROR("Invalid arguments\n%s <port>", argv[0]);
      break;
  }

  pthread_t serial_tid, net_tid;

  pthread_create(&serial_tid, NULL, readSerialThread, (void *)port_name);
  pthread_create(&net_tid, NULL, readNetThread, NULL);

  pthread_join(serial_tid, NULL);
  pthread_join(net_tid, NULL);
}
