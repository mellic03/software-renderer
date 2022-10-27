#ifdef __unix__
  #include <SDL2/SDL.h>
#elif defined(_WIN32) || defined(WIN32)
  #define SDL_MAIN_HANDLED
  #include <SDL.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include <pthread.h>
#include <x86intrin.h>
#include <cblas.h>

#include "graphics.h"
#include "lighting.h"
#include "shaders.h"
#include "camera.h"
#include "../math/enginemath.h"
#include "../GameEngine/gameengine.h"


SDL_Surface *front_buffer;
SDL_Surface *back_buffer;
float z_buffer[SCREEN_WDTH * SCREEN_HGHT];

SDL_Surface *pix_buf_tl; float *dep_buf_tl;
SDL_Surface *pix_buf_tr; float *dep_buf_tr;
SDL_Surface *pix_buf_bl; float *dep_buf_bl;
SDL_Surface *pix_buf_br; float *dep_buf_br;

double delta_time = 0.001;

Camera *GE_cam;

RSR_queue_t *GE_transform_queue;  // Queue of polygons waiting to be rotated
RSR_queue_t *GE_clip_queue;       // Queue of polygons waiting to be clipped
RSR_queue_t *GE_rasterise_queue;  // Queue of polygons waiting to be rasterised

pthread_t thread_render_1, thread_render_2, thread_render_3, thread_render_4;
RSR_queue_t *GE_rasterise_tl, *GE_rasterise_tr, *GE_rasterise_bl, *GE_rasterise_br; 

void GE_tile_render(  int xmin, int xmax, int ymin, int ymax,
                      SDL_Surface *pixel_buffer,
                      float *depth_buffer,
                      RSR_queue_t *queue_in  )
{
  clear_screen(pixel_buffer, 109, 133, 169);
  int size = queue_in->size;
  for (int i=0; i<size; i++)
  {
    GE_rasterise(pixel_buffer, depth_buffer, RSR_front(queue_in), xmin, xmax, ymin, ymax);
    RSR_dequeue(queue_in);
  }

  SDL_Rect src;
  src.x = xmin;
  src.y = ymin;
  src.w = xmax-xmin;
  src.h = ymax-ymin;

  SDL_Rect dest;
  dest.x = xmin;
  dest.y = ymin;
  dest.w = xmax-xmin;
  dest.h = ymax-ymin;

  SDL_BlitSurface(pixel_buffer, &src, back_buffer, &dest);

  int row;
  for (int y=ymin; y<ymax; y++)
  {
    row = SCREEN_WDTH*y;
    for (int x=xmin; x<xmax; x++)
      depth_buffer[row + x] = 0;
  }
}


void *render_1()
{
  GE_tile_render(0, HALF_SCREEN_WDTH, 0, HALF_SCREEN_HGHT, pix_buf_tl, dep_buf_tl, GE_rasterise_tl);
}
void *render_2()
{
  GE_tile_render(HALF_SCREEN_WDTH, REN_RES_X, 0, HALF_SCREEN_HGHT, pix_buf_tr, dep_buf_tr, GE_rasterise_tr);
}
void *render_3()
{
  GE_tile_render(0, HALF_SCREEN_WDTH, HALF_SCREEN_HGHT, REN_RES_Y, pix_buf_bl, dep_buf_bl, GE_rasterise_bl);
}
void *render_4()
{
  GE_tile_render(HALF_SCREEN_WDTH, REN_RES_X, HALF_SCREEN_HGHT, REN_RES_Y, pix_buf_br, dep_buf_br, GE_rasterise_br);
}


/** Initialise GraphicsEngine
 */
void GE_init(SDL_Window *win)
{
  front_buffer = SDL_GetWindowSurface(win);
  back_buffer = SDL_DuplicateSurface(front_buffer);

  GE_transform_queue = RSR_queue_init();
  GE_clip_queue = RSR_queue_init();
  GE_rasterise_queue = RSR_queue_init();

  GE_rasterise_tl = RSR_queue_init();
  GE_rasterise_tr = RSR_queue_init();
  GE_rasterise_bl = RSR_queue_init();
  GE_rasterise_br = RSR_queue_init();

  pix_buf_tl = SDL_DuplicateSurface(front_buffer);
  pix_buf_tr = SDL_DuplicateSurface(front_buffer);
  pix_buf_bl = SDL_DuplicateSurface(front_buffer);
  pix_buf_br = SDL_DuplicateSurface(front_buffer);

  dep_buf_tl = (float *)calloc(SCREEN_WDTH*SCREEN_HGHT, sizeof(float));
  dep_buf_tr = (float *)calloc(SCREEN_WDTH*SCREEN_HGHT, sizeof(float));
  dep_buf_bl = (float *)calloc(SCREEN_WDTH*SCREEN_HGHT, sizeof(float));
  dep_buf_br = (float *)calloc(SCREEN_WDTH*SCREEN_HGHT, sizeof(float));

  // LightSource *light = GE_lightsource_create(GE_SPOTLIGHT);
  // light->frag_shader = &shade_phong_pointlight;
  // light->pos = (Vector3){0, -3, 0};
  // light->dir = (Vector3){1, 0, 0};
}


