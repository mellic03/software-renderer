#include "vector.h"

#ifndef CAMERA_H
#define CAMERA_H

typedef struct Camera {
  Vector3 pos;
  Vector3 R; // rotation
  Vector3 dir;
  float fov; // field of view
  
  float field_of_view;
  float vfov; 
  float hfov;

  Vector3 left_normal;
  Vector3 right_normal;
  Vector3 top_normal;
  Vector3 bot_normal;
} Camera;

Camera create_camera(void);

#endif /* CAMERA_H */
