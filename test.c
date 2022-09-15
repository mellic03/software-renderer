#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Vector3 {
  float x, y, z;
} Vector3;

typedef struct Polygon {
  Vector3 *vertices; // Array of three vertices
  Vector3 normal_vector;
} Polygon;

typedef struct Model {
  Vector3 pos;
  int vertex_count;
  int normal_count;
  int polygon_count;
  Polygon *polygons; // Array of polygons
} Model;

void change_data(Model *model)
{
  
}


int main()
{
  Model model;
  change_data(&model);

  return 0;
}