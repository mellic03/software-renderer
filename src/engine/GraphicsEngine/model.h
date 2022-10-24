#ifndef MODEL_H
#define MODEL_H

#ifdef __unix__
  #include <SDL2/SDL.h>
#elif defined(_WIN32) || defined(WIN32)
  #define SDL_MAIN_HANDLED
  #include <SDL.h>
#endif

#include <stdbool.h>

#include "../math/vector.h"
#include "../math/fixedpoint/v3fp.h"

typedef enum {SHADE_NONE, SIMD_SHADE_NONE, SHADE_FLAT, SHADE_GOURAUD, SHADE_PHONG} ShaderType;



typedef struct {

  Vector3 ambient, diffuse;
  Vector3 specular;
  float specular_exponent;
  Vector3 emissive;

} Material;



typedef struct {

  Vector3 og_vertices[3];
  Vector3 vertices[3];

  Vector2 proj_verts[3];
  Vector2 og_proj_verts[3];

  Vector3 normals[3];
  Vector3 face_normal;
  Vector2 uvs[3];
  
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

  int poly_count;
  Polygon *polygons; // Array of polygons

  int mat_count;
  SDL_Surface **textures;
  SDL_Surface **normal_maps;
  Material *materials;

} Model;

void model_free(Model *model);
Model *model_load(char *filepath);

void GE_material_init(Material *material);


#endif /* MODEL_H */