// DRAWING
//-------------------------------------------------------------------------------
void clear_screen(SDL_Surface *pixel_buffer, Uint8 r, Uint8 g, Uint8 b)
{
  // for (int i=0; i<SCREEN_WDTH; i++)
  //   for (int j=0; j<SCREEN_HGHT; j++)
  //     pixel(i, j, r, g, b);

  SDL_FillRect(pixel_buffer, NULL, (Uint32)(r<<16) + (Uint16)(g<<8) + b);

  for (int i=0; i<SCREEN_WDTH; i++)
    for (int j=0; j<SCREEN_HGHT; j++)
      z_buffer[SCREEN_WDTH*j + i] = 0;
}

inline void pixel(SDL_Surface *pixel_buffer, int x, int y, Uint8 r, Uint8 g, Uint8 b)
{
  Uint8 *const blue  = ((Uint8 *) pixel_buffer->pixels + (y*4*SCREEN_WDTH) + (x*4 + 0));
  *blue = b;
  Uint8 *const green = ((Uint8 *) pixel_buffer->pixels + (y*4*SCREEN_WDTH) + (x*4 + 1));
  *green = g;
  Uint8 *const red   = ((Uint8 *) pixel_buffer->pixels + (y*4*SCREEN_WDTH) + (x*4 + 2));
  *red = r;
}

bool in_range(float n, float l, float u)
{
  return (n >= l && n <= u) ? true : false;
}

void line_2d(Vector3 stroke, Vector2 p1, Vector2 p2, SDL_Surface *pixel_arr)
{
  float m = (p1.y-p2.y) / (p1.x-p2.x); // slope
  float c = p1.y - m*p1.x; // constant

  float xmin = MIN(p1.x, p2.x);
  float xmax = MAX(p1.x, p2.x);
  float ymin = MIN(p1.y, p2.y);
  float ymax = MAX(p1.y, p2.y);

  // If "vertical"
  if (m < -100 || m > 100)
  {
    for (int y=ymin; y<=ymax; y++)
      pixel(pixel_arr, (int)p1.x, y, stroke.x, stroke.y, stroke.z);
  }

  // if gradient is not between -1 and 1
  else if (!in_range(m, -1, 1))
  {
    for (int y=ymin; y<=ymax; y++)
      pixel(pixel_arr, (int)((y-c)/m), y, stroke.x, stroke.y, stroke.z);
  }

  // if gradient is between -1 and 1
  else
  {
    for (int x=xmin; x<=xmax; x++)
      pixel(pixel_arr, x, (int)(m*x+c), stroke.x, stroke.y, stroke.z);
  }
}

/** Return true if a polygon defined by three vectors has a clockwise winding order
 */
bool GE_poly_clockwise(Vector2 a, Vector2 b, Vector2 c)
{
  return vector2_cross(vector2_sub(b, a), vector2_sub(c, a)) >= 0;
}

Vector3 calculate_barycentric(int x, int y, Vector2 v1, Vector2 v2, Vector2 v3, float denom)
{
  Vector3 weights;
  weights.x = ((v2.y-v3.y)*(x-v3.x) + (v3.x-v2.x)*(y-v3.y)) / denom;
  weights.y = ((v3.y-v1.y)*(x-v3.x) + (v1.x-v3.x)*(y-v3.y)) / denom;
  weights.z = 1 - weights.x - weights.y;
  return weights;
}

float f_xy(int x, int y, Vector2 v2, Vector2 v3, float denom)
{
  return ((v2.y-v3.y)*(x-v3.x) + (v3.x-v2.x)*(y-v3.y)) / denom;
}

float g_xy(int x, int y, Vector2 v1, Vector2 v3, float denom)
{
  return ((v3.y-v1.y)*(x-v3.x) + (v1.x-v3.x)*(y-v3.y)) / denom;
}

float q_xy(float f_xy, float g_xy)
{
  return 1 - f_xy - g_xy;
}

/** Rasterise a Polygon
 * \param pxl_buf pixel buffer to draw to
 * \param dep_buf depth buffer to compare depth values to
 * \param tri polygon to rasterise
 * \param xmin lower x bound
 * \param xmax upper x bound
 * \param ymin lower y bound
 * \param ymax upper y bound
 * x/y bounds exist for multithreading, if rasterising singlethreaded,
 * they should be set to the screen dimensions.
 */
