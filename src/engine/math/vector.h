#ifndef VECTOR_H
#define VECTOR_H

#define Sq(x) ((x)*(x))

typedef struct Vector2 {
  double x, y;
  double w;
} Vector2;

typedef struct Vector3 {
  double x, y, z;
} Vector3;

// VECTOR2
//----------------------------------------------
float vector2_dot(Vector2 v1, Vector2 v2);
float vector2_angle(Vector2 v1, Vector2 v2);
float vector2_dist(Vector2 v1, Vector2 v2);
void vector2_normalise(Vector2 *v1);

Vector2 vector2_add(Vector2 v1, Vector2 v2);
Vector2 vector2_sub(Vector2 v1, Vector2 v2);
Vector2 vector2_scale(Vector2 v, float scalar);
Vector2 vector2_lerp(Vector2 v1, Vector2 v2, float alpha);

//----------------------------------------------


// VECTOR3
//----------------------------------------------
float vector3_dist(Vector3 p1, Vector3 p2);
float vector3_dot(Vector3 v1, Vector3 v2);
float vector3_angle(Vector3 v1, Vector3 v2);
float vector3_mag(Vector3 v);
void vector3_normalise(Vector3 *v1);

Vector3 vector3_add(Vector3 v1, Vector3 v2);
Vector3 vector3_sub(Vector3 v1, Vector3 v2);
Vector3 vector3_scale(Vector3 v, float scalar);
Vector3 vector3_cross(Vector3 v1, Vector3 v2);
//----------------------------------------------

void matrix_mult(int w1, int h1, int w2, int h2, float m1m2[][w2], float m1[][w1], float m2[][w2]);

#endif /* VECTOR_H */