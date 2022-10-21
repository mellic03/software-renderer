#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "fp32.h"


fp32 float_to_fp32(float a)
{
  return (fp32)(a * (float)(1 << sizeof(fp32)*4) + (a >= 0 ? 0.5 : -0.5));
}

float fp32_to_float(fp32 a)
{
  return (float)a / (float)(1 << sizeof(fp32)*4);
}

void print_fp32(fp32 a)
{
  unsigned long n = 1UL << sizeof(fp32)*8;

  for (int i=0; i<=(sizeof(fp32)*8) + 1; i++)
  {
    printf("%u ", n&a ? 1 : 0);
      if (i == sizeof(fp32)*4)
        printf(". ");
    n >>= 1;
  }
  printf("\n");
}

fp32 fp32_mul(fp32 a, fp32 b)
{
  return ((int64_t)a * (int64_t)b) >> 16;
}

fp32 fp32_div(fp32 a, fp32 b)
{
  return ((int64_t)(a)<<16) / (int64_t)b;
}

int main()
{
  fp32 a = float_to_fp32(1.0);
  fp32 b = float_to_fp32(5.0);
  fp32 c = FDIV(a, b);


  print_fp32(a);
  print_fp32(b);
  print_fp32(c);

  float d = fp32_to_float(c);

  printf("%f\n", d);

  return 0;
}