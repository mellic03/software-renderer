#include <math.h>

#include "graphics.h"
#include "camera.h"

Camera create_camera(void)
{
  Camera cam;
  cam.pos = (Vector3){0, -4.5, -4};
  cam.dir = (Vector3){0, 0, 1};
  cam.rot = (Vector3){0, 0, 0};

  cam.speed = 1;

  cam.near_norm = (Vector3){0, 0, 1};
  cam.far_norm = (Vector3){0, 0, -1};
  
  cam.l_norm = (Vector3){1/sqrt(2), 0, 1/sqrt(2)};
  cam.r_norm = (Vector3){-1/sqrt(2), 0, 1/sqrt(2)};
  cam.t_norm = (Vector3){0, 1/sqrt(2), 1/sqrt(2)};
  cam.b_norm = (Vector3){0, -1/sqrt(2), 1/sqrt(2)};

  return cam;
}
