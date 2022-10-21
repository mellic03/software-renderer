#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "v3fp.h"
#include "fp32.h"

v3fp_t v3fp_add(v3fp_t a, v3fp_t b)
{
  return (v3fp_t){
    FADD(a.x, b.x),
    FADD(a.y, b.y),
    FADD(a.z, b.z)
  };  
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
  return FADD(FMUL(a.x, b.x), FADD(FMUL(a.y, b.y), FMUL(a.z, b.z)));
}