void GE_rasterise(SDL_Surface *pxl_buf, float *dep_buf, Polygon *tri, int xmin, int xmax, int ymin, int ymax)
{
  Vector2 v1 = tri->proj_verts[0];
  Vector2 v2 = tri->proj_verts[1];
  Vector2 v3 = tri->proj_verts[2];

  float denom = (v2.y-v3.y)*(v1.x-v3.x) + (v3.x-v2.x)*(v1.y-v3.y);

  float inv_u[3] = {tri->uvs[0].x * v1.w, tri->uvs[1].x * v2.w, tri->uvs[2].x * v3.w};
  float inv_v[3] = {tri->uvs[0].y * v1.w, tri->uvs[1].y * v2.w, tri->uvs[2].y * v3.w};

  int lx = MIN(v1.x, MIN(v2.x, v3.x));
  int hx = MAX(v1.x, MAX(v2.x, v3.x));
  int ly = MIN(v1.y, MIN(v2.y, v3.y));
  int hy = MAX(v1.y, MAX(v2.y, v3.y));

  lx = MAX(xmin, lx); lx = MIN(lx, xmax-1);
  hx = MAX(xmin, hx); hx = MIN(hx, xmax-1);
  ly = MAX(ymin, ly); ly = MIN(ly, ymax-1);
  hy = MAX(ymin, hy); hy = MIN(hy, ymax-1);

  Uint16 u=0, v=0;
  float z_index; 

  Uint8 *red, *green, *blue;
  Uint8 *pixels = tri->texture->pixels;

  Vector3 lighting = {1, 1, 1};
  Vector3 deprojected;

  const float A1 = f_xy(lx+1, ly, v2, v3, denom); float A2 = f_xy(lx, ly, v2, v3, denom);
  const float B1 = f_xy(lx, ly+1, v2, v3, denom);
  
  const float D1 = g_xy(lx+1, ly, v1, v3, denom); float D2 = g_xy(lx, ly, v1, v3, denom);
  const float E1 = g_xy(lx, ly+1, v1, v3, denom);

  const float f_xstep = A1-A2,  g_xstep = D1-D2,  q_xstep = q_xy(A1, D1) - q_xy(A2, D2);
  const float f_ystep = B1-A2,  g_ystep = E1-D2,  q_ystep = q_xy(B1, E1) - q_xy(A2, D2);
  
  float og_q = q_xy(A2, D2);

  float f, g, q;
  
  int count;

  for (int y=ly; y<=hy; y++)
  {
    count = LIGHT_PIXEL_STEP;
    f = A2, g = D2, q = og_q;

    for (int x=lx; x<=hx; x++)
    {
      if (f >= 0 && g >= 0 && q >= 0)
      {
        z_index = f*v1.w + g*v2.w + q*v3.w;

        if (z_index > dep_buf[SCREEN_WDTH*y + x])
        {
          dep_buf[SCREEN_WDTH*y + x] = z_index;

          u = (Uint16)((f*inv_u[0] + g*inv_u[1] + q*inv_u[2]) / z_index) % tri->texture->w;
          v = (Uint16)((f*inv_v[0] + g*inv_v[1] + q*inv_v[2]) / z_index) % tri->texture->h;

          u *= tri->texture->format->BytesPerPixel;

          red   = pixels + v*tri->texture->pitch + u+2;
          green = pixels + v*tri->texture->pitch + u+1;
          blue  = pixels + v*tri->texture->pitch + u+0;

          if (count >= LIGHT_PIXEL_STEP)
          {
            count = 0;
            deprojected = GE_screen_to_view((Vector2){x, y, z_index});
            lighting = GE_lightsource_perform_fragment(tri, deprojected);
          }

          float r = (float)*red * lighting.x;
          float g = (float)*green * lighting.y;
          float b = (float)*blue * lighting.z;

          pixel(pxl_buf, x, y, (Uint8)r, (Uint8)g, (Uint8)b);
        }
      }

      f += f_xstep;
      g += g_xstep;
      q += q_xstep;
      count++;
    }

    A2   += f_ystep;
    D2   += g_ystep;
    og_q += q_ystep;
  }
}


/*
  // calculate normal, tangent and bitangent
  Vector3 delta_pos1 = vector3_sub(tri->vertices[0], tri->vertices[0]);
  Vector3 delta_pos2 = vector3_sub(tri->vertices[2], tri->vertices[0]);

  Vector2 delta_uv1 = vector2_sub(tri->uvs[2], tri->uvs[0]);
  Vector2 delta_uv2 = vector2_sub(tri->uvs[2], tri->uvs[0]);
  
  float tr = 1 / ((delta_uv1.x * delta_uv2.y) - (delta_uv1.y * delta_uv2.x));
  Vector3 tangent = (vector3_sub(vector3_scale(delta_pos1, delta_uv2.y), vector3_scale(delta_pos2, delta_uv1.y)));
  Vector3 bitangent = (vector3_add(vector3_scale(delta_pos1, -delta_uv2.y), vector3_scale(delta_pos2, delta_uv1.y)));
  
  Vector3 mapped_normal = {
    *(nmap + v*tri->texture->pitch + u+2),
    *(nmap + v*tri->texture->pitch + u+1),
    *(nmap + v*tri->texture->pitch + u+0)
  };
*/


/**
 * @return __m128 where each element is the weighting of each (x, y) with respect to the first input vertex
 */
__m128 SIMD_calculate_barycentric_first(__m128 *_x, __m128 *_y, __m128 *_v3x, __m128 *_v3y, __m128 *_v2y_take_v3y, __m128 *_v3x_take_v2x, __m128 *_denom)
{
  // first = ((v2.y-v3.y)*(x-v3.x) + (v3.x-v2.x)*(y-v3.y)) / denom
  __m128 _first = _mm_sub_ps(*_x, *_v3x);
  _first = _mm_mul_ps(*_v2y_take_v3y, _first);
  _first = _mm_add_ps(_first, _mm_mul_ps(*_v3x_take_v2x, _mm_sub_ps(*_y, *_v3y)));
  _first = _mm_div_ps(_first, *_denom);
  return _first;
}

/**
 * @return __m128 where each element is the weighting of each (x, y) with respect to the second input vertex
 */
