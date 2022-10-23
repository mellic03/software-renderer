#ifndef FP32_H
#define FP32_H

#include <stdint.h>

typedef int64_t fp32;

#define FADD(a, b)  ((a) + (b))
#define FSUB(a, b)  ((a) + (~b+1))
#define FMUL(a, b)  (((a) * (b)) >> 16)
#define FDIV(a, b)  (((a)<<16) / ((b)))

#define FSQR(a)  (FMUL(a, a))
#define FNEG(a)  ((~a) + 1)


float fp32_to_float(fp32 a);
fp32 float_to_fp32(float a);
fp32 fp32_sqrt(fp32 a);


#endif /* FP32_H */