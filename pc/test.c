#include "trilateration.h"

#include <string.h>
#include <stdio.h>
#include <math.h>




void testRotate2D() {
  Vector v = {.x = 1, .y=0, .z=0};
  Vector vout;

  vout = vec_rotate(M_PI, v, Z);

  if((roundf(vout.x) == -1.0) && (roundf(vout.y) == 0.0) && (roundf(vout.z == 0.0))) {
    printf("rotate2D Passed\n");
  } else {
    printf("rotate2D Failed\n");
    printf("Got {%f, %f %f}\n", vout.x, vout.y, vout.z);
  }
}

void testTranslate2D() {
  Vector p = {.x = 1, .y = 1};
  Vector translation = {.x=-1, .y=-1, .z=-1};
  Vector out = vec_add(p, translation);

  if((roundf(out.x) == 0) && (roundf(out.y) == 0) && (roundf(out.z) == -1)) {
    printf("translate2D Passed\n");
  } else {
    printf("translate2D Failed\n");
    printf("Got {%f, %f, %f}\n", out.x, out.y, out.z);
  }
}


void testTrilateration2D() {

  // Make sure an example works
  Vector v1 = {
    .x = 1,
    .y = 0,
    .z = 0
  };
  float r1 = 1;
  Vector v2 = {
    .x = 3,
    .y = 0,
    .z = 0
  };
  float r2 = 1;
  Vector v3 = {
    .x = 2,
    .y = 1,
    .z = 0
  };
  float r3 = 1;

  Vector expected = {.x=2, .y=0, .z=0};


  Vector output = trilaterate2D(v1, r1, v2, r2, v3, r3);

  if((roundf(output.x) == expected.x) && (roundf(output.y) == expected.y) &&
      (roundf(output.y) == expected.z)) {
    printf("trilaterate2D Passed\n");
  } else {
    printf("trilaterate2D Failed\n");
    printf("Output was: {%f, %f, %f}\n", output.x, output.y, output.z);
  }
}


void testTrilateration3D() {
  // Make sure an example works
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
  Vector output = trilaterate3D(v1, r1, v2, r2, v3, r3);

  if((roundf(output.x) == expected.x) && (roundf(output.y) == expected.y) &&
      (roundf(output.y) == expected.z)) {
    printf("trilaterate2D Passed\n");
  } else {
    printf("trilaterate2D Failed\n");
    printf("Output was: {%f, %f, %f}\n", output.x, output.y, output.z);
  }
}

void test_mat_mat_multiply() {
  float mat1[3][3] = {{1, 0, 0},
                      {0, 1, 0},
                      {0, 0, 1}};
  float mat2[3][3] = {{1, 2, 3},
                      {1, 2, 3},
                      {1, 2, 3}};

  float output[3][3];
  mat_mat_multiply((float *)mat1, (float *)mat2, (float *)output, 3, 3, 3);

  int same = 1;
  for(int i=0; i < 3; i++) {
    for(int j=0; j < 3; j++) {
      if(output[j][i] != mat2[j][i]) {
        same = 0;
      }
    }
  }
  if(same) {
    printf("mat_mat_multiply Passed\n");
  } else {
    printf("mat_mat_multiply Failed\n");
    printf("Got \
        [[%f %f %f], \
         [%f %f %f], \
         [%f %f %f]] \
        ", output[0][0], output[0][1], output[0][2],
        output[1][0], output[1][1], output[1][2],
        output[2][0], output[2][1], output[2][2]);
  }
}
void test_mat_vec_multiply() {
  float mat[3][3] = {{1, 0, 0},
                     {0, 1, 0},
                     {0, 0, 1}};
  float vec[3] = {1, 2, 3};

  float output[3];
  mat_vec_multiply((float *)vec, (float *)mat, (float *)output, 3, 3);

  int same = 1;
  for(int i=0; i < 3; i++) {
    if(output[i] != vec[i]) {
      same = 0;
    }
  }
  if(same) {
    printf("mat_vec_multiply Passed\n");
  } else {
    printf("mat_vec_multiply Failed\n");
    printf("Got [%f %f %f]\n", output[0], output[1], output[2]);
  }
}

int main() {
  testTranslate2D();
  testRotate2D();
  testTrilateration2D();
  testTrilateration3D();
  test_mat_mat_multiply();
  test_mat_vec_multiply();
  return 0;
}
