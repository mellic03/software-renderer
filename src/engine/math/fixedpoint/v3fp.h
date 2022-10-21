#ifndef V3FP_H
#define V3FP_H

#include "fp32.h"

// VECTOR 3 FIXED POINT
//----------------------------------------------

typedef struct {
  fp32 x, y, z;
} v3fp_t;

v3fp_t v3fp_add(v3fp_t v1, v3fp_t v2);
v3fp_t v3fp_sub(v3fp_t v1, v3fp_t v2);
//----------------------------------------------

#endif /* V3FP_H */