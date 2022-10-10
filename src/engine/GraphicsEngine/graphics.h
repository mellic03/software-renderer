#ifdef __unix__
  #include <SDL2/SDL.h>
#elif defined(_WIN32) || defined(WIN32)
  #define SDL_MAIN_HANDLED
  #include <SDL.h>
#endif

#include "camera.h"
#include "../math/vector.h"
#include "model.h"

#ifndef GRAPHICS_H
#define GRAPHICS_H

#define RENDER_DISTANCE 50

#define MIN(a, b) (a<b ? a : b)
#define MAX(a, b) (a>b ? a : b)

// GLOBALS
//----------------------------------------
extern SDL_Surface *pixel_array;
extern double delta_time;
extern Vector3 lightsource;
extern Camera *graphicsengine_cam;

//----------------------------------------

void triangle_2d_flat(Model *model, Polygon *tri);


void pixel(int x, int y, Uint8 r, Uint8 g, Uint8 b);

void model_draw(Camera *cam, Model *model);
void bounding_box_draw(Camera *cam, Model *model);

void rotate_x(Model *model, float rotation);
void rotate_y(Model *model, float r);
void rotate_z(Model *model, float r);
void rotate_point(Vector3 *pt, float x, float y, float z);
void translate_model(Model *model, float x, float y, float z);
void scale(Model *model, float alpha);
void scale_xyz(Model *model, float x, float y, float z);

void clear_screen(Uint8 r, Uint8 g, Uint8 b);
void render_screen(SDL_Renderer *ren);

void box_3d(Vector3 *pos, float w, float h, float d);

Vector3 calculate_barycentric_3d(int x, int y, Vector3 v1, Vector3 v2, Vector3 v3);

// INTERNAL
//-----------------------------------------
Vector3 calculate_barycentric(int x, int y, Vector2 v1, Vector2 v2, Vector2 v3);
Polygon *clip_against_planes(Camera *cam, int in_size, Polygon *polygons_in, int *out_size);
Vector2 project_coordinate(Vector3 *pt);
int clip_against_plane(Vector3 plane_normal, int poly_count, Polygon *unclipped_triangles, Polygon *clipped_triangles);
int clip_polygon(Vector3 plane_normal, Polygon *tri_in, Polygon *tri_out1, Polygon *tri_out2);
int points_inside_plane(Vector3 plane_normal, Polygon *tri, int *index_of_inside, int *index_of_outside);
Vector3 line_plane_intersect(Vector3 plane_normal, Vector3 p1, Vector3 p2, float *t);
//-----------------------------------------



#endif /* GRAPHICS_H */