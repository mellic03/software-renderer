#include <math.h>

#include "camera.h"
#include "screen.h"
#include "engine.h"

Camera create_camera(void)
{
  Camera cam;
  cam.pos = (Vector3){0, 0, -10};
  cam.dir = (Vector3){0, 0, 1};
  cam.R = (Vector3){0, 0, 0};


  cam.near_norm = (Vector3){0, 0, 1};
  cam.far_norm = (Vector3){0, 0, -1};

  cam.l_norm = (Vector3){1/sqrt(2), 0, 1/sqrt(2)};
  cam.r_norm = (Vector3){-1/sqrt(2), 0, 1/sqrt(2)};
  cam.t_norm = (Vector3){0, 1/sqrt(2), 1/sqrt(2)};
  cam.b_norm = (Vector3){0, -1/sqrt(2), 1/sqrt(2)};


  camera_pos = (Vector3)cam.pos;
  return cam;
}
