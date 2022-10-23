#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "v3fp.h"
#include "fp32.h"

v3fp_t v3fp_add(v3fp_t a, v3fp_t b)
{
  return (v3fp_t){a.x + b.x, a.y + b.y, a.z + b.z};  
}

v3fp_t v3fp_sub(v3fp_t a, v3fp_t b)
{
  return (v3fp_t){
    FSUB(a.x, b.x),
    FSUB(a.y, b.y),
    FSUB(a.z, b.z)
  };  
}

fp32 v3fp_dot(v3fp_t a, v3fp_t b)
{
  return FMUL(a.x, b.x) + (FMUL(a.y, b.y) + FMUL(a.z, b.z));
}

v3fp_t v3fp_cross(v3fp_t a, v3fp_t b)
{
  return (v3fp_t) {
    +FSUB(FMUL(a.y, b.z), FMUL(a.z, b.y)),
    -FSUB(FMUL(a.x, b.z), FMUL(a.z, b.x)),
    +FSUB(FMUL(a.x, b.y), FMUL(a.y, b.x))
  };
}

fp32 v3fp_mag(v3fp_t a)
{
  return fp32_sqrt(FSQR(a.x) + FSQR(a.y) + FSQR(a.z));
}


v3fp_t v3fp_scale(v3fp_t a, fp32 scalar)
{
  return (v3fp_t){FMUL(a.x, scalar), FMUL(a.y, scalar), FMUL(a.z, scalar)};
}

void v3fp_normalise(v3fp_t *a)
{
  fp32 mag = v3fp_mag(*a);
  a->x = FDIV(a->x, mag);
  a->y = FDIV(a->y, mag);
  a->z = FDIV(a->z, mag);
}