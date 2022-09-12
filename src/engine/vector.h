#ifndef VECTOR_H
#define VECTOR_H

typedef struct Vector2 {
  float x, y;
} Vector2;

typedef struct Vector3 {
  float x, y, z;
} Vector3;

float vector2_dot(Vector2 v1, Vector2 v2);
float vector2_angle(Vector2 v1, Vector2 v2);

Vector3 vector3_add(Vector3 v1, Vector3 v2);
Vector3 vector3_sub(Vector3 v1, Vector3 v2);
float vector3_dot(Vector3 v1, Vector3 v2);
float vector3_cross(Vector3 v1, Vector3 v2);
float vector3_angle(Vector3 v1, Vector3 v2);
int vector3_dist(Vector3 p1, Vector3 p2);
void matrix_mult(int w1, int h1, int w2, int h2, float m1m2[][w2], float m1[h1][w1], float m2[h2][w2]);

#endif /* VECTOR_H */