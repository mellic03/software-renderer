#ifndef ENGINE_H
#define ENGINE_H
#include <stdlib.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

#include "vector.h"
#include "camera.h"
#include "screen.h"

#define RENDER_DISTANCE 500

#define MIN(a, b) (a<b?a:b)
#define MAX(a, b) (a>b?a:b)


// GLOBALS
//----------------------------------------
extern Vector3 camera_pos;
extern Vector3 lightsource;

//----------------------------------------

extern uint8_t pixels[SCREEN_WIDTH * SCREEN_HEIGHT * 4];
extern int z_buffer[SCREEN_WIDTH * SCREEN_HEIGHT];
extern SDL_Texture *window_texture;

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

typedef struct Pixel {
  int r, g, b, a;
} Pixel;

Model load_model(char *filepath);
void draw_model(Camera cam, Model model);

void clear_screen(void);
void render_screen(SDL_Renderer *ren);

Vector2 project_coordinate(Camera cam, Vector3 pt);
Vector2 project_coordinate_without_cblas(Camera cam, Vector3 pt);

void line_3d(SDL_Renderer *renderer, Camera cam, Vector3 p1, Vector3 p2);

void rotate_x(Model model, float rotation);
void rotate_y(Model model, float r);
void rotate_z(Model model, float r);
void rotate_point(Vector3 *pt, float x, float y, float z);
void translate_model(Model *model, float x, float y, float z);

void translate_world(float x, float y, float z);
void rotate_world(float x, float y, float z);
void reset_translation(void);
void reset_rotation(void);

#endif /* ENGINE_H */