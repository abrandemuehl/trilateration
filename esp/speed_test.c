#include "../common/trilateration.h"
#include "user_interface.h"
#include "osapi.h"
#include "math.h"





void user_init() {
  uart_div_modify(0, UART_CLK_FREQ / 115200);
  Vector v1 = {
    .x = 1,
    .y = 0,
    .z = 0
  };
  float r1 = 1;
  Vector v2 = {
    .x = 0,
    .y = 1,
    .z = 0
  };
  float r2 = 1;
  Vector v3 = {
    .x = 0,
    .y = 0,
    .z = 1
  };
  float r3 = 1;
  Vector expected = {.x=0, .y=0, .z=0};
  int i=0;
  uint32_t start, end;
  os_printf("Starting benchmark\n");
  start = system_get_time();
  Vector output;
  for(i = 0; i < 1000; i++) {
    output = trilaterate3D(v1, r1, v2, r2, v3, r3);
  }

  if((roundf(output.x) == expected.x) && (roundf(output.y) == expected.y) &&
      (roundf(output.y) == expected.z)) {
    // Do nothing
  } else {
    os_printf("Incorrect output\n");
  }
  end = system_get_time();
  os_printf("%dus passed for 1000 iterations\n", end-start);

}
