
#include "camera.h"

Camera create_camera(void)
{
  Camera cam;
  cam.pos = (Vector3){0, 0, -10};
  cam.dir = (Vector3){0, 0, 1};
  cam.R = (Vector3){0, 0, 0};
  cam.fov = 500;
  return cam;
}
