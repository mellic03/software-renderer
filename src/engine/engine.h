#ifndef ENGINE_H
#define ENGINE_H

#include "vector.h"
#include "camera.h"
#include "screen.h"

#define RENDER_DISTANCE 50

#define MIN(a, b) (a<b?a:b)
#define MAX(a, b) (a>b?a:b)

extern SDL_Point *fill_matrix;

typedef struct Polygon {
  Vector3 vertices[3]; // Array of three vertices
  Vector3 normal_vector;
} Polygon;

typedef struct Model {
  Vector3 pos;
  int vertex_count;
  int normal_count;
  int polygon_count;
  Polygon *polygons; // Array of polygons
  Vector3 fill;
  Vector3 stroke;
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