__m128 SIMD_calculate_barycentric_second(__m128 *_x, __m128 *_y, __m128 *_v3x, __m128 *_v3y, __m128 *_v3y_take_v1y, __m128 *_v1x_take_v3x, __m128 *_denom)
{
  // second = ((v3.y-v1.y)*(x-v3.x) + (v1.x-v3.x)*(y-v3.y)) / denom
  __m128 _second = _mm_sub_ps(*_x, *_v3x);
  _second = _mm_mul_ps(*_v3y_take_v1y, _second);
  _second = _mm_add_ps(_second, _mm_mul_ps(*_v1x_take_v3x, _mm_sub_ps(*_y, *_v3y)));
  _second = _mm_div_ps(_second, *_denom);
  return _second;
}

/**
 * @return __m128 where each element is the weighting of each (x, y) with respect to the third input vertex
 */
void SIMD_calculate_barycentric_third(__m128 *identity, __m128 *first, __m128 *second, __m128 *output)
{
  *output = _mm_sub_ps(*identity, *first);
  *output = _mm_sub_ps(*output, *second);
}

int SIMD_cmp_dpth_buf(__m128 *_z_index, __m128 *_dpth_buf)
{
  return _mm_movemask_ps(_mm_cmp_ps(*_z_index, *_dpth_buf, 0x0E));
}

