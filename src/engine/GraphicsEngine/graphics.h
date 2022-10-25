#ifdef __unix__
  #include <SDL2/SDL.h>
#elif defined(_WIN32) || defined(WIN32)
  #define SDL_MAIN_HANDLED
  #include <SDL.h>
#endif

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "camera.h"
#include "lighting.h"
#include "../math/enginemath.h"
#include "model.h"
#include "datastructures/datastructures.h"

#define RENDER_DISTANCE 50



// GLOBALS
//----------------------------------------
extern SDL_Surface *front_buffer;
extern double delta_time;
extern Camera *GE_cam;

extern RSR_queue_t *GE_transform_queue;
extern RSR_queue_t *GE_clip_queue;
extern RSR_queue_t *GE_rasterise_queue;
extern Polygon *front_faces;

//----------------------------------------

void GE_init(SDL_Window *win);

void triangle_2d(SDL_Surface *pixel_buffer, float *depth_buffer, Polygon *tri, int thread_xmin, int thread_xmax, int thread_ymin, int thread_ymax);
void triangle_2d_shaded(SDL_Surface *buffer, float *depth_buffer, Polygon *tri);
void SIMD_triangle_2d(SDL_Surface *pxl_buf, float *dpth_buf, Polygon *tri, int thread_xmin, int thread_xmax, int thread_ymin, int thread_ymax);


void pixel(SDL_Surface *pixel_arr, int x, int y, Uint8 r, Uint8 g, Uint8 b);

void GE_model_enque(Model *model);

void GE_transform_all(void);
void GE_clip_all(void);
void GE_rasterise_all(void);


void clear_screen(SDL_Surface *pixel_arr, Uint8 r, Uint8 g, Uint8 b);
void draw_screen(SDL_Window *window);

static Vector3 calculate_barycentric(int x, int y, Vector2 v1, Vector2 v2, Vector2 v3, float denom);
static Polygon *clip_against_planes(Camera *cam, int in_size, Polygon *polygons_in, int *out_size);
static Vector2 GE_view_to_screen(Vector3 *pt);
static int clip_polygon(Vector3 plane_normal, Polygon *tri_in, Polygon *tri_out1, Polygon *tri_out2);
static int points_inside_plane(Vector3 plane_normal, Polygon *tri, int *index_of_inside, int *index_of_outside);
static Vector3 line_plane_intersect(Vector3 plane_pos, Vector3 plane_normal, Vector3 p1, Vector3 p2, float *t);
void GE_world_to_view(Polygon *polygon);
Vector3 GE_screen_to_view(Vector2 *pt);
Vector3 GE_view_to_world(Vector3 v0);





#endif /* GRAPHICS_H */