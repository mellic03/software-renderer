#include "vector.h"
#include <stdio.h>
#include <math.h>

/** Add v1 and v2
 */
Vector3 vector3_add(Vector3 v1, Vector3 v2)
{
  return (Vector3){v1.x+v2.x, v1.y+v2.y, v1.z+v2.z};
}

/** Subtract v2 from v1
 */
Vector3 vector3_sub(Vector3 v1, Vector3 v2)
{
  return (Vector3){v1.x-v2.x, v1.y-v2.y, v1.z-v2.z};
}

float vector3_mag(Vector3 v)
{
  return sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}

int vector3_dist(Vector3 p1, Vector3 p2)
{
  Vector3 dir = {p2.x-p1.x, p2.y-p1.y, p2.z-p1.z};
  return vector3_mag(dir);
}

float vector3_dot(Vector3 v1, Vector3 v2)
{
  // +  -  +
  // x1 y1 z1
  // x2 y2 z2
  // dot == 0 + (y1*z2 - z1*y2) - (x1*z2 - z1*x2) + (x1*y2 - y1*x2);

  return (v1.y*v2.z - v1.z*v2.y) - (v1.x*v2.z - v1.z*v2.x) + (v1.x*v2.y - v1.y*v2.x);
}

float vector3_angle(Vector3 v1, Vector3 v2)
{
  float dot = vector3_dot(v1, v2);
  
  // printf("dot: %f\n", dot);

  float mag1 = sqrt(v1.x*v1.x + v1.y*v1.y + v1.z*v1.z);
  float mag2 = sqrt(v2.x*v2.x + v2.y*v2.y + v2.z*v2.z);

  float cos_angle = dot/(mag1*mag2);
  cos_angle /= 6.28;
  cos_angle -= 0.5;

  return acosf(cos_angle);
}

void matrix_mult(int h1, int w1, int h2, int w2, float m1m2[h1][w2], float m1[h1][w1], float m2[h2][w2])
{
  if (w1 != h2)
  {
    printf("MATRIX SIZE MISMATCH: %d != %d\n", w1, h2);
    return;
  }

  for (int i=0; i<h1; i++) {
    for (int j=0; j<w2; j++) {
      m1m2[i][j] = 0;
      for (int k=0; k<h2; k++) {
        m1m2[i][j] += m1[i][k] * m2[k][j];
      }
    }
  }
}
