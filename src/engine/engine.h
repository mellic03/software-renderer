#ifndef ENGINE_H
#define ENGINE_H

#include "vector.h"
#include "camera.h"
#include "screen.h"

#define RENDER_DISTANCE 100

#define MIN(a, b) (a<b?a:b)
#define MAX(a, b) (a>b?a:b)

extern SDL_Point *point_array;

typedef struct Model {

  Vector3 pos;

  int vertex_count ;
  Vector3 *vertices;

  int *vertex_normals; // Array of vertex normals
  
  int polygon_count;
  int **polygon_order; // Array of arrays of indices of vertices in *vertices

  // vertex_normals[n] = (Vector2){0.7563, -0.0299, 0.6535}
  // the vertex norm of vertices[n] will be vertex_normals[n];
  // therefore the size of the vertex_normals array should be the same
  // size as the vertices array.

  //  f 943/1034/1817 965/1062/1817 967/1064/1817
  // Vertex #943's normal vector is normal vector #1817

} Model;

Model load_model(char *filepath);
void draw_model(SDL_Renderer *ren, Camera cam, Model model);

Vector2 project_coordinate(Camera cam, Vector3 pt);
Vector2 project_coordinate_without_cblas(Camera cam, Vector3 pt);

void line_3d(SDL_Renderer *renderer, Camera cam, Vector3 p1, Vector3 p2);

void rotate_x(Model model, float rotation);
void rotate_y(Model model, float r);
void rotate_z(Model model, float r);

void translate_world(float x, float y, float z);
void rotate_world(float x, float y, float z);
void reset_translation(void);
void reset_rotation(void);

#endif /* ENGINE_H */