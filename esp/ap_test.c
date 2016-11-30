#include "user_interface.h"
#include "osapi.h"

#define SSID "WifiTrilateration"
#define PASSWORD "password"

#define SSID "Adrian's"
#define SSID_PASSWORD "password"


void ICACHE_FLASH_ATTR user_init() {
  uart_div_modify(0, UART_CLK_FREQ / 115200);
  char ssid[32] = SSID;
  char password[64] = SSID_PASSWORD;
  struct softap_config apConfig;

  //Set AP mode
  wifi_set_opmode(2);

  //Set ap settings
  os_memcpy(&apConfig.ssid, ssid, 32);
  os_memcpy(&apConfig.password, password, 64);
  apConfig.channel = 1;
  apConfig.authmode = 4;
  wifi_softap_set_config (&apConfig);
}
