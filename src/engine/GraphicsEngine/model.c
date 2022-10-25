#ifdef __unix__
  #include <SDL2/SDL.h>
#elif defined(_WIN32) || defined(WIN32)
  #define SDL_MAIN_HANDLED
  #include <SDL.h>
#endif
#include "model.h"


void GE_material_init(Material *material)
{
  material = (Material *)calloc(1, sizeof(Material));
}

void GE_model_free(Model *model)
{
  free(model->polygons);

  free(model->vertices);
  free(model->vertex_normals);
  for (int i=0; i<model->mat_count*2; i++)
    SDL_FreeSurface(model->textures[i]);
  free(model->textures);
  free(model);
}

void GE_model_translate(Model *model, float x, float y, float z)
{
  model->pos = vector3_add(model->pos, (Vector3){x, y, z});

  for (int i=0; i<model->poly_count; i++)
    for (int j=0; j<3; j++)
    {
      model->polygons[i].vertices[j].x += x;
      model->polygons[i].vertices[j].y += y;
      model->polygons[i].vertices[j].z += z;
      model->polygons[i].og_vertices[j].x += x;
      model->polygons[i].og_vertices[j].y += y;
      model->polygons[i].og_vertices[j].z += z;
    }
}

void GE_model_scale(Model *model, float alpha)
{
  for (int i=0; i<model->poly_count; i++)
    for (int j=0; j<3; j++)
      model->polygons[i].vertices[j] = vector3_scale(model->polygons[i].vertices[j], alpha);
}

void GE_model_scale_xyz(Model *model, float x, float y, float z)
{
  for (int i=0; i<model->poly_count; i++)
    for (int j=0; j<3; j++)
    {
      model->polygons[i].vertices[j].x *= x;
      model->polygons[i].vertices[j].y *= y;
      model->polygons[i].vertices[j].z *= z;
      model->polygons[i].og_vertices[j].x *= x;
      model->polygons[i].og_vertices[j].y *= y;
      model->polygons[i].og_vertices[j].z *= z;
    }
}

void GE_model_rotx(Model *model, float alpha)
{
  Vector3 model_pos = model->pos;
  GE_model_translate(model, -model_pos.x, -model_pos.y, -model_pos.z);

  for (int i=0; i<model->poly_count; i++)
  {
    for (int j=0; j<3; j++)
      vector3_rotx(&model->polygons[i].vertices[j], alpha);
    vector3_rotx(&model->polygons[i].face_normal, alpha);
  }

  for (int i=0; i<model->vertex_count; i++)
    vector3_rotx(&model->vertex_normals[i], alpha);

  GE_model_translate(model, model_pos.x, model_pos.y, model_pos.z);
}

void GE_model_roty(Model *model, float alpha)
{
  Vector3 model_pos = model->pos;
  GE_model_translate(model, -model_pos.x, -model_pos.y, -model_pos.z);

  for (int i=0; i<model->poly_count; i++)
  {
    for (int j=0; j<3; j++)
      vector3_roty(&model->polygons[i].vertices[j], alpha);
    vector3_roty(&model->polygons[i].face_normal, alpha);
  }

  for (int i=0; i<model->vertex_count; i++)
    vector3_roty(&model->vertex_normals[i], alpha);

  GE_model_translate(model, model_pos.x, model_pos.y, model_pos.z);
}

void GE_model_rotz(Model *model, float alpha)
{
  Vector3 model_pos = model->pos;
  GE_model_translate(model, -model_pos.x, -model_pos.y, -model_pos.z);

  for (int i=0; i<model->poly_count; i++)
  {
    for (int j=0; j<3; j++)
      vector3_rotz(&model->polygons[i].vertices[j], alpha);
    vector3_rotz(&model->polygons[i].face_normal, alpha);
  }

  for (int i=0; i<model->vertex_count; i++)
    vector3_rotz(&model->vertex_normals[i], alpha);

  GE_model_translate(model, model_pos.x, model_pos.y, model_pos.z);
}

