#include "trilateration.h"
#include "math.h"
#include "string.h"
#include <stdio.h>


float pointMag(Point *p) {
  float out = 0.0;
  for(int i=0; i < 3; i++) {
    out += pow(p->vec[i], 2);
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
void matMul(float *vec, float *mat, float *output, int m, int n) {
  float dot;
  for(int row=0; row<m; row++) {
    dot = 0.0;
    for(int col=0; col < n; col++) {
      dot += vec[col] * mat[row*n + col];
    }
    output[row] = dot;
  }
}


// Translates a 2D vector
void translate2D(Point *p, float translation[2], Point *output) {
  for(int i=0; i < 2; i++) {
    output->vec[i] = p->vec[i] + translation[i];
  }
}

// Apply a rotation to a vector based on a theta
void rotate2D(float theta, Point *p, Point *output) {
  float R[2][2];

  R[0][0] = cos(theta);
  R[0][1] = -sin(theta);
  R[1][0] = -R[0][1];
  R[1][1] = R[0][0];

  matMul(p->vec, (float *)R, output->vec, 2, 2);
}


void trilaterate2D(Point p1, float r1, Point p2, float r2, Point p3,
                   float r3, Point *output) {
  Point p2_prime, p3_prime;
  memset(&p2_prime, 0, sizeof(p2_prime));
  memset(&p3_prime, 0, sizeof(p3_prime));
  float translation[2];

  // Step 1: Bring P1 to the origin

  translation[0] = -p1.x;
  translation[1] = -p1.y;

  translate2D(&p2, translation, &p2_prime);
  translate2D(&p3, translation, &p3_prime);

  // Step 2: Rotate P2 to the axis
  Point p2_final, p3_final;
  memset(&p2_final, 0, sizeof(p2_final));
  memset(&p3_final, 0, sizeof(p3_final));

  float p2_mag = pointMag(&p2_prime);
  float theta = acos(p2_prime.x/p2_mag);

  rotate2D(theta, &p2_prime, &p2_final);
  rotate2D(theta, &p3_prime, &p3_final);


  // Step 3: Solve system of equations
  // Let d = P2.x, i = p3.x, j = p3.y
  // x = (r1^2 - r2^2 + d) / (2*d)
  // y = (r1^2 - r3^2, + i^2 + j^2) / (2*j) - (i/j)*x
  float d = p2_final.x;
  float i = p3_final.x;
  float j = p3_final.y;
  float x = (pow(r1, 2) - pow(r2, 2) + pow(d, 2))/(2 * d);
  float y = (pow(r1, 2) - pow(r3, 2) + pow(i, 2) + pow(j, 2))/(2*j) - (i/j) * x;

  Point tmp;
  memset(output, 0, sizeof(*output));
  memset(&tmp, 0, sizeof(tmp));
  tmp.x = x;
  tmp.y = y;

  // Step 4: Undo rotation
  rotate2D(-theta, &tmp, output);

  // Step 5: Undo translation
  translate2D(output, p1.vec, output);


  // Done
}
