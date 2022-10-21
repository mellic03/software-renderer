#include "../math/vector.h"

#ifndef CAMERA_H
#define CAMERA_H

#define SCREEN_WDTH 1000
#define SCREEN_HGHT 1000
#define HALF_SCREEN_WDTH (SCREEN_WDTH/2)
#define HALF_SCREEN_HGHT (SCREEN_HGHT/2)

extern float VIEWPLANE_WDTH;
extern float VIEWPLANE_HGHT;

typedef struct Camera {
  
  Vector3 rot; // rotation
  Vector3 dir;
  Vector3 *pos;
  Vector3 *vel;

  float translation_matrix[16];

  float vfov;

  int speed;

  Vector3 left_plane, right_plane;
  Vector3 top_plane, bottom_plane;
  Vector3 near_plane, far_plane;

} Camera;

void calculate_frustum(Camera *cam, float angle);
Camera *create_camera(void);

#endif /* CAMERA_H */
