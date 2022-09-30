#ifndef PHYSICS_H
#define PHYSICS_H

#include "../math/vector.h"

typedef struct {

  Vector3 *pos;
  float xwidth, ywidth, zwidth;

} BoxCollider;


typedef struct {

  Vector3 *pos;
  float radius;

} SphereCollider;


#endif /* PHYSICS_H */