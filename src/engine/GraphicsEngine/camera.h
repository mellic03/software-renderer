#include "../math/vector.h"

#ifndef CAMERA_H
#define CAMERA_H

#define SCREEN_WIDTH 1000
#define SCREEN_HEIGHT 1000
#define HALF_SCREEN_WIDTH SCREEN_WIDTH/2
#define HALF_SCREEN_HEIGHT SCREEN_HEIGHT/2

typedef struct Camera {
  
  Vector3 rot; // rotation
  Vector3 dir;
  Vector3 *pos;
  Vector3 *vel;

  int speed;

  Vector3 near_norm, far_norm;
  Vector3 l_norm, r_norm;
  Vector3 t_norm, b_norm;

  Vector3 ml_norm, mr_norm;

} Camera;

Camera *create_camera(void);

#endif /* CAMERA_H */
