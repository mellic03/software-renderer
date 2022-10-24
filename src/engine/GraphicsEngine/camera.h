#include "../math/vector.h"

#ifndef CAMERA_H
#define CAMERA_H

#define SCREEN_WDTH 1024
#define SCREEN_HGHT 1024

#define REN_RES_X ((SCREEN_WDTH) >> 0)
#define REN_RES_Y ((SCREEN_HGHT) >> 0)

#define HALF_SCREEN_WDTH ((REN_RES_X)/2)
#define HALF_SCREEN_HGHT ((REN_RES_Y)/2)

extern float VIEWPLANE_WDTH;
extern float VIEWPLANE_HGHT;

typedef struct Camera {
  
  Vector3 rot; // rotation
  Vector3 dir;
  Vector3 *pos;
  Vector3 *vel;

  float vfov;

  int speed;

  Vector3 left_plane, right_plane;
  Vector3 top_plane, bottom_plane;
  Vector3 near_plane, far_plane;

} Camera;

void calculate_frustum(Camera *cam, float angle);
Camera *create_camera(void);

#endif /* CAMERA_H */
