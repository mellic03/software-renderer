#ifndef ENGINE_H
#define ENGINE_H

#include "vector.h"
#include "camera.h"
#include "shapes.h"

typedef struct Model {
  int polygon_count;
  Vector2 **vertices;
  int triangle_count;
  int **triangle_order;
} Model;

Vector3 *load_vertices(int *v_count, char filepath[]);
void load_vertex_order(int v_count, int **orders, char filepath[]);
Vector2 project_coordinate(Camera cam, Vector3 pt);

#endif /* ENGINE_H */