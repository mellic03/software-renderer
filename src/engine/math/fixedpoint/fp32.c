#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <math.h>

#include "fp32.h"


fp32 float_to_fp32(float a)
{
  return (fp32)(a * (float)(1 << sizeof(int32_t)*4) + (a >= 0 ? 0.5 : -0.5));
}

float fp32_to_float(fp32 a)
{
  return (float)a / (float)(1 << sizeof(int32_t)*4);
}

void print_fp32(fp32 a)
{
  unsigned long n = 1UL << sizeof(int32_t)*8;

  for (int i=0; i<=(sizeof(int32_t)*8) + 1; i++)
  {
    printf("%u ", n&a ? 1 : 0);
      if (i == sizeof(int32_t)*4)
        printf(". ");
    n >>= 1;
  }
  printf("\n");
}

fp32 fp32_mul(fp32 a, fp32 b)
{
  return (a * b) >> 16;
}

fp32 fp32_div(fp32 a, fp32 b)
{
  return ((a)<<16) / b;
}

fp32 fp32_abs(fp32 a)
{
  fp32 mask = a >> 31;
  a = a ^ mask;
  a = a - mask;
  return a;
}

fp32 fp32_sqrt(fp32 a)
{
  return float_to_fp32(sqrt(fp32_to_float(a)));
}

// fp32 fp32_sqrt(fp32 a)
// {
//   fp32 x0 = a >> 1;
//   fp32 x1 = a;

//   while (fp32_abs(FSUB(x0, x1)) > 1 << 14)
//   {
//     x1 = x0;
//     x0 = x0 + FDIV(a, x0) >> 1;
//   }

//   return x0;
// }

