#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../engine.h"
#include "enginemath.h"
#include <xmmintrin.h>


Vector2 vector2_add(Vector2 v1, Vector2 v2)
{
  return (Vector2){v1.x+v2.x, v1.y+v2.y, 1};
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

float vector2_cross(Vector2 v0, Vector2 v1)
{
  return v0.x*v1.y - v0.y*v1.x;
}

/** Scale v1 by a scalar
 */
Vector2 vector2_scale(Vector2 v1, float scalar)
{
  return (Vector2){v1.x * scalar, v1.y * scalar, 1};
}

Vector2 vector2_sub(Vector2 v1, Vector2 v2)
{
  return (Vector2){v1.x-v2.x, v1.y-v2.y, 1};
}

float vector2_mag(Vector2 v)
{
  return sqrt(v.x*v.x + v.y*v.y);
}

float vector2_dist(Vector2 v1, Vector2 v2)
{
  return sqrt(Sq(v2.x-v1.x) + Sq(v2.y-v1.y));
}

void vector2_normalise(Vector2 *v1)
{
  float mag = vector2_mag(*v1);
  v1->x /= mag;
  v1->y /= mag;
}

/** Move v1 towards v2 by alpha amount
 */
Vector2 vector2_lerp(Vector2 v1, Vector2 v2, float alpha)
{
  Vector2 dir = vector2_sub(v2, v1);
  vector2_normalise(&dir);
  return vector2_add(v1, vector2_scale(dir, alpha));
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

Vector3 vector3_scale(Vector3 v1, float scalar)
{
  return (Vector3){v1.x * scalar, v1.y * scalar, v1.z * scalar};
}

float vector3_mag(Vector3 v)
{
  float sqd = v.x*v.x + v.y*v.y + v.z*v.z;
  return sqrt(sqd);
}

float vector3_dist(Vector3 v1, Vector3 v2)
{
  return sqrt(Sq(v2.x-v1.x) + Sq(v2.y-v1.y) + Sq(v2.z-v1.z));
}

float vector3_dot(Vector3 v1, Vector3 v2)
{
  return (v1.x*v2.x) + (v1.y*v2.y) + (v1.z*v2.z);
}

void vector3_normalise(Vector3 *v1)
{
  float mag = vector3_mag(*v1);
  v1->x /= mag;
  v1->y /= mag;
  v1->z /= mag;
}

void vector3_add_normalise(Vector3 *v0, Vector3 v1)
{
  *v0 = vector3_scale(*v0, 1/sqrt(Sq(v0->x+v1.x) + Sq(v0->y+v1.y) + Sq(v0->z+v1.z)));
}

Vector3 vector3_cross(Vector3 v1, Vector3 v2)
{
  return (Vector3){(v1.y*v2.z - v1.z*v2.y), -(v1.x*v2.z - v1.z*v2.x), (v1.x*v2.y - v1.y*v2.x)};
}

float vector3_angle(Vector3 v1, Vector3 v2)
{
  float dot = vector3_dot(v1, v2);
  float mag1 = vector3_mag(v1);
  float mag2 = vector3_mag(v2);
  float cos_angle = dot/(mag1*mag2);
  return acosf(cos_angle);
}

inline void vector3_rotx(Vector3 *v0, float a)
{
  float y = v0->y*cos(a) - v0->z*sin(a);
  float z = v0->y*sin(a) + v0->z*cos(a);
  v0->y = y;
  v0->z = z;
}

inline void vector3_roty(Vector3 *v0, float a)
{
  float x =  v0->x*cos(a) + v0->z*sin(a);
  float z = -v0->x*sin(a) + v0->z*cos(a);
  v0->x = x;
  v0->z = z;
}

inline void vector3_rotz(Vector3 *v0, float a)
{
  float x = v0->x*cos(a) - v0->y*sin(a);
  float y = v0->x*sin(a) + v0->y*cos(a);
  v0->x = x;
  v0->y = y;
}


/** Move a towards b by alpha
 */
Vector3 vector3_lerp(Vector3 *a, Vector3 *b, float alpha)
{
  return vector3_add(vector3_scale(*a, 1-alpha), vector3_scale(*b, alpha));
}

Vector3 vector3_negate(Vector3 v0)
{
  return (Vector3){-v0.x, -v0.y, -v0.z};
}

Vector3 vector3_negate_normalise(Vector3 v0)
{
  float mag = -vector3_mag(v0);
  return (Vector3){v0.x/mag, v0.y/mag, v0.z/mag};
}

/** Reflect a vector about a normal
 */
Vector3 vector3_reflect(Vector3 reflectee, Vector3 reflector)
{
  // r = d - 2(d . n)n
  return vector3_sub(reflectee, vector3_scale(reflector, 2*vector3_dot(reflectee, reflector)));
}

void vector3_clamp(Vector3 *v0, float min, float max)
{
  v0->x = MIN(MAX(v0->x, min), max);
  v0->y = MIN(MAX(v0->y, min), max);
  v0->z = MIN(MAX(v0->z, min), max);
}

void matrix_mult(int h1, int w1, int h2, int w2, float m1m2[][w2], float m1[][w1], float m2[][w2])
{
  if (w1 != h2)
  {
    printf("MATRIX SIZE MISMATCH: w1 (%d) != h2 (%d)\n", w1, h2);
    exit(1);
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

void vector3_add8()
{

}