void GE_SIMD_rasterise(SDL_Surface *pxl_buf, float *dpth_buf, Polygon *tri, int thread_xmin, int thread_xmax, int thread_ymin, int thread_ymax)
{
  Vector2 v1 = GE_view_to_screen(&tri->vertices[0]);
  Vector2 v2 = GE_view_to_screen(&tri->vertices[1]);
  Vector2 v3 = GE_view_to_screen(&tri->vertices[2]);

  // Vector3 light2 = lightsource;
  // vector3_normalise(&light2);
  // float shade = vector3_dot(tri->face_normal, light2);
  // shade += 1;
  // shade /= 2;

  // Barycentric
  //--------------------------------------------------
  __m128 _v1_w = _mm_set1_ps(v1.w);
  __m128 _v2_w = _mm_set1_ps(v2.w);
  __m128 _v3_w = _mm_set1_ps(v3.w);
  
  __m128 _v3x = _mm_set1_ps(v3.x);
  __m128 _v3y = _mm_set1_ps(v3.y);

  __m128 _v2y_take_v3y = _mm_set1_ps(v2.y-v3.y);
  __m128 _v3x_take_v2x = _mm_set1_ps(v3.x-v2.x);

  __m128 _v3y_take_v1y = _mm_set1_ps(v3.y-v1.y);
  __m128 _v1x_take_v3x = _mm_set1_ps(v1.x-v3.x);

  __m128 _denom = _mm_set1_ps((v2.y-v3.y)*(v1.x-v3.x) + (v3.x-v2.x)*(v1.y-v3.y));
  
  __m128 _x, _y;
  __m128 _ones = _mm_set1_ps(1);
  __m128 _zeroes = _mm_set1_ps(0.0f);
  __m128 _first, _second, _third;
  //--------------------------------------------------


  // Texturing
  //--------------------------------------------------
  __m128 _inv_u0 = _mm_set1_ps(tri->uvs[0].x * v1.w);
  __m128 _inv_u1 = _mm_set1_ps(tri->uvs[1].x * v2.w);
  __m128 _inv_u2 = _mm_set1_ps(tri->uvs[2].x * v3.w);

  __m128 _inv_v0 = _mm_set1_ps(tri->uvs[0].y * v1.w);
  __m128 _inv_v1 = _mm_set1_ps(tri->uvs[1].y * v2.w);
  __m128 _inv_v2 = _mm_set1_ps(tri->uvs[2].y * v3.w);
  Uint8 *red, *green, *blue;
  //--------------------------------------------------

  Uint8 masks[4] = { 0b00000001, 0b00000010, 0b00000100, 0b00001000 };

  int lx = MIN(v1.x, MIN(v2.x, v3.x));
  int hx = MAX(v1.x, MAX(v2.x, v3.x));
  int ly = MIN(v1.y, MIN(v2.y, v3.y));
  int hy = MAX(v1.y, MAX(v2.y, v3.y));

  lx = MAX(thread_xmin, lx), lx = MIN(lx, thread_xmax);
  hx = MAX(thread_xmin, hx), hx = MIN(hx, thread_xmax);
  ly = MAX(thread_ymin, ly), ly = MIN(ly, thread_ymax);
  hy = MAX(thread_ymin, hy), hy = MIN(hy, thread_ymax);


  // line_2d((Vector3){0, 255, 0}, (Vector2){v1.x, v1.y, 1}, (Vector2){v2.x, v2.y, 1}, pxl_buf);
  // line_2d((Vector3){0, 255, 0}, (Vector2){v2.x, v2.y, 1}, (Vector2){v3.x, v3.y, 1}, pxl_buf);
  // line_2d((Vector3){0, 255, 0}, (Vector2){v3.x, v3.y, 1}, (Vector2){v1.x, v1.y, 1}, pxl_buf);

  // line_2d((Vector3){0, 0, 0}, (Vector2){lx, ly, 1}, (Vector2){hx, ly, 1}, pxl_buf);
  // line_2d((Vector3){0, 0, 0}, (Vector2){hx, ly, 1}, (Vector2){hx, hy, 1}, pxl_buf);
  // line_2d((Vector3){0, 0, 0}, (Vector2){hx, hy, 1}, (Vector2){lx, hy, 1}, pxl_buf);
  // line_2d((Vector3){0, 0, 0}, (Vector2){lx, hy, 1}, (Vector2){lx, ly, 1}, pxl_buf);

  Uint16 u=0, v=0;

  Uint8 remainder = (hx-lx) % 4;

  for (int x=lx; x<=hx-remainder; x+=4)
  {
    _x = _mm_set_ps(x+3, x+2, x+1, x+0);

    for (int y=ly; y<=hy; y++)
    {
      int row = SCREEN_WDTH*y;
      _y = _mm_set1_ps(y);
      
      // Barycentric coordinates
      _first = SIMD_calculate_barycentric_first(&_x, &_y, &_v3x, &_v3y, &_v2y_take_v3y, &_v3x_take_v2x, &_denom);
      _second = SIMD_calculate_barycentric_second(&_x, &_y, &_v3x, &_v3y, &_v3y_take_v1y, &_v1x_take_v3x, &_denom);
      SIMD_calculate_barycentric_third(&_ones, &_first, &_second, &_third);
      __m128 _fir_gtrthan_zero = _mm_cmp_ps(_first,  _zeroes, 0x0D);
      __m128 _sec_gtrthan_zero = _mm_cmp_ps(_second, _zeroes, 0x0D);
      __m128 _thi_gtrthan_zero = _mm_cmp_ps(_third,  _zeroes, 0x0D);
      int in_triangle = _mm_movemask_ps(_fir_gtrthan_zero) & _mm_movemask_ps(_sec_gtrthan_zero) & _mm_movemask_ps(_thi_gtrthan_zero);

      // z index and z buffer
      __m128 _z_indices = _mm_add_ps(_mm_mul_ps(_v1_w, _first),  _mm_add_ps(_mm_mul_ps(_v2_w, _second), _mm_mul_ps(_v3_w, _third)));
      __m128 _z_buf = _mm_set_ps(dpth_buf[row + (x+3)], dpth_buf[row + (x+2)], dpth_buf[row + (x+1)], dpth_buf[row + (x+0)]);
      int gtr_thn_zbuf = SIMD_cmp_dpth_buf(&_z_indices, &_z_buf);


      // Texture coordinates
      __m128 _texture_u = _mm_add_ps(_mm_mul_ps(_first, _inv_u0), _mm_add_ps(_mm_mul_ps(_second, _inv_u1), _mm_mul_ps(_third, _inv_u2)));
      __m128 _texture_v = _mm_add_ps(_mm_mul_ps(_first, _inv_v0), _mm_add_ps(_mm_mul_ps(_second, _inv_v1), _mm_mul_ps(_third, _inv_v2)));
      _texture_u = _mm_div_ps(_texture_u, _z_indices);
      _texture_v = _mm_div_ps(_texture_v, _z_indices);

      Uint8 in_tri_and_gtr_thn_zbuf = in_triangle & gtr_thn_zbuf;

      for (Uint8 i=0; i<4; i++)
      {
        if (masks[i] & in_tri_and_gtr_thn_zbuf)
        {
          dpth_buf[row + x+i] = _z_indices[i];

          u = (Uint16)_texture_u[i] % tri->texture->w;
          v = (Uint16)_texture_v[i] % tri->texture->h;

          u *= tri->texture->format->BytesPerPixel;

          red   = tri->texture->pixels + v*tri->texture->pitch + u+2;
          green = tri->texture->pixels + v*tri->texture->pitch + u+1;
          blue  = tri->texture->pixels + v*tri->texture->pitch + u+0;

          pixel(pxl_buf, x+i, y, *red, *green, *blue);
        }
      }
    }
  }

  float inverse_u_coords[3] = {tri->uvs[0].x * v1.w, tri->uvs[1].x * v2.w, tri->uvs[2].x * v3.w};
  float inverse_v_coords[3] = {tri->uvs[0].y * v1.w, tri->uvs[1].y * v2.w, tri->uvs[2].y * v3.w};
  float denom = (v2.y-v3.y)*(v1.x-v3.x) + (v3.x-v2.x)*(v1.y-v3.y);

  Vector3 vert_weights;
  float z_index;

  for (int x=hx-remainder; x<=hx; x++)
  {
    for (int y=ly; y<=hy; y++)
    {
     vert_weights = calculate_barycentric(x, y, v1, v2, v3, denom);

      if (vert_weights.x >= 0 && vert_weights.y >= 0 && vert_weights.z >= 0)
      {
        z_index = v1.w*vert_weights.x + v2.w*vert_weights.y + v3.w*vert_weights.z;

        if (z_index > dpth_buf[SCREEN_WDTH*y + x])
        {
          dpth_buf[SCREEN_WDTH*y + x] = z_index;

          u = (Uint16)((vert_weights.x*inverse_u_coords[0] + vert_weights.y*inverse_u_coords[1] + vert_weights.z*inverse_u_coords[2]) / z_index) % tri->texture->w;
          v = (Uint16)((vert_weights.x*inverse_v_coords[0] + vert_weights.y*inverse_v_coords[1] + vert_weights.z*inverse_v_coords[2]) / z_index) % tri->texture->h;

          u *= tri->texture->format->BytesPerPixel;

          red   = tri->texture->pixels + v*tri->texture->pitch + u+2;
          green = tri->texture->pixels + v*tri->texture->pitch + u+1;
          blue  = tri->texture->pixels + v*tri->texture->pitch + u+0;

          pixel(pxl_buf, x, y, *red, *green, *blue);
        }
      }
    }
  }
}

