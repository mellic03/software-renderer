#include "enginemath.h"

float clamp(float n, float min, float max)
{
  return MIN(MAX(n, min), max);
}