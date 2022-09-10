#include "vector.h"

#ifndef CAMERA_H
#define CAMERA_H

typedef struct Camera {
  Vector3 pos;
  Vector3 R; // rotation
  float fov; // field of view

} Camera;

#endif /* CAMERA_H */
