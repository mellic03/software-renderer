#ifdef __unix__
  #include <SDL2/SDL.h>
#elif defined(_WIN32) || defined(WIN32)
  #define SDL_MAIN_HANDLED
  #include <SDL.h>
#endif

#include "camera.h"
#include "../math/vector.h"
#include "../screen.h"

#ifndef ENGINE_H
#define ENGINE_H

#define RENDER_DISTANCE 50

#define MIN(a, b) (a<b ? a : b)
#define MAX(a, b) (a>b ? a : b)

// GLOBALS
//----------------------------------------
extern SDL_Surface *pixel_array;
extern double delta_time;
extern Vector3 lightsource;
//----------------------------------------

typedef enum {SHADE_NONE, SHADE_FLAT, SHADE_SMOOTH} ShaderType;

typedef struct {

  Vector3 vertices[3];
  Vector3 normals[3];
  Vector3 og_vertices[3];

  int vertex_indices[3]; // Used only in load_polygons for generating vertex normals

  Vector3 face_normal;

  Vector2 uvs[3];
  int mat_index; // index of material to use

} Polygon;

typedef struct {
  
  Vector3 pos;

  ShaderType shade_style;

  int vertex_count;
  int normal_count;
  int uv_count;

  Vector3 lightsource;

  Vector3 *vertex_normals;

  int poly_count;
  Polygon *polygons; // Array of polygons
  Vector3 *vertices;

  int mat_count;
  char **mat_names;
  SDL_Surface **materials;

} Model;


void pixel(int x, int y, Uint8 r, Uint8 g, Uint8 b);

void draw_model(Camera cam, Model *model);

void rotate_x(Model *model, float rotation);
void rotate_y(Model *model, float r);
void rotate_z(Model model, float r);
void rotate_point(Vector3 *pt, float x, float y, float z);
void translate_model(Model *model, float x, float y, float z);
void scale(Model *model, float alpha);


void clear_screen(Uint8 r, Uint8 g, Uint8 b);
void render_screen(SDL_Renderer *ren);


// INTERNAL
//-----------------------------------------
Vector2 project_coordinate(Vector3 *pt);
int clip_against_plane(Vector3 plane_normal, int poly_count, Polygon *unclipped_triangles, Polygon *clipped_triangles);
int clip_polygon(Vector3 plane_normal, Polygon *tri_in, Polygon *tri_out1, Polygon *tri_out2);
int points_inside_plane(Vector3 plane_normal, Polygon *tri, int *index_of_inside, int *index_of_outside);
Vector3 line_plane_intersect(Vector3 plane_normal, Vector3 p1, Vector3 p2, float *t);
//-----------------------------------------



#endif /* ENGINE_H */