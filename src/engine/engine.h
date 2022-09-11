#ifndef ENGINE_H
#define ENGINE_H

#include "vector.h"
#include "camera.h"

typedef struct Model {

  Vector3 pos;

  int vertex_count ;
  Vector3 *vertices;
  
  int polygon_count;
  int **polygon_order; // array of arrays of three indices of vertices in *vertices

} Model;

Vector2 project_coordinate(Camera cam, Vector3 pt);
Model load_model(char *filepath);

void draw_model(SDL_Renderer *ren, Camera cam, Model model);
void rotate_x(Model model, float rotation);
void rotate_y(Model model, float r);
void rotate_z(Model model, float r);

void translate_world(float x, float y, float z);
void rotate_world(float x, float y, float z);
void reset_translation(void);
void reset_rotation(void);

#endif /* ENGINE_H */