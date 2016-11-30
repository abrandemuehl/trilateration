#include "trilateration.h"
#include <math.h>
#ifdef __xtensa__
#include "user_interface.h"
#else
#include "string.h"
#include <stdio.h>
#endif



float
#ifdef __xtensa__
  ICACHE_FLASH_ATTR
#endif
vec_magnitude(Vector v, plane_t plane)
{
  float out = 0.0;

  switch (plane) {
    case XY:
      out = pow(v.x, 2) + pow(v.y, 2);
      break;
    case XZ:
      out = pow(v.x, 2) + pow(v.z, 2);
      break;
    case YZ:
      out = pow(v.y, 2) + pow(v.z, 2);
      break;
    case XYZ:
      out = pow(v.x, 2) + pow(v.y, 2) + pow(v.z, 2);
      break;
  }
  return sqrt(out);
}

// Matrix-vector multiplication
// Multiply an mxn matrix by a nx1 vector
//
// EXAMPLE:
// mat = [[1,2],
//        [3,4],
//        [5,6]];
// vec = [1,
//        2]
// m = 3
// n = 2
// output is mx1
void
#ifdef __xtensa__
  ICACHE_FLASH_ATTR
#endif
mat_vec_multiply(float *vec, float *mat, float *output, int m, int n)
{
  float dot;
  int row, col;
  for(row=0; row<m; row++) {
    dot = 0.0;
    for(col=0; col < n; col++) {
      dot += vec[col] * mat[row*n + col];
    }
    output[row] = dot;
  }
}


// Matrix matrix multiplication
// Multiply an mxn matrix by a nxk matrix
//
// EXAMPLE:
// mat1 = [[1,2],
//        [3,4],
//        [5,6]];
// mat2 = [[1,2],
//         [3,4]]
// m = 3
// n = 2
// k = 2
// output is mxk
void
#ifdef __xtensa__
  ICACHE_FLASH_ATTR
#endif
mat_mat_multiply(float *mat1, float *mat2, float *output, int m, int n, int k)
{
  float dot;
  int y, x, i;
  for(y=0; y < k; y++) {
    for(x=0; x < m; x++) {
      dot = 0.0;
      for(i=0; i < n; i++) {
        dot += mat1[y*n + i] * mat2[i*k + x];
      }
      output[y*k+x] = dot;
    }
  }
}

// Vector addition
Vector
#ifdef __xtensa__
  ICACHE_FLASH_ATTR
#endif
vec_add(Vector v1, Vector v2)
{
  Vector vout;
  int i;
  for(i=0; i < 3; i++) {
    vout.xyz[i] = v1.xyz[i] + v2.xyz[i];
  }
  return vout;
}

// Apply a rotation to a vector based on a theta
Vector
#ifdef __xtensa__
  ICACHE_FLASH_ATTR
#endif
rotate2D(float theta, Vector v) {
  return vec_rotate(theta, v, Z);
}

Vector
#ifdef __xtensa__
  ICACHE_FLASH_ATTR
#endif
vec_rotate(float theta, Vector v, axis_t axis) {
  if(theta == 0.0 || theta == -0.0) {
    return v;
  }
  float R[3][3] = {{0}, {0}, {0}};
  Vector vout;

  float cos_theta = cos(theta);
  float sin_theta = sin(theta);

  switch(axis) {
    case X:
      R[0][0] = 1;
      R[1][1] = cos_theta;
      R[1][2] = -sin_theta;
      R[2][1] = sin_theta;
      R[2][2] = cos_theta;
      break;
    case Y:
      R[1][1] = 1;
      R[0][0] = cos_theta;
      R[0][2] = sin_theta;
      R[2][0] = -sin_theta;
      R[2][2] = cos_theta;
      break;
    case Z:
      R[2][2] = 1;
      R[0][0] = cos_theta;
      R[0][1] = -sin_theta;
      R[1][0] = sin_theta;
      R[1][1] = cos_theta;
      break;
  }
  mat_vec_multiply(v.xyz, (float *)R, vout.xyz, 3, 3);
  return vout;
}


Vector
#ifdef __xtensa__
  ICACHE_FLASH_ATTR
