#include "trilateration.h"

#include <string.h>
#include <stdio.h>
#include <math.h>




void testRotate2D() {
  Point p = {.x = 1, .y=0, .z=0};
  Point out;

  rotate2D(M_PI, &p, &out);

  if((roundf(out.x) == -1.0) && (roundf(out.y) == 0.0) && (roundf(out.z == 0.0))) {
    printf("rotate2D Passed\n");
  } else {
    printf("rotate2D Failed\n");
    printf("Got {%f, %f %f}\n", out.x, out.y, out.z);
  }
}

void testTranslate2D() {
  Point p = {.x = 1, .y = 1};
  float translation[2] = {-1.0, -1.0};
  Point out = {.x=0, .y=0, .z=0};
  translate2D(&p, translation, &out);

  if((roundf(out.x) == 0) && (roundf(out.y) == 0) && (roundf(out.z) == 0)) {
    printf("translate2D Passed\n");
  } else {
    printf("translate2D Failed\n");
    printf("Got {%f, %f, %f}\n", out.x, out.y, out.z);
  }
}


void testTrilateration2D() {

  // Make sure an example works
  Point p1 = {
    .x = 1,
    .y = 0,
    .z = 0
  };
  float r1 = 1;
  Point p2 = {
    .x = 3,
    .y = 0,
    .z = 0
  };
  float r2 = 1;
  Point p3 = {
    .x = 2,
    .y = 1,
    .z = 0
  };
  float r3 = 1;

  Point expected = {.x=2, .y=0, .z=0};


  Point output;
  memset(&output, 0, sizeof(output));

  trilaterate2D(p1, r1, p2, r2, p3, r3, &output);

  if((roundf(output.x) == expected.x) && (roundf(output.y) == expected.y) &&
      (roundf(output.y) == expected.z)) {
    printf("trilaterate2D Passed\n");
  } else {
    printf("trilaterate2D Failed\n");
    printf("Output was: {%f, %f, %f}\n", output.x, output.y, output.z);
  }
}


int main() {
  testTranslate2D();
  testRotate2D();
  testTrilateration2D();
  return 0;
}