/**
 * @param p1 start of line
 * @param p2 end of line
 */
Vector3 line_plane_intersect(Vector3 plane_pos, Vector3 plane_normal, Vector3 p1, Vector3 p2, float *t)
{
  vector3_normalise(&plane_normal);
  float plane_d = -vector3_dot(plane_normal, plane_pos);
  float ad = vector3_dot(p1, plane_normal);
  float bd = vector3_dot(p2, plane_normal);
  *t = (-plane_d - ad) / (bd - ad);
  Vector3 lste = vector3_sub(p2, p1);
  Vector3 lti = vector3_scale(lste, *t);
  return vector3_add(p1, lti);
}

float GE_point_plane_dist(Vector3 *point, Vector3 *plane_pos, Vector3 *plane_normal)
{
  return vector3_dot(*plane_normal, vector3_sub(*point, *plane_pos));
}

int GE_clip_against_plane(Vector3 *plane_pos, Vector3 *plane_normal, Polygon *tri_in, Polygon *tri_out1, Polygon *tri_out2)
{
  vector3_normalise(plane_normal);

  int inside_index = 0, outside_index = 0;
  int inside_count = 0, outside_count = 0;
  Vector3 *inside_verts[3];
  Vector3 *outside_verts[3];

  Vector2 *inside_uvs[3];
  Vector2 *outside_uvs[3];

  Vector3 *inside_normals[3], *outside_normals[3];

  float t;

  float d0 = GE_point_plane_dist(&tri_in->vertices[0], plane_pos, plane_normal);
  float d1 = GE_point_plane_dist(&tri_in->vertices[1], plane_pos, plane_normal);
  float d2 = GE_point_plane_dist(&tri_in->vertices[2], plane_pos, plane_normal);

  float dots[3] = { d0, d1, d2 };

  for (int i=0; i<3; i++)
  {
    if (dots[i] >= 0)
    {
      inside_verts[inside_count] = &tri_in->vertices[i];
      inside_uvs[inside_count] = &tri_in->uvs[i];
      // inside_normals[inside_count] = &tri_in->normals[i];
      inside_index = i; inside_count += 1;
    }

    else
    {
      outside_verts[outside_count] = &tri_in->vertices[i];
      outside_uvs[outside_count] = &tri_in->uvs[i];
      // outside_normals[outside_count] = &tri_in->normals[i];
      outside_index = i; outside_count += 1;
    }
  }

  int insd = inside_index;
  int outsd1 = (inside_index+1)%3;
  int outsd2 = (inside_index+2)%3;

  int outsd = outside_index;
  int insd1 = (outside_index+1)%3;
  int insd2 = (outside_index+2)%3;

  switch (inside_count)
  {
    case (0): return 0;

    case (3):
      *tri_out1 = *tri_in;
      return 1;

    case (1): // 1 point inside, 2 outside
      *tri_out1 = *tri_in;

      tri_out1->vertices[insd] = *inside_verts[0];
      tri_out1->vertices[outsd1] = line_plane_intersect(*plane_pos, *plane_normal, *inside_verts[0], *outside_verts[0], &t);
      tri_out1->uvs[outsd1].x = inside_uvs[0]->x + t*(outside_uvs[0]->x - inside_uvs[0]->x);
      tri_out1->uvs[outsd1].y = inside_uvs[0]->y + t*(outside_uvs[0]->y - inside_uvs[0]->y);
    
      tri_out1->vertices[outsd2] = line_plane_intersect(*plane_pos, *plane_normal, *inside_verts[0], *outside_verts[1], &t);
      tri_out1->uvs[outsd2].x = inside_uvs[0]->x + t*(outside_uvs[1]->x - inside_uvs[0]->x);
      tri_out1->uvs[outsd2].y = inside_uvs[0]->y + t*(outside_uvs[1]->y - inside_uvs[0]->y);
      return 1;

    case (2): // 2 points inside, 1 outside

      *tri_out1 = *tri_in;
      tri_out1->vertices[outsd] = *inside_verts[0];
      tri_out1->vertices[insd1] = *inside_verts[1];
      tri_out1->vertices[insd2] = line_plane_intersect(*plane_pos, *plane_normal, *inside_verts[0], *outside_verts[0], &t);

      tri_out1->uvs[outsd] = *inside_uvs[0];
      tri_out1->uvs[insd1] = *inside_uvs[1];
      tri_out1->uvs[insd2].x = inside_uvs[0]->x + (t)*(outside_uvs[0]->x - inside_uvs[0]->x);
      tri_out1->uvs[insd2].y = inside_uvs[0]->y + (t)*(outside_uvs[0]->y - inside_uvs[0]->y);


      *tri_out2 = *tri_in;
      tri_out2->vertices[outsd] = *inside_verts[1];
      tri_out2->vertices[insd1] = tri_out1->vertices[insd2];
      tri_out2->vertices[insd2] = line_plane_intersect(*plane_pos, *plane_normal, *inside_verts[1], *outside_verts[0], &t);

      tri_out2->uvs[outsd] = *inside_uvs[1];
      tri_out2->uvs[insd1] = tri_out1->uvs[insd2];
      tri_out2->uvs[insd2].x = inside_uvs[1]->x + (t)*(outside_uvs[0]->x - inside_uvs[1]->x);
      tri_out2->uvs[insd2].y = inside_uvs[1]->y + (t)*(outside_uvs[0]->y - inside_uvs[1]->y);
      return 2;
  }
}

