#include <math.h>

#include "graphics.h"
#include "camera.h"

Camera *create_camera(void)
{
  Camera *cam = (Camera *)calloc(1, sizeof(Camera));
  // cam->pos = (Vector3){0, 0, 0};
  cam->dir = (Vector3){0, 0, 1};
  cam->rot = (Vector3){0, 0, 0};

  cam->speed = 1;

  cam->left_plane = (Vector3){1/sqrt(2), 0, 1/sqrt(2)};
  cam->right_plane = (Vector3){-1/sqrt(2), 0, 1/sqrt(2)};
  cam->top_plane = (Vector3){0, 1/sqrt(2), 1/sqrt(2)};
  cam->bottom_plane = (Vector3){0, -1/sqrt(2), 1/sqrt(2)};

  cam->near_plane = (Vector3){0, 0, 1};
  cam->far_plane = (Vector3){0, 0, -1};


  return cam;
}
