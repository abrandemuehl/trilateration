#ifndef TRILATERATION_H
#define TRILATERATION_H

typedef union {
  struct {
    float x;
    float y;
    float z;
    float err;
  };
  float xyz[3];
} Vector;



typedef enum {
  X,
  Y,
  Z,
} axis_t;

typedef enum {
  XY,
  XZ,
  YZ,
  XYZ,
} plane_t;

void mat_mat_multiply(float *mat1, float *mat2, float *output, int m, int n, int k);
void mat_vec_multiply(float *vec, float *mat, float *output, int m, int n);
float vec_magnitude(Vector v, plane_t plane);
Vector vec_add(Vector v1, Vector v2);
Vector vec_rotate(float theta, Vector v, axis_t axis);
Vector trilaterate2D(Vector v1, float r1, Vector v2, float r2, Vector v3, float r3);
Vector trilaterate3D(Vector v1, float r1, Vector v2, float r2, Vector v3, float r3);


#endif
