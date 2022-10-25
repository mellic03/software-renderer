#include "graphics.h"
#include "lighting.h"
#include "../math/enginemath.h"

static LightSource *head = NULL;

LightSource *GE_lightsource_init(LightType light_type)
{
  LightSource *light = (LightSource *)calloc(1, sizeof(LightSource));

  light->light_type = light_type;

  light->pos = (Vector3){0, 0, 0};
  light->colour = (Vector3){1, 1, 1};
  light->intensity = 0.5;

  if (light_type == GE_SPOTLIGHT)
  {
    light->dir = (Vector3){0, 0, 1};
    light->inner_cutoff = 0.9;
    light->outer_cutoff = 0.8;
  }

  light->next = NULL;

  return light;
}

LightSource *GE_lightsource_create(LightType light_type)
{
  if (head == NULL)
  {
    head = GE_lightsource_init(light_type);
    return head;
  }

  LightSource *new = GE_lightsource_init(light_type);
  new->next = head;
  head = new;

  return head;
}


/** Perform fragment shader calculations for all lightsources
 * \param frag_out output light value
 */
Vector3 GE_lightsource_perform_fragment(Polygon *tri, Vector3 frag_pos)
{
  Vector3 output = (Vector3){0, 0, 0};

  if (head == NULL)
    return (Vector3){1, 1, 1};
  
  LightSource *lightsource = head;

  while (lightsource != NULL)
  {
    output = vector3_add(output, lightsource->frag_shader(tri, frag_pos, *lightsource));
    lightsource = lightsource->next;
  }

  vector3_clamp(&output, 0, 1);

  return output;
}

void GE_lightsource_world_to_view(LightSource *lightsource)
{
  lightsource->pos_viewspace = vector3_sub(lightsource->pos, *GE_cam->pos);
  lightsource->dir_viewspace = lightsource->dir;

  vector3_roty(&lightsource->pos_viewspace, GE_cam->rot.y);
  vector3_rotx(&lightsource->pos_viewspace, GE_cam->rot.x);

  vector3_roty(&lightsource->dir_viewspace, GE_cam->rot.y);
  vector3_rotx(&lightsource->dir_viewspace, GE_cam->rot.x);
}

void GE_lightsource_world_to_view_all(void)
{
  if (head == NULL)
    return;
  
  LightSource *lightsource = head;

  while (lightsource != NULL)
  {
    GE_lightsource_world_to_view(lightsource);
    lightsource = lightsource->next;
  }
}