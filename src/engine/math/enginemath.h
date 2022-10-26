#ifndef ENGINEMATH_H
#define ENGINEMATH_H


// GENERAL UTILITIES
//-----------------------------------------
#define MIN(a, b) (a<b ? a : b)
#define MAX(a, b) (a>b ? a : b)
#define Sq(x) ((x)*(x))

float clamp(float n, float min, float max);
//-----------------------------------------


// VECTOR2
//----------------------------------------------
typedef struct Vector2 {
  double x, y;
  double w;
} Vector2;

float vector2_dot(Vector2 v0, Vector2 v1);
float vector2_angle(Vector2 v0, Vector2 v1);
float vector2_dist(Vector2 v0, Vector2 v1);
float vector2_cross(Vector2 v0, Vector2 v1);
void vector2_normalise(Vector2 *v0);

Vector2 vector2_add(Vector2 v0, Vector2 v1);
Vector2 vector2_sub(Vector2 v0, Vector2 v1);
Vector2 vector2_scale(Vector2 v0, float scalar);
Vector2 vector2_lerp(Vector2 v0, Vector2 v1, float alpha);
//----------------------------------------------


// VECTOR3
//----------------------------------------------
typedef struct Vector3 {
  double x, y, z;
} Vector3;

float vector3_dist(Vector3 v0, Vector3 v1);
float vector3_dot(Vector3 v0, Vector3 v1);
float vector3_angle(Vector3 v0, Vector3 v1);
float vector3_mag(Vector3 v0);
void vector3_normalise(Vector3 *v0);
void vector3_rotx(Vector3 *v0, float a);
void vector3_roty(Vector3 *v0, float a);
void vector3_rotz(Vector3 *v0, float a);
void vector3_clamp(Vector3 *v0, float min, float max);

Vector3 vector3_add(Vector3 v0, Vector3 v1);
Vector3 vector3_sub(Vector3 v0, Vector3 v1);
Vector3 vector3_scale(Vector3 v0, float scalar);
Vector3 vector3_cross(Vector3 v0, Vector3 v1);
Vector3 vector3_lerp(Vector3 *v0, Vector3 *v1, float alpha);
Vector3 vector3_reflect(Vector3 reflectee, Vector3 reflector);
Vector3 vector3_negate(Vector3 v0);
//----------------------------------------------

void matrix_mult(int w1, int h1, int w2, int h2, float m1m2[][w2], float m1[][w1], float m2[][w2]);

#endif /* ENGINEMATH_H */