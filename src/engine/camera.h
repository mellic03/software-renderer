#include "vector.h"

#ifndef CAMERA_H
#define CAMERA_H


typedef struct Camera {
  Vector3 rot; // rotation
  Vector3 dir;
  Vector3 pos;
  Vector3 vel;

  int speed;

  Vector3 near_norm, far_norm;
  Vector3 l_norm, r_norm;
  Vector3 t_norm, b_norm;
} Camera;

Camera create_camera(void);

#endif /* CAMERA_H */
