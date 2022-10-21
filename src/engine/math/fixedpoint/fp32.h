#ifndef FP32_H
#define FP32_H

#include <stdint.h>

typedef int32_t fp32;

#define FADD(a, b)  (a) + (b)
#define FSUB(a, b)  (a) + (~b+1)
#define FMUL(a, b)  ((int64_t)(a)) * ((int64_t)(b)) >> 16
#define FDIV(a, b)  ((int64_t)(a)<<16) / ((int64_t)(b))


fp32 float_to_fp32(float a);
float fp32_to_float(fp32 a);

#endif /* FP32_H */