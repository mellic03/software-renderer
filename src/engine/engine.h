#ifndef ENGINE_H
#define ENGINE_H

#include "vector.h"
#include "camera.h"
#include "shapes.h"

typedef struct Model {
  int vertex_count ;
  Vector3 *vertices;
  int polygon_count;
  int **polygon_order; // array of arrays of three indices of vertices in *vertices
} Model;

Vector3 *load_vertices(int *polygon_count, char filepath[]);
void load_vertex_order(int polygon_count, int **polygon_order, char filepath[]);
Vector2 project_coordinate(Camera cam, Vector3 pt);
Model load_model(char *filepath);
void draw_model(SDL_Renderer *ren, Camera cam, Model model);

#endif /* ENGINE_H */