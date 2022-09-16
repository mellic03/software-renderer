#include "camera.h"
#include "screen.h"
#include "math.h"
#include "stdlib.h"
#include "stdio.h"

Camera create_camera(void)
{
  Camera cam;
  cam.pos = (Vector3){0, 0, -10};
  cam.dir = (Vector3){0, 0, 1};
  cam.R = (Vector3){0, 0, 0};
  cam.fov = 500;

  float aspect_ratio = SCREEN_WIDTH / SCREEN_HEIGHT;
  cam.vfov = 1.57;
  cam.hfov = cam.vfov * aspect_ratio;


  int near_plane_dist = 15;
  int far_plane_dist = 20;

  float near_plane_height = near_plane_dist * tan(cam.field_of_view);
  
  // Far plane height gives y component of top left and right vectors
  // z component is the distance

  float height = near_plane_dist * tan(cam.vfov * 0.5);
  float width = height*aspect_ratio;

  Vector3 top_l = (Vector3){1, height, -width};
  Vector3 top_r = (Vector3){1, height, width};
  Vector3 bot_l = (Vector3){1, -height, -width};
  Vector3 bot_r = (Vector3){1, -height, width};

  cam.left_normal  = vector3_cross(top_l, bot_l);
  cam.right_normal = vector3_cross(top_r, bot_r);
  cam.top_normal   = vector3_cross(top_l, top_r);
  cam.bot_normal   = vector3_cross(bot_l, bot_r);

  vector3_normalise(&cam.left_normal);
  vector3_normalise(&cam.right_normal);
  vector3_normalise(&cam.top_normal);
  vector3_normalise(&cam.bot_normal);

  return cam;
}
