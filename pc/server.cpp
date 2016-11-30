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
#include <string>
#include <stdio.h>
#include <sys/ioctl.h>
#include <net/if.h>


extern "C" {
#include "../common/messages.h"
#include "../common/trilateration.h"
#include "error.h"
}
#include "serial.h"
#include "types.h"
#include "message_handler.h"


#define DEFAULT_PORT "/dev/ttyUSB0"


#define PORT 5555


void *readSerialThread(void *arg) {
  int fd = *(int *)arg;

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

  bcast_message msg;
  uint8_t *buf = (uint8_t *)&msg.m;
  while(1) {
    // Wait for prefix
    read(fd, &buf[0], 1);
    /* printf("Received %02x %c\n", buf[0], buf[0]); */
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
    my_chipid = msg.m.src_id;
    printf("Serial: %02x:%02x:%02x:%02x:%02x:%02x %d\n",
        msg.m.mac[0], msg.m.mac[1], msg.m.mac[2],
        msg.m.mac[3], msg.m.mac[4], msg.m.mac[5], msg.m.rssi);

    if(sendto(sock, (uint8_t *)&msg, sizeof(msg), 0, (struct sockaddr *)&s, sizeof(s)) < 0) {
      fprintf(stderr, "Failed to broadcast message\n");
    }
  }
  close(fd);
  return NULL;
}

void *readNetThread(void *arg) {
  int fd = *(int *)arg;

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

  bcast_message m;
  m.x = myx;
  m.y = myy;
  m.z = myz;
  while(1) {
    int len = recv(sock, (uint8_t *)&m, sizeof(m), 0);
    if(len != sizeof(m)) {
      fprintf(stderr, "Bad message received of length %d\n", len);
      continue;
    }
    if(m.m.head.PREFIX0 != 'f' || m.m.head.PREFIX1 != 'd' || m.m.head.PREFIX2 != 'a') {
      fprintf(stderr, "Bad message received prefix: %c%c%c", m.m.head.PREFIX0, m.m.head.PREFIX1, m.m.head.PREFIX2);
      continue;
    }
    write(fd, &m, sizeof(m));
    printf("WIFI:   %02x:%02x:%02x:%02x:%02x:%02x %d\n", m.m.mac[0], m.m.mac[1], m.m.mac[2], m.m.mac[3], m.m.mac[4], m.m.mac[5], m.m.rssi);
  }
  return NULL;
}



int main(int argc, char **argv) {
  // Check if a serial port is specified

  std::string port_name;
  switch(argc) {
    case 5:
      port_name = argv[1];
      sscanf(argv[2], "%f", &myx);
      sscanf(argv[3], "%f", &myy);
      sscanf(argv[4], "%f", &myz);
      break;
    default:
      ERROR("Invalid arguments\n%s <port> <x> <y> <z>", argv[0]);
      break;
  }

  int fd = open(port_name.c_str(), O_RDWR | O_NOCTTY | O_SYNC);

  if(fd < 0) {
    ERROR("%d %s", errno, strerror(errno));
  }
  set_interface_attribs(fd, B115200, 0);
  set_blocking (fd, 1);

  pthread_t serial_tid, net_tid;
  pthread_create(&serial_tid, NULL, readSerialThread, (void *)&fd);
  pthread_create(&net_tid, NULL, readNetThread, (void *)&fd);

  pthread_join(serial_tid, NULL);
  pthread_join(net_tid, NULL);
}
