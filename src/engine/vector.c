#include "vector.h"
#include <stdio.h>
#include <math.h>


Vector2 vector2_add(Vector2 v1, Vector2 v2)
{
  return (Vector2){v1.x+v2.x, v1.y+v2.y};
}

float vector2_dot(Vector2 v1, Vector2 v2)
{
  float dot = 0;
  dot += v1.x * v2.x;
  dot += v1.y * v2.y;
  return dot;
}

float vector2_angle(Vector2 v1, Vector2 v2)
{
  float dot = vector2_dot(v1, v2);
  float mag1 = sqrt(v1.x*v1.x + v1.y*v1.y);
  float mag2 = sqrt(v2.x*v2.x + v2.y*v2.y);
  float cos_angle = dot/(mag1*mag2);
  return acosf(cos_angle);
} 

/** Scale v1 by a scalar
 */
Vector2 vector2_scale(Vector2 v1, float scalar)
{
  return (Vector2){v1.x * scalar, v1.y * scalar};
}

Vector2 vector2_sub(Vector2 v1, Vector2 v2)
{
  return (Vector2){v1.x-v2.x, v1.y-v2.y};
}


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
  float dot = 0;
  dot += v1.x * v2.x;
  dot += v1.y * v2.y;
  dot += v1.z * v2.z;
  return dot;
}

float vector3_cross(Vector3 v1, Vector3 v2)
{
  return (v1.y*v2.z - v1.z*v2.y) - (v1.x*v2.z - v1.z*v2.x) + (v1.x*v2.y - v1.y*v2.x);
}

float vector3_angle(Vector3 v1, Vector3 v2)
{
  float dot = vector3_dot(v1, v2);
  float mag1 = vector3_mag(v1);
  float mag2 = vector3_mag(v2);
  float cos_angle = dot/(mag1*mag2);
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
