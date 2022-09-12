#ifndef ENGINE_H
#define ENGINE_H

#include "vector.h"
#include "camera.h"

#define MIN(a, b) (a<b?a:b)
#define MAX(a, b) (a>b?a:b)

typedef struct Model {

  Vector3 pos;

  int vertex_count ;
  Vector3 *vertices;
  
  int polygon_count;
  int **polygon_order; // array of arrays of three indices of vertices in *vertices

} Model;

Vector2 project_coordinate(Camera cam, Vector3 pt);
Model load_model(char *filepath);
void triangle_2d_filled(SDL_Renderer *ren, Vector2 *p1, Vector2 *p2, Vector2 *p3);

Vector2 vector2_add(Vector2 v1, Vector2 v2);

void line_2d(SDL_Renderer *renderer, Vector2 p1, Vector2 p2);
void line_3d(SDL_Renderer *renderer, Camera cam, Vector3 p1, Vector3 p2);

void draw_model(SDL_Renderer *ren, Camera cam, Model model);
void rotate_x(Model model, float rotation);
void rotate_y(Model model, float r);
void rotate_z(Model model, float r);

void rotate_point_x(Vector3 *point, float x);
void rotate_point_y(Vector3 *point, float y);
void rotate_point_z(Vector3 *point, float z);


void translate_world(float x, float y, float z);
void rotate_world(float x, float y, float z);
void reset_translation(void);
void reset_rotation(void);

#endif /* ENGINE_H */