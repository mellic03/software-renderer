#include "graphics.h"


void GE_lightsource_init(LightSource *lightsource, LightType light_type)
{
  lightsource->light_type = light_type;
  lightsource->inner_cutoff = 0.6;
  lightsource->outer_cutoff = 0.4;
  lightsource->dir = (Vector3){0, 0, 1};
  lightsource->colour = (Vector3){1, 1, 1};
  lightsource->pos = (Vector3){0, 0, 0};
}


LightSource GE_lightsource_world_to_view(LightSource *lightsource_world)
{
  LightSource lightsource_view = *lightsource_world;
  lightsource_view.pos = vector3_sub(lightsource_view.pos, *GE_cam->pos);
  rotate_point(&lightsource_view.pos, GE_cam->rot.x, GE_cam->rot.y, 0);
  rotate_point(&lightsource_view.dir, GE_cam->rot.x, GE_cam->rot.y, 0);
  return lightsource_view;
}