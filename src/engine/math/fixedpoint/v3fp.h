#ifndef V3FP_H
#define V3FP_H

#include "fp32.h"

// VECTOR 3 FIXED POINT
//----------------------------------------------

typedef struct {
  fp32 x, y, w;
} v2fp_t;

typedef struct {
  fp32 x, y, z;
} v3fp_t;

fp32 v3fp_dot(v3fp_t a, v3fp_t b);

v3fp_t v3fp_add(v3fp_t v1, v3fp_t v2);
v3fp_t v3fp_sub(v3fp_t v1, v3fp_t v2);
v3fp_t v3fp_cross(v3fp_t a, v3fp_t b);
v3fp_t v3fp_scale(v3fp_t a, fp32 scalar);
void v3fp_normalise(v3fp_t *a);
//----------------------------------------------

#endif /* V3FP_H */