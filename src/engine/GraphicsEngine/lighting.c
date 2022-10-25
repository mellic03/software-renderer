#include "graphics.h"
#include "lighting.h"
#include "../math/vector.h"

static LightSource *head = NULL;

void GE_lightsource_init(LightSource *lightsource, LightType light_type)
{
}

LightSource *GE_lightsource_create(LightType light_type)
{
  if (head == NULL)
  {
    head = (LightSource *)calloc(1, sizeof(LightSource));
    head->next = NULL;
    head->light_type = light_type;
    return head;
  }

  LightSource *new = (LightSource *)calloc(1, sizeof(LightSource));
  new->light_type = light_type;
  new->next = head;
  head = new;

  return head;
}


LightSource GE_lightsource_world_to_view(LightSource *lightsource_world)
{
  LightSource lightsource_view = *lightsource_world;
  lightsource_view.pos = vector3_sub(lightsource_view.pos, *GE_cam->pos);

  vector3_roty(&lightsource_view.pos, GE_cam->rot.y);
  vector3_rotx(&lightsource_view.pos, GE_cam->rot.x);

  vector3_roty(&lightsource_view.dir, GE_cam->rot.y);
  vector3_rotx(&lightsource_view.dir, GE_cam->rot.x);
  return lightsource_view;
}