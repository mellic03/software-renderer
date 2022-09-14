#include "vector.h"

#ifndef CAMERA_H
#define CAMERA_H

typedef struct Camera {
  Vector3 pos;
  Vector3 R; // rotation
  Vector3 dir;
  float fov; // field of view

} Camera;

Camera create_camera(void);

#endif /* CAMERA_H */