void GE_clip_queue_3D(Vector3 plane_pos, Vector3 plane_normal, RSR_queue_t *in_queue, RSR_queue_t *out_queue)
{
  Polygon tri_in, tri_out1, tri_out2;

  int size = in_queue->size;
  for (int i=0; i<size; i++)
  {
    tri_in = *RSR_front(in_queue);
    RSR_dequeue(in_queue);

    int n = GE_clip_against_plane(&plane_pos, &plane_normal, &tri_in, &tri_out1, &tri_out2);

    switch (n)
    {
      case (0): break;

      case (1):
        RSR_enque(out_queue, &tri_out1);
        break;

      case (2):
        RSR_enque(out_queue, &tri_out1);
        RSR_enque(out_queue, &tri_out2);
        break;
    }
  }
}


/** Enque a model's polygons in the GE_transform_queue
 */
void GE_model_enque(Model *model)
{
  // Only queue front faces
  for (int i=0; i<model->poly_count; i++)
    if (vector3_dot(vector3_sub(model->polygons[i].vertices[0], *GE_cam->pos), model->polygons[i].face_normal) < 0)
      RSR_enque(GE_transform_queue, &model->polygons[i]);
}
//-------------------------------------------------------------------------------

/** Translate all polygons in GE_transform_queue by -GE_cam.pos, rotate them by -GE_cam.rot and move them to GE_clip_queue.
 *  GE_transform_queue will be emptied.
 */
void GE_transform_all(void)
{
  GE_lightsource_world_to_view_all();

  int size = GE_transform_queue->size;

  for (int i=0; i<size; i++)
  {
    Polygon tri = *RSR_front(GE_transform_queue);
    RSR_dequeue(GE_transform_queue);
    GE_world_to_view(&tri);

    if (tri.vertices[0].z > 0 || tri.vertices[1].z > 0 || tri.vertices[2].z > 0)
      RSR_enque(GE_clip_queue, &tri);
  }
}

void GE_clip_all(void)
{
  GE_clip_queue_3D((Vector3){0, 0, 1}, GE_cam->near_plane, GE_clip_queue, GE_rasterise_queue);
  // GE_clip_queue_3D((Vector3){0, 0, 1}, GE_cam->near_plane, GE_clip_queue, GE_clip_queue);
  // GE_clip_queue_3D((Vector3){0, 0, 0}, GE_cam->left_plane, GE_clip_queue, GE_clip_queue);
  // GE_clip_queue_3D((Vector3){0, 0, 0}, GE_cam->right_plane, GE_clip_queue, GE_clip_queue);
  // GE_clip_queue_3D((Vector3){0, 0, 0}, GE_cam->top_plane, GE_clip_queue, GE_clip_queue);
  // GE_clip_queue_3D((Vector3){0, 0, 0}, GE_cam->bottom_plane, GE_clip_queue, GE_rasterise_queue);
}

/** Rasterise all polygons in GE_rasterise_queue
 *  GE_rasterise_queue will be emptied.
 */
