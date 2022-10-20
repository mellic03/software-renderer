#ifdef __unix__
  #include <SDL2/SDL.h>
#elif defined(_WIN32) || defined(WIN32)
  #define SDL_MAIN_HANDLED
  #include <SDL.h>
#endif

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "camera.h"
#include "../math/vector.h"
#include "model.h"
#include "datastructures/datastructures.h"

#define RENDER_DISTANCE 50

#define MIN(a, b) (a<b ? a : b)
#define MAX(a, b) (a>b ? a : b)

// GLOBALS
//----------------------------------------
extern SDL_Surface *pixel_array;
extern double delta_time;
extern Vector3 lightsource;
extern Camera *GE_cam;

extern RSR_queue_t *GE_transform_queue;
extern RSR_queue_t *GE_clip_queue;
extern RSR_queue_t *GE_rasterise_queue;
extern Polygon *front_faces;

//----------------------------------------
void GE_init(SDL_Window *win);

void triangle_2d(SDL_Surface *buffer, float *depth_buffer, Polygon *tri);
void triangle_2d_shaded(SDL_Surface *buffer, float *depth_buffer, Polygon *tri);
void SIMD_triangle_2d(SDL_Surface *pxl_buf, float *dpth_buf, Polygon *tri);



void pixel(SDL_Surface *pixel_arr, int x, int y, Uint8 r, Uint8 g, Uint8 b);

void GE_model_enque(Model *model);

void GE_queue_perform_transformation(void);
void GE_queue_perform_clipping(void);
void GE_queue_perform_rasterisation(void);


void rotate_x(Model *model, float r);
void rotate_y(Model *model, float r);
void rotate_z(Model *model, float r);
void rotate_point(Vector3 *pt, float x, float y, float z);
void translate_model(Model *model, float x, float y, float z);
void scale(Model *model, float alpha);
void scale_xyz(Model *model, float x, float y, float z);

void clear_screen(SDL_Surface *pixel_arr, Uint8 r, Uint8 g, Uint8 b);
void draw_screen(SDL_Window *window);



static Vector3 calculate_barycentric(int x, int y, Vector2 v1, Vector2 v2, Vector2 v3);
static Polygon *clip_against_planes(Camera *cam, int in_size, Polygon *polygons_in, int *out_size);
static Vector2 GE_view_to_screen(Vector3 *pt);
static int clip_polygon(Vector3 plane_normal, Polygon *tri_in, Polygon *tri_out1, Polygon *tri_out2);
static int points_inside_plane(Vector3 plane_normal, Polygon *tri, int *index_of_inside, int *index_of_outside);
static Vector3 line_plane_intersect(Vector3 plane_pos, Vector3 plane_normal, Vector3 p1, Vector3 p2, float *t);
void GE_world_to_view(Polygon *polygon);



#endif /* GRAPHICS_H */