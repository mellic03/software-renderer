#include <math.h>

#include "graphics.h"
#include "camera.h"

float VIEWPLANE_HGHT;
float VIEWPLANE_WDTH;

void calculate_frustum(Camera *cam, float angle)
{
  cam->vfov = angle;

  float aspect =  SCREEN_WDTH / SCREEN_HGHT;
  float znear = 1;
  float zfar = 1000;

  float hh = tan((cam->vfov)/2) * znear;
  float hw = hh * aspect;

  VIEWPLANE_HGHT = hh;
  VIEWPLANE_WDTH = hw;

  Vector3 nw = {-hw, hh, 1};
  Vector3 ne = {hw, hh, 1};
  Vector3 se = {hw, -hh, 1};
  Vector3 sw = {-hw, -hh, 1};

  cam->top_plane = vector3_cross(ne, nw);
  cam->right_plane = vector3_cross(se, ne);
  cam->bottom_plane = vector3_cross(sw, se);
  cam->left_plane = vector3_cross(nw, sw);

  vector3_normalise(&cam->top_plane);
  vector3_normalise(&cam->right_plane);
  vector3_normalise(&cam->bottom_plane);
  vector3_normalise(&cam->left_plane);

  cam->near_plane = (Vector3){0, 0, 1};
  cam->far_plane = (Vector3){0, 0, -1};


  // cam->left_plane = (Vector3){1/sqrt(2), 0, 1/sqrt(2)};
  // cam->right_plane = (Vector3){-1/sqrt(2), 0, 1/sqrt(2)};
  // cam->top_plane = (Vector3){0, 1/sqrt(2), 1/sqrt(2)};
  // cam->bottom_plane = (Vector3){0, -1/sqrt(2), 1/sqrt(2)};

  // cam->near_plane = (Vector3){0, 0, 1};
  // cam->far_plane = (Vector3){0, 0, -1};

}

Camera *create_camera(void)
{
  Camera *cam = (Camera *)calloc(1, sizeof(Camera));
  // cam->pos = (Vector3){0, 0, 0};
  cam->dir = (Vector3){0, 0, 1};
  cam->rot = (Vector3){0, 0, 0};

  cam->speed = 1;

  calculate_frustum(cam, 2.05);

  return cam;
}