void GE_rasterise_all(void)
{
  clear_screen(back_buffer, 255, 0, 0);

  int size = GE_rasterise_queue->size;

  for (int i=0; i<size; i++)
  {
    Polygon tri = *RSR_front(GE_rasterise_queue);
    RSR_dequeue(GE_rasterise_queue);
   
    for (int j=0; j<3; j++)
      tri.proj_verts[j] = GE_view_to_screen(&tri.vertices[j]);

    GE_rasterise(back_buffer, z_buffer, &tri, 0, REN_RES_X, 0, REN_RES_Y);

    // If left
    // if (tri.proj_verts[0].x < HALF_SCREEN_WDTH && tri.proj_verts[1].x < HALF_SCREEN_WDTH && tri.proj_verts[2].x < HALF_SCREEN_WDTH)
    // {
    //   // If top
    //   if (tri.proj_verts[0].y < HALF_SCREEN_HGHT && tri.proj_verts[1].y < HALF_SCREEN_HGHT && tri.proj_verts[2].y < HALF_SCREEN_HGHT)
    //     RSR_enque(GE_rasterise_tl, &tri);

    //   // If bottom
    //   else if (tri.proj_verts[0].y >= HALF_SCREEN_HGHT && tri.proj_verts[1].y >= HALF_SCREEN_HGHT && tri.proj_verts[2].y >= HALF_SCREEN_HGHT)
    //     RSR_enque(GE_rasterise_bl, &tri);
    
    //   // If top and bottom
    //   else
    //   {
    //     RSR_enque(GE_rasterise_tl, &tri);
    //     RSR_enque(GE_rasterise_bl, &tri);
    //   }
    // }
    
    // // If right
    // else if (tri.proj_verts[0].x >= HALF_SCREEN_WDTH && tri.proj_verts[1].x >= HALF_SCREEN_WDTH && tri.proj_verts[2].x >= HALF_SCREEN_WDTH)
    // {
    //   // If top
    //   if (tri.proj_verts[0].y < HALF_SCREEN_HGHT && tri.proj_verts[1].y < HALF_SCREEN_HGHT && tri.proj_verts[2].y < HALF_SCREEN_HGHT)
    //     RSR_enque(GE_rasterise_tr, &tri);
      
    //   // If bottom
    //   else if (tri.proj_verts[0].y >= HALF_SCREEN_HGHT && tri.proj_verts[1].y >= HALF_SCREEN_HGHT && tri.proj_verts[2].y >= HALF_SCREEN_HGHT)
    //     RSR_enque(GE_rasterise_br, &tri);

    //   // If top and bottom
    //   else
    //   {
    //     RSR_enque(GE_rasterise_tr, &tri);
    //     RSR_enque(GE_rasterise_br, &tri);
    //   }
    // }    

    // // If left and right
    // else
    // {
    //   // If top
    //   if (tri.proj_verts[0].y < HALF_SCREEN_HGHT && tri.proj_verts[1].y < HALF_SCREEN_HGHT && tri.proj_verts[2].y < HALF_SCREEN_HGHT)
    //   {
    //     RSR_enque(GE_rasterise_tl, &tri);
    //     RSR_enque(GE_rasterise_tr, &tri);
    //   }

    //   // If bottom
    //   else if (tri.proj_verts[0].y >= HALF_SCREEN_HGHT && tri.proj_verts[1].y >= HALF_SCREEN_HGHT && tri.proj_verts[2].y >= HALF_SCREEN_HGHT)
    //   {
    //     RSR_enque(GE_rasterise_bl, &tri);
    //     RSR_enque(GE_rasterise_br, &tri);
    //   }

    //   // If all four quadrants
    //   else
    //   {
    //     RSR_enque(GE_rasterise_tl, &tri);
    //     RSR_enque(GE_rasterise_tr, &tri);
    //     RSR_enque(GE_rasterise_bl, &tri);
    //     RSR_enque(GE_rasterise_br, &tri);
    //   }
    // }
  }


  // pthread_create(&thread_render_1, NULL, render_1, NULL);
  // pthread_create(&thread_render_2, NULL, render_2, NULL);
  // pthread_create(&thread_render_3, NULL, render_3, NULL);
  // pthread_create(&thread_render_4, NULL, render_4, NULL);

  // pthread_join(thread_render_1, NULL);
  // pthread_join(thread_render_2, NULL);
  // pthread_join(thread_render_3, NULL);
  // pthread_join(thread_render_4, NULL);

  SDL_Rect src;
  src.x = 0;
  src.y = 0;
  src.w = REN_RES_X;
  src.h = REN_RES_Y;

  SDL_Rect dest;
  dest.x = 0;
  dest.y = 0;
  dest.w = SCREEN_WDTH;
  dest.h = SCREEN_HGHT;

  SDL_BlitScaled(back_buffer, &src, front_buffer, &dest);
}


/** Convert a polygon's vertices from world-space to view-space
 */
void GE_world_to_view(Polygon *polygon)
{
  for (int i=0; i<3; i++)
  {
    polygon->vertices[i] = vector3_sub(polygon->vertices[i], *GE_cam->pos);

    vector3_roty(&polygon->vertices[i], GE_cam->rot.y);
    vector3_rotx(&polygon->vertices[i], GE_cam->rot.x);

    vector3_roty(&polygon->normals[i], GE_cam->rot.y);
    vector3_rotx(&polygon->normals[i], GE_cam->rot.x);

    // polygon->og_vertices[i] = vector3_sub(polygon->og_vertices[i], *GE_cam->pos);
    // vector3_rotate(&polygon->og_vertices[i], GE_cam->rot.x, GE_cam->rot.y, 0);
  }
  vector3_roty(&polygon->face_normal, GE_cam->rot.y);
  vector3_rotx(&polygon->face_normal, GE_cam->rot.x);
}

/** Convert a view-space Vector3 to a screen-space Vector2
 * 1/z is preserved for depth buffer.
 */
Vector2 GE_view_to_screen(Vector3 *pt)
{
  float nearplane_z = 0.5;
  
  float canvas_x = (pt->x * nearplane_z * REN_RES_X) / (pt->z*VIEWPLANE_WDTH);
  float canvas_y = (pt->y * nearplane_z * REN_RES_Y) / (pt->z*VIEWPLANE_HGHT);
 
  canvas_x += HALF_SCREEN_WDTH;
  canvas_y += HALF_SCREEN_HGHT;

  // float canvas_x = (pt->x * HALF_SCREEN_WDTH) / pt->z;
  // float canvas_y = (pt->y * HALF_SCREEN_HGHT) / pt->z;
  // canvas_x += HALF_SCREEN_WDTH - 0.5;
  // canvas_y += HALF_SCREEN_HGHT - 0.5;

  return (Vector2){canvas_x, canvas_y, 1/pt->z};
}

/** Convert a screen-space Vector2 to a view-space Vector3
 */
Vector3 GE_screen_to_view(Vector2 pt)
{
  float z = 1/pt.w;
  Vector3 deprojected = {
    ((z*VIEWPLANE_WDTH) * (pt.x - HALF_SCREEN_WDTH)) / (0.5 * REN_RES_X),
    ((z*VIEWPLANE_HGHT) * (pt.y - HALF_SCREEN_HGHT)) / (0.5 * REN_RES_Y),
    z
  };
  return deprojected;
}

Vector3 GE_view_to_world(Vector3 v0)
{
  vector3_roty(&v0, GE_cam->rot.y);
  vector3_rotx(&v0, GE_cam->rot.x);
  v0 = vector3_add(v0, *GE_cam->pos);
  return v0;
}