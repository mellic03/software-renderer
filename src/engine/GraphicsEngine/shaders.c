#include <math.h>
#include "graphics.h"
#include "model.h"
#include "lighting.h"
#include "../math/enginemath.h"


Vector3 shade_phong_spotlight(Polygon *tri, Vector3 frag_pos, LightSource lightsource)
{
  Vector3 diffuse = tri->material->diffuse;
  Vector3 specular = tri->material->specular;
  Vector3 output_lighting;

  Vector3 light_dir = vector3_sub(frag_pos, lightsource.pos_viewspace);
  vector3_normalise(&light_dir);

  float dot0 = vector3_dot(light_dir, lightsource.dir_viewspace);

  if (dot0 < lightsource.outer_cutoff)
    return AMBIENT_LIGHT;

  float theta = vector3_dot(light_dir, lightsource.dir_viewspace);
  float epsilon = lightsource.inner_cutoff - lightsource.outer_cutoff;
  float intensity = clamp((theta - lightsource.outer_cutoff)/epsilon, 0, 1);

  float diff = vector3_dot(vector3_negate(light_dir), tri->face_normal) + 1;
  diff /= 0.5 * vector3_dist(frag_pos, lightsource.pos_viewspace);
  diff = MAX(diff, 0);
  diff = MIN(diff, 1);
  diffuse = vector3_scale(diffuse, diff);
  diffuse = (Vector3){lightsource.colour.x * diffuse.x, lightsource.colour.y * diffuse.y, lightsource.colour.z * diffuse.z};

  Vector3 view_dir = vector3_negate(frag_pos);
  vector3_normalise(&view_dir);

  Vector3 reflect_dir = vector3_reflect(light_dir, tri->face_normal);
  vector3_normalise(&reflect_dir);

  float spec = pow(MAX(vector3_dot(view_dir, reflect_dir), 0.0f), tri->material->specular_exponent);
  specular = vector3_scale(specular, spec);
  specular = (Vector3){lightsource.colour.x * specular.x, lightsource.colour.y * specular.y, lightsource.colour.z * specular.z};

  diffuse = vector3_scale(diffuse, intensity);
  specular = vector3_scale(specular, intensity);
  output_lighting = vector3_add(diffuse, specular);
  output_lighting = vector3_add(output_lighting, AMBIENT_LIGHT);
  
  return output_lighting;
}



Vector3 shade_phong_pointlight(Polygon *tri, Vector3 frag_pos, LightSource lightsource)
{
  Vector3 diffuse = tri->material->diffuse;
  Vector3 specular = tri->material->specular;
  Vector3 output_lighting;

  Vector3 light_dir = vector3_sub(frag_pos, lightsource.pos_viewspace);
  vector3_normalise(&light_dir);

  Vector3 norm = tri->normals[0];
  norm = vector3_lerp(&norm, &tri->normals[1], tri->bar1);
  norm = vector3_lerp(&norm, &tri->normals[2], tri->bar2);

  float diff = vector3_dot(vector3_negate(light_dir), tri->face_normal) + 1;
  diff /= vector3_dist(frag_pos, lightsource.pos_viewspace);
  diffuse = vector3_scale(diffuse, diff);

  Vector3 view_dir = vector3_negate(frag_pos);
  vector3_normalise(&view_dir);

  Vector3 reflect_dir = vector3_reflect(light_dir, tri->face_normal);
  vector3_normalise(&reflect_dir);

  float spec = pow(MAX(vector3_dot(view_dir, reflect_dir), 0.0f), tri->material->specular_exponent);
  specular = vector3_scale(specular, spec);
  specular = (Vector3){lightsource.colour.x * specular.x, lightsource.colour.y * specular.y, lightsource.colour.z * specular.z};

  output_lighting = vector3_add(diffuse, specular);
  output_lighting = vector3_add(output_lighting, AMBIENT_LIGHT);
  
  return output_lighting;
}