#endif
trilaterate2D(Vector v1, float r1, Vector v2, float r2, Vector v3, float r3) {
  Vector output;
  Vector v2_prime, v3_prime;

  // Step 1: Bring v1 to the origin
  Vector translation;
  translation.xyz[0] = -v1.x;
  translation.xyz[1] = -v1.y;

  v2_prime = vec_add(v2, translation);
  v3_prime = vec_add(v3, translation);

  // Step 2: Rotate v2 to the axis
  Vector v2_final, v3_final;

  float v2_mag = vec_magnitude(v2_prime, XY);
  float theta = acos(v2_prime.x/v2_mag);

  v2_final = vec_rotate(theta, v2_prime, Z);
  v3_final = vec_rotate(theta, v3_prime, Z);


  // Step 3: Solve system of equations
  // Let d = v2.x, i = v3.x, j = v3.y
  // x = (r1^2 - r2^2 + d) / (2*d)
  // y = (r1^2 - r3^2, + i^2 + j^2) / (2*j) - (i/j)*x
  float d = v2_final.x;
  float i = v3_final.x;
  float j = v3_final.y;
  float x = (pow(r1, 2) - pow(r2, 2) + pow(d, 2))/(2 * d);
  float y = (pow(r1, 2) - pow(r3, 2) + pow(i, 2) + pow(j, 2))/(2*j) - (i/j) * x;

  Vector tmp;
  tmp.x = x;
  tmp.y = y;
  tmp.z = 0;

  // Step 4: Undo rotation
  output = rotate2D(-theta, tmp);

  // Step 5: Undo translation
  output = vec_add(output, v1);

  // Done
  return output;
}

Vector
#ifdef __xtensa__
  ICACHE_FLASH_ATTR
#endif
trilaterate3D(Vector v1, float r1, Vector v2, float r2, Vector v3, float r3) {
  Vector output;
  Vector v2_prime, v3_prime;

  // Step 1: Move points to where v1 == (0, 0, 0)
  Vector translation;
  translation.xyz[0] = -v1.x;
  translation.xyz[1] = -v1.y;
  translation.xyz[2] = -v1.z;

  v2_prime = vec_add(v2, translation);
  v3_prime = vec_add(v3, translation);

  // Step 2: Rotate v2 about the z and x axis to get a
  // About Z axis
  float mag_xy = vec_magnitude(v2_prime, XY);
  float theta_z;
  if(mag_xy == 0.0 || mag_xy == -0.0) {
    theta_z = 0;
  } else {
    theta_z = asin(v2_prime.y/mag_xy);
  }

  v2_prime = vec_rotate(theta_z, v2_prime, Z);
  v3_prime = vec_rotate(theta_z, v3_prime, Z);

  // About Y axis
  float mag_xz = vec_magnitude(v2_prime, XZ);
  float theta_y;
  if(mag_xz == 0.0 || mag_xz == -0.0) {
    theta_y = 0;
  } else {
    theta_y = acos(v2_prime.x/mag_xz);
  }

  v2_prime = vec_rotate(theta_y, v2_prime, Y);
  v3_prime = vec_rotate(theta_y, v3_prime, Y);


  // Solve system of equations
  // Let P1 = (0, 0, 0), P2 = (d, 0, 0), P3 = (i, j, 0)
  // x = (r1^2-r2^2+d^2)/(2d)
  // y = (r1^2-r3^2-x^2+(x-i)^2+j^2)/(2j)
  // z = plus_minus sqrt(x^2+y^2-r1^2)

  float d = v2_prime.x;
  float i = v3_prime.x;
  float j = v3_prime.y;

  output.x = (pow(r1,2) - pow(r2,2) + pow(d, 2)) / (2*d);
  output.y = (pow(r1,2) - pow(r3,2) - pow(output.x, 2) + pow(output.x-i, 2) + pow(j,2))/(2*j);
  output.z = sqrt(pow(output.x,2) + pow(output.y,2) - pow(r1, 2));
  if(isnan(output.z)) {
    // Doesn't necessarily output a correct Z
    output.z = 0.0;
  }
  // Step 4: Undo rotation
  output = vec_rotate(-theta_z, output, Z);
  output = vec_rotate(-theta_y, output, Y);

  // Step 5: Undo translation
  output = vec_add(output, v1);

  // Done
  return output;
}
