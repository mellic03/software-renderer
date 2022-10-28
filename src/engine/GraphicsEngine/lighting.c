#include "graphics.h"
#include "lighting.h"
#include "../math/enginemath.h"

LightSource *lightsource_head = NULL;

Vector3 GE_default_vertex_shader(Polygon *tri, Vector3 vert_pos, LightSource lightsource)
{
  return (Vector3){1, 1, 1};
}

LightSource *GE_lightsource_init(LightType light_type)
{
  LightSource *light = (LightSource *)calloc(1, sizeof(LightSource));

  light->vert_shader = &GE_default_vertex_shader;

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
  if (lightsource_head == NULL)
  {
    lightsource_head = GE_lightsource_init(light_type);
    return lightsource_head;
  }

  LightSource *new = GE_lightsource_init(light_type);
  new->next = lightsource_head;
  lightsource_head = new;

  return lightsource_head;
}


/** Perform fragment shader calculations for all lightsources
 * \param frag_out output light value
 */
Vector3 GE_lightsource_perform_fragment(Polygon *tri, Vector3 frag_pos)
{
  Vector3 output = (Vector3){0, 0, 0};

  LightSource *lightsource = lightsource_head;

  while (lightsource != NULL)
  {
    output = vector3_add(output, lightsource->frag_shader(tri, frag_pos, *lightsource));
    lightsource = lightsource->next;
  }

  vector3_clamp(&output, 0, 1);
  return output;
}

void GE_lightsource_perform_vertex(Polygon *tri, Vector3 *out1, Vector3 *out2, Vector3 *out3)
{
  *out1 = (Vector3){0, 0, 0};
  *out2 = (Vector3){0, 0, 0};
  *out3 = (Vector3){0, 0, 0};

  LightSource *lightsource = lightsource_head;

  while (lightsource != NULL)
  {
    *out1 = vector3_add(*out1, lightsource->vert_shader(tri, tri->vertices[0], *lightsource));
    *out2 = vector3_add(*out2, lightsource->vert_shader(tri, tri->vertices[1], *lightsource));
    *out3 = vector3_add(*out3, lightsource->vert_shader(tri, tri->vertices[2], *lightsource));
    lightsource = lightsource->next;
  }

  vector3_clamp(out1, 0, 1);
  vector3_clamp(out2, 0, 1);
  vector3_clamp(out3, 0, 1);
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
  if (lightsource_head == NULL)
    return;
  
  LightSource *lightsource = lightsource_head;

  while (lightsource != NULL)
  {
    GE_lightsource_world_to_view(lightsource);
    lightsource = lightsource->next;
  }
}