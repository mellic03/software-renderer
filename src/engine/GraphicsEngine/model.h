#ifndef MODEL_H
#define MODEL_H

#ifdef __unix__
  #include <SDL2/SDL.h>
#elif defined(_WIN32) || defined(WIN32)
  #define SDL_MAIN_HANDLED
  #include <SDL.h>
#endif

#include <stdbool.h>

#include "../math/enginemath.h"

typedef enum {SHADE_NONE, SIMD_SHADE_NONE, SHADE_FLAT, SHADE_GOURAUD, SHADE_PHONG} ShaderType;



typedef struct {

  Vector3 ambient, diffuse;
  Vector3 specular;
  float specular_exponent;
  Vector3 emissive;

} Material;



typedef struct {

  Vector3 vertices[3];

  Vector2 proj_verts[3];

  Vector3 normals[3];
  Vector3 face_normal;
  Vector2 uvs[3];

  float bar0, bar1, bar2;
  
  SDL_Surface *texture, *normal_map;
  Material *material;
  
  int vertex_indices[3]; // Used only in load_polygons for generating vertex normals
  int mat_index; // index of material to use

} Polygon;



typedef struct {
  
  Vector3 pos;

  float xmin, xmax, ymin, ymax, zmin, zmax;

  ShaderType shade_style;

  int vertex_count, normal_count, uv_count;

  Vector3 *vertices;
  Vector3 *vertex_normals;
  Vector3 *vertex_normals_worldspace;
  Vector3 *vertex_normals_viewspace;

  int poly_count;
  Polygon *polygons;

  int mat_count;
  SDL_Surface **textures;
  SDL_Surface **normal_maps;
  Material *materials;

} Model;

void GE_material_init(Material *material);

void GE_model_free(Model *model);
Model *GE_model_load(char *filepath);


void GE_model_rotx(Model *model, float r);
void GE_model_roty(Model *model, float r);
void GE_model_rotz(Model *model, float r);
void GE_model_translate(Model *model, float x, float y, float z);
void GE_model_scale(Model *model, float alpha);
void GE_model_scale_xyz(Model *model, float x, float y, float z);




#endif /* MODEL_H */