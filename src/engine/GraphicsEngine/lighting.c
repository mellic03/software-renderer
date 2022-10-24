#include "graphics.h"


void GE_lightsource_init(LightSource *lightsource, LightType light_type)
{
  lightsource = (LightSource *)calloc(1, sizeof(LightSource));
  lightsource->light_type = light_type;
  lightsource->cutoff_angle = 0.5;
  lightsource->dir = (Vector3){0, 0, 1};
}


LightSource GE_lightsource_world_to_view(LightSource *lightsource_world)
{
  LightSource lightsource_view = *lightsource_world;
  lightsource_view.pos = vector3_sub(lightsource_view.pos, *GE_cam->pos);
  rotate_point(&lightsource_view.pos, GE_cam->rot.x, GE_cam->rot.y, 0);
  rotate_point(&lightsource_view.dir, GE_cam->rot.x, GE_cam->rot.y, 0);
  return lightsource_view;
}