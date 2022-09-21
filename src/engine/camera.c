#include <math.h>

#include "camera.h"
#include "screen.h"
#include "engine.h"

Camera create_camera(void)
{
  Camera cam;
  cam.pos = (Vector3){0, 0, -10};
  cam.dir = (Vector3){0, 0, 1};
  cam.rot = (Vector3){0, 0, 0};

  cam.speed = 10;

  cam.near_norm = (Vector3){0, 0, 1};
  cam.far_norm = (Vector3){0, 0, -1};

  
  cam.l_norm = (Vector3){1/sqrt(2), 0, 1/sqrt(2)};
  cam.r_norm = (Vector3){-1/sqrt(2), 0, 1/sqrt(2)};
  cam.t_norm = (Vector3){0, 1/sqrt(2), 1/sqrt(2)};
  cam.b_norm = (Vector3){0, -1/sqrt(2), 1/sqrt(2)};

  // float vfov = 1.57;

  // float aspect = SCREEN_WIDTH / SCREEN_HEIGHT;

  // float z_near = 1;
  // float hh = tan(vfov/2) * z_near;
  // float hw = hh * aspect;

  // Vector3 top_left = {-hw, hh, 1};
  // Vector3 top_right = {hw, hh, 1};
  // Vector3 bottom_right = {hw, -hh, 1};
  // Vector3 bottom_left = {-hw, -hh, 1};

  // vector3_normalise(&top_left);
  // vector3_normalise(&top_right);
  // vector3_normalise(&bottom_left);
  // vector3_normalise(&bottom_right);

  // cam.t_norm = vector3_cross(top_left, top_right);
  // cam.r_norm = vector3_cross(top_right, bottom_right);
  // cam.b_norm = vector3_cross(bottom_left, bottom_right);
  // cam.l_norm = vector3_cross(top_left, bottom_left);

  // vector3_normalise(&cam.t_norm);
  // vector3_normalise(&cam.r_norm);
  // vector3_normalise(&cam.b_norm);
  // vector3_normalise(&cam.l_norm);

  return cam;
}
