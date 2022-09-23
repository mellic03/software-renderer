#include <SDL2/SDL.h>

#include "vector.h"
#include "camera.h"
#include "screen.h"

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

extern Vector3 camera_pos;
//----------------------------------------

typedef struct {
  Vector3 vertices[3];
  Vector3 normal_vector;
  Vector3 fill;
  Vector2 texture_coords[3];
} Polygon;

typedef struct {
  Vector3 pos;
  int vertex_count;
  int normal_count;
  int tex_coord_count;
  int polygon_count;

  Polygon *polygons; // Array of polygons
  
  Vector3 fill;
  SDL_Surface *texture;

} Model;



void pixel(int x, int y, Uint8 r, Uint8 g, Uint8 b);


Model load_model(char *filepath, char *material);
void draw_model(Camera cam, Model *model);
void fill_model(Model *model, int r, int g, int b);

void rotate_x(Model *model, float rotation);
void rotate_y(Model *model, float r);
void rotate_z(Model model, float r);
void rotate_point(Vector3 *pt, float x, float y, float z);
void translate_model(Model *model, float x, float y, float z);
void set_position_model(Model *model, Vector3 pos);

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