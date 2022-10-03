#ifndef PHYSICS_H
#define PHYSICS_H

#include "../math/vector.h"

#define G 0.01 // Gravitational constant

typedef struct {

  Vector3 *pos;
  float xwidth, ywidth, zwidth;

} BoxCollider;

typedef struct {

  Vector3 *pos;
  float radius;

} SphereCollider;

typedef struct {
  Vector3 *pos, dir;
} PlaneCollider;

void physics_attract(Vector3 *attracted_vel, Vector3 *attracted, Vector3 *attractor);
int box_colliding(BoxCollider *box1, BoxCollider *box2);
int sphere_colliding(SphereCollider *sphere1, SphereCollider *sphere2);
float sphere_plane_colliding(SphereCollider *sphere, PlaneCollider *plane);
#endif /* PHYSICS_H */