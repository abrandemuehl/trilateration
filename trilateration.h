#ifndef TRILATERATION_H
#define TRILATERATION_H

typedef union {
  struct {
    float x;
    float y;
    float z;
  };
  float vec[3];
} Point;



float pointMag(Point *p);
void matMul(float *vec, float *mat, float *output, int m, int n);
void translate2D(Point *p, float translation[2], Point *output);
void rotate2D(float theta, Point *p, Point *output);
void trilaterate2D(Point p1, float r1, Point p2, float r2, Point p3,
                   float r3, Point *output);


#endif
