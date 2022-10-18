#ifdef __unix__
  #include <SDL2/SDL.h>
#elif defined(_WIN32) || defined(WIN32)
  #define SDL_MAIN_HANDLED
  #include <SDL.h>
#endif

#include <math.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <x86intrin.h>

#include "graphics.h"
#include "camera.h"
#include "../math/vector.h"


// pixel buffers are blitted onto pixel_array after thread finishes
SDL_Surface *pixel_buffer_left; 
SDL_Surface *pixel_buffer_right; 
SDL_Surface *pixel_array;

float z_buffer[SCREEN_WDTH * SCREEN_HGHT];

Vector3 lightsource = {0, -1, 0};

double delta_time = 0.001;
Camera *GE_cam;

RSR_queue_t *GE_transform_queue;  // Queue of polygons waiting to be rotated
RSR_queue_t *GE_clip_queue;       // Queue of polygons waiting to be clipped
RSR_queue_t *GE_rasterise_queue;  // Queue of polygons waiting to be rasterised

RSR_queue_t *GE_rasterise_left, *GE_rasterise_right;
pthread_t thread_render_1, thread_render_2;

pthread_cond_t rasterise_ready;
pthread_cond_t thread_done;
pthread_mutex_t rasterise_left, rasterise_right, rasterise_done;

void *render_1()
{
  clear_screen(pixel_buffer_left, 109, 133, 169);

  int size = GE_rasterise_left->size;
  for (int i=0; i<size; i++)
  {
    triangle_2d(pixel_buffer_left, RSR_front(GE_rasterise_left));
    RSR_dequeue(GE_rasterise_left);
  }
  
  SDL_Rect src;
  src.x = 0;
  src.y = 0;
  src.w = HALF_SCREEN_WDTH;
  src.h = SCREEN_HGHT;

  SDL_Rect dest;
  dest.x = 0;
  dest.y = 0;
  dest.w = HALF_SCREEN_WDTH;
  dest.h = SCREEN_HGHT;

  SDL_BlitSurface(pixel_buffer_left, &src, pixel_array, &dest);
}

void *render_2()
{
  clear_screen(pixel_buffer_right, 109, 133, 169);

  int size = GE_rasterise_right->size;
  for (int i=0; i<size; i++)
  {
    triangle_2d(pixel_buffer_right, RSR_front(GE_rasterise_right));
    RSR_dequeue(GE_rasterise_right);
  }
  
  SDL_Rect src;
  src.x = HALF_SCREEN_WDTH;
  src.y = 0;
  src.w = SCREEN_WDTH;
  src.h = SCREEN_HGHT;

  SDL_Rect dest;
  dest.x = HALF_SCREEN_WDTH;
  dest.y = 0;
  dest.w = SCREEN_WDTH;
  dest.h = SCREEN_HGHT;

  SDL_BlitSurface(pixel_buffer_right, &src, pixel_array, &dest);
}

/** Initialise GraphicsEngine
 */
void GE_init(SDL_Window *win)
{
  GE_transform_queue = RSR_queue_init();
  GE_clip_queue = RSR_queue_init();
  GE_rasterise_queue = RSR_queue_init();
  
  GE_rasterise_left = RSR_queue_init();
  GE_rasterise_right = RSR_queue_init();


  pixel_array = SDL_GetWindowSurface(win);

  pthread_cond_init(&rasterise_ready, NULL);
  pthread_cond_init(&thread_done, NULL);
  pthread_mutex_init(&rasterise_left, NULL);
  pthread_mutex_init(&rasterise_right, NULL);
  pthread_mutex_init(&rasterise_done, NULL);

  pixel_buffer_left = SDL_DuplicateSurface(pixel_array);
  // pthread_create(&thread_render_1, NULL, render_1, NULL);
  // pthread_detach(thread_render_1);

  pixel_buffer_right = SDL_DuplicateSurface(pixel_array);
  // pthread_create(&thread_render_2, NULL, render_2, NULL);
  // pthread_detach(thread_render_2);
}


// TRANSFORMATIONS
//-------------------------------------------------------------------------------
void translate_model(Model *model, float x, float y, float z)
{
  model->pos = vector3_add(model->pos, (Vector3){x, y, z});

  for (int i=0; i<model->poly_count; i++)
    for (int j=0; j<3; j++)
    {
      model->polygons[i].vertices[j].x += x;
      model->polygons[i].vertices[j].y += y;
      model->polygons[i].vertices[j].z += z;
      model->polygons[i].og_vertices[j].x += x;
      model->polygons[i].og_vertices[j].y += y;
      model->polygons[i].og_vertices[j].z += z;
    }
}

void translate_point(Vector3 *point, float x, float y, float z)
{
  point->x += x;
  point->y += y;
  point->z += z;
}

void translate_point_2d(Vector2 *point, float x, float y, float z)
{
  point->x += x;
  point->y += y;
  // point->z += z;
}

void rotate_point(Vector3 *pt, float x, float y, float z)
{
  float rot_x[3][3] = {
    { 1, 0,       0      },
    { 0, cos(x), -sin(x) },
    { 0, sin(x),  cos(x) }
  };

  float rot_y[3][3] = {
    { cos(y),  0, sin(y) },
    { 0,       1, 0      },
    { -sin(y), 0, cos(y) }
  };

  float rot_z[3][3] = {
    { cos(z), -sin(z), 0 },
    { sin(z), cos(z),  0 },
    { 0,      0,       1 }
  };

  float output[3][1];
  float output2[3][1];
  float output3[3][1];

  float pt_as_arr[3][1] = {{pt->x}, {pt->y}, {pt->z}};
  matrix_mult(3, 3, 3, 1, output, rot_y, pt_as_arr);
  matrix_mult(3, 3, 3, 1, output2, rot_x, output);
  matrix_mult(3, 3, 3, 1, output3, rot_z, output2);

  pt->x = output3[0][0];
  pt->y = output3[1][0];
  pt->z = output3[2][0];
}

void rotate_x(Model *model, float r)
{
  Vector3 model_pos = model->pos;
  translate_model(model, -model_pos.x, -model_pos.y, -model_pos.z);

  float rot_x[3][3] = {
    { 1, 0,       0      },
    { 0, cos(r), -sin(r) },
    { 0, sin(r),  cos(r) }
  };

  float result[3][1];

  for (int i=0; i<model->poly_count; i++)
  {
    // rotate vertices
    for (int j=0; j<3; j++)
    {
      float coord1[3][1] = {{model->polygons[i].vertices[j].x}, {model->polygons[i].vertices[j].y}, {model->polygons[i].vertices[j].z}};
      matrix_mult(3, 3, 3, 1, result, rot_x, coord1);
      model->polygons[i].vertices[j].x = result[0][0];
      model->polygons[i].vertices[j].y = result[1][0];
      model->polygons[i].vertices[j].z = result[2][0];

      float coord2[3][1] = {{model->polygons[i].og_vertices[j].x}, {model->polygons[i].og_vertices[j].y}, {model->polygons[i].og_vertices[j].z}};
      matrix_mult(3, 3, 3, 1, result, rot_x, coord2);
      model->polygons[i].og_vertices[j].x = result[0][0];
      model->polygons[i].og_vertices[j].y = result[1][0];
      model->polygons[i].og_vertices[j].z = result[2][0];
    }

    // rotate face normals
    float coord3[3][1] = {{model->polygons[i].face_normal.x}, {model->polygons[i].face_normal.y}, {model->polygons[i].face_normal.z}};
    matrix_mult(3, 3, 3, 1, result, rot_x, coord3);
    model->polygons[i].face_normal.x = result[0][0];
    model->polygons[i].face_normal.y = result[1][0];
    model->polygons[i].face_normal.z = result[2][0];
  }

  // rotate vertex normals
  for (int i=0; i<model->vertex_count; i++)
  {
    float coord4[3][1] = {{model->vertex_normals[i].x}, {model->vertex_normals[i].y}, {model->vertex_normals[i].z}};
    matrix_mult(3, 3, 3, 1, result, rot_x, coord4);
    model->vertex_normals[i].x = result[0][0];
    model->vertex_normals[i].y = result[1][0];
    model->vertex_normals[i].z = result[2][0];
  } 
  translate_model(model, model_pos.x, model_pos.y, model_pos.z);
}

void rotate_y(Model *model, float r)
{
  Vector3 model_pos = model->pos;
  translate_model(model, -model_pos.x, -model_pos.y, -model_pos.z);

  float rot_y[3][3] = {
    { cos(r),  0, sin(r) },
    { 0,       1, 0      },
    { -sin(r), 0, cos(r) }
  };

  float result[3][1];

  for (int i=0; i<model->poly_count; i++)
  {
    // rotate vertices
    for (int j=0; j<3; j++)
    {
      float coord1[3][1] = {{model->polygons[i].vertices[j].x}, {model->polygons[i].vertices[j].y}, {model->polygons[i].vertices[j].z}};
      matrix_mult(3, 3, 3, 1, result, rot_y, coord1);
      model->polygons[i].vertices[j].x = result[0][0];
      model->polygons[i].vertices[j].y = result[1][0];
      model->polygons[i].vertices[j].z = result[2][0];

      float coord2[3][1] = {{model->polygons[i].og_vertices[j].x}, {model->polygons[i].og_vertices[j].y}, {model->polygons[i].og_vertices[j].z}};
      matrix_mult(3, 3, 3, 1, result, rot_y, coord2);
      model->polygons[i].og_vertices[j].x = result[0][0];
      model->polygons[i].og_vertices[j].y = result[1][0];
      model->polygons[i].og_vertices[j].z = result[2][0];
    }

    // rotate normals
    float coord3[3][1] = {{model->polygons[i].face_normal.x}, {model->polygons[i].face_normal.y}, {model->polygons[i].face_normal.z}};
    matrix_mult(3, 3, 3, 1, result, rot_y, coord3);
    model->polygons[i].face_normal.x = result[0][0];
    model->polygons[i].face_normal.y = result[1][0];
    model->polygons[i].face_normal.z = result[2][0];
  }

  // rotate vertex normals
  for (int i=0; i<model->vertex_count; i++)
  {
    float coord4[3][1] = {{model->vertex_normals[i].x}, {model->vertex_normals[i].y}, {model->vertex_normals[i].z}};
    matrix_mult(3, 3, 3, 1, result, rot_y, coord4);
    model->vertex_normals[i].x = result[0][0];
    model->vertex_normals[i].y = result[1][0];
    model->vertex_normals[i].z = result[2][0];
  } 


  translate_model(model, model_pos.x, model_pos.y, model_pos.z);
}

void rotate_z(Model *model, float r)
{
  Vector3 model_pos = model->pos;
  translate_model(model, -model_pos.x, -model_pos.y, -model_pos.z);

  float rot_z[3][3] = {
    { cos(r), -sin(r), 0 },
    { sin(r), cos(r),  0 },
    { 0,      0,       1 }
  };

  float result[3][1];

  for (int i=0; i<model->poly_count; i++)
  {
    // rotate vertices
    for (int j=0; j<3; j++)
    {
      float coord1[3][1] = {{model->polygons[i].vertices[j].x}, {model->polygons[i].vertices[j].y}, {model->polygons[i].vertices[j].z}};
      matrix_mult(3, 3, 3, 1, result, rot_z, coord1);
      model->polygons[i].vertices[j].x = result[0][0];
      model->polygons[i].vertices[j].y = result[1][0];
      model->polygons[i].vertices[j].z = result[2][0];

      float coord2[3][1] = {{model->polygons[i].og_vertices[j].x}, {model->polygons[i].og_vertices[j].y}, {model->polygons[i].og_vertices[j].z}};
      matrix_mult(3, 3, 3, 1, result, rot_z, coord2);
      model->polygons[i].og_vertices[j].x = result[0][0];
      model->polygons[i].og_vertices[j].y = result[1][0];
      model->polygons[i].og_vertices[j].z = result[2][0];
    }

    // rotate normals
    float coord3[3][1] = {{model->polygons[i].face_normal.x}, {model->polygons[i].face_normal.y}, {model->polygons[i].face_normal.z}};
    matrix_mult(3, 3, 3, 1, result, rot_z, coord3);
    model->polygons[i].face_normal.x = result[0][0];
    model->polygons[i].face_normal.y = result[1][0];
    model->polygons[i].face_normal.z = result[2][0];
  }

  // rotate vertex normals
  for (int i=0; i<model->vertex_count; i++)
  {
    float coord4[3][1] = {{model->vertex_normals[i].x}, {model->vertex_normals[i].y}, {model->vertex_normals[i].z}};
    matrix_mult(3, 3, 3, 1, result, rot_z, coord4);
    model->vertex_normals[i].x = result[0][0];
    model->vertex_normals[i].y = result[1][0];
    model->vertex_normals[i].z = result[2][0];
  } 


  translate_model(model, model_pos.x, model_pos.y, model_pos.z);
}

void scale(Model *model, float alpha)
{
  for (int i=0; i<model->poly_count; i++)
    for (int j=0; j<3; j++)
      model->polygons[i].vertices[j] = vector3_scale(model->polygons[i].vertices[j], alpha);
}

void scale_xyz(Model *model, float x, float y, float z)
{
  for (int i=0; i<model->poly_count; i++)
    for (int j=0; j<3; j++)
    {
      model->polygons[i].vertices[j].x *= x;
      model->polygons[i].vertices[j].y *= y;
      model->polygons[i].vertices[j].z *= z;
      model->polygons[i].og_vertices[j].x *= x;
      model->polygons[i].og_vertices[j].y *= y;
      model->polygons[i].og_vertices[j].z *= z;
    }
}
//-------------------------------------------------------------------------------

// DRAWING
//-------------------------------------------------------------------------------
void clear_screen(SDL_Surface *pixel_arr, Uint8 r, Uint8 g, Uint8 b)
{
  // for (int i=0; i<SCREEN_WDTH; i++)
  //   for (int j=0; j<SCREEN_HGHT; j++)
  //     pixel(i, j, r, g, b);

  Uint32 red = r << 16;
  Uint16 green = g << 8;
  SDL_FillRect(pixel_arr, NULL, red + green + b);

  for (int i=0; i<SCREEN_WDTH; i++)
    for (int j=0; j<SCREEN_HGHT; j++)
      z_buffer[SCREEN_WDTH*j + i] = 0;
}

inline void pixel(SDL_Surface *pixel_arr, int x, int y, Uint8 r, Uint8 g, Uint8 b)
{
  Uint8 *const blue  = ((Uint8 *) pixel_arr->pixels + (y*4*SCREEN_WDTH) + (x*4 + 0));
  *blue = b;
  Uint8 *const green = ((Uint8 *) pixel_arr->pixels + (y*4*SCREEN_WDTH) + (x*4 + 1));
  *green = g;
  Uint8 *const red   = ((Uint8 *) pixel_arr->pixels + (y*4*SCREEN_WDTH) + (x*4 + 2));
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

  // If vertical
  if (m < -100 || m > 100)
  {
    if (p1.y < p2.y)
      for (int y=p1.y; y<p2.y; y++)
        pixel(pixel_arr, (int)p1.x, y, stroke.x, stroke.y, stroke.z);

    else if (p1.y > p2.y)
      for (int y=p2.y; y<p1.y; y++)
        pixel(pixel_arr, (int)p1.x, y, stroke.x, stroke.y, stroke.z);
  }

  // if gradient is not between -1 and 1
  else if (!in_range(m, -1, 1))
  {
    if (p1.y < p2.y)
      for (int y=p1.y; y<p2.y; y++)
        pixel(pixel_arr, (int)((y-c)/m), y, stroke.x, stroke.y, stroke.z);

    else if (p1.y > p2.y)
      for (int y=p2.y; y<p1.y; y++)
        pixel(pixel_arr, (int)((y-c)/m), y, stroke.x, stroke.y, stroke.z);
  }

  // if gradient is between -1 and 1
  else
  {
    if (p1.x < p2.x)
      for (int x=p1.x; x<=p2.x; x++)
        pixel(pixel_arr, x, (int)(m*x+c), stroke.x, stroke.y, stroke.z);

    else if (p1.x > p2.x)
      for (int x=p2.x; x<=p1.x; x++)
        pixel(pixel_arr, x, (int)(m*x+c), stroke.x, stroke.y, stroke.z);
  }
}

Vector3 calculate_barycentric(int x, int y, Vector2 v1, Vector2 v2, Vector2 v3)
{
  Vector3 weights;
  weights.x = ((v2.y-v3.y)*(x-v3.x) + (v3.x-v2.x)*(y-v3.y)) / ((v2.y-v3.y)*(v1.x-v3.x) + (v3.x-v2.x)*(v1.y-v3.y));
  weights.y = ((v3.y-v1.y)*(x-v3.x) + (v1.x-v3.x)*(y-v3.y)) / ((v2.y-v3.y)*(v1.x-v3.x) + (v3.x-v2.x)*(v1.y-v3.y));
  weights.z = 1 - weights.x - weights.y;
  return weights;
}

void triangle_2d(SDL_Surface *buffer, Polygon *tri)
{
  Vector2 v1 = tri->proj_verts[0];
  Vector2 v2 = tri->proj_verts[1];
  Vector2 v3 = tri->proj_verts[2];

  line_2d((Vector3){0, 0, 0}, (Vector2){v1.x, v1.y, 1}, (Vector2){v2.x, v2.y, 1}, buffer);
  line_2d((Vector3){0, 0, 0}, (Vector2){v2.x, v2.y, 1}, (Vector2){v3.x, v3.y, 1}, buffer);
  line_2d((Vector3){0, 0, 0}, (Vector2){v3.x, v3.y, 1}, (Vector2){v1.x, v1.y, 1}, buffer);

  // float inverse_u_coords[3] = {tri->uvs[0].x * v1.w, tri->uvs[1].x * v2.w, tri->uvs[2].x * v3.w};
  // float inverse_v_coords[3] = {tri->uvs[0].y * v1.w, tri->uvs[1].y * v2.w, tri->uvs[2].y * v3.w};

  // Uint16 lx = MIN(v1.x, MIN(v2.x, v3.x));
  // Uint16 hx = MAX(v1.x, MAX(v2.x, v3.x));
  // Uint16 ly = MIN(v1.y, MIN(v2.y, v3.y));
  // Uint16 hy = MAX(v1.y, MAX(v2.y, v3.y));

  // Uint16 u=0, v=0;
  // float z_index;

  // Vector3 vert_weights;
  // Uint8 *red, *green, *blue;
  // Uint8 *pixels = tri->texture->pixels;

  // for (Uint16 y=ly; y<=hy; y++)
  // {
  //   for (Uint16 x=lx; x<=hx; x++)
  //   {
  //     vert_weights = calculate_barycentric(x, y, v1, v2, v3);

  //     if (vert_weights.x >= 0 && vert_weights.y >= 0 && vert_weights.z >= 0)
  //     {
  //       z_index = v1.w*vert_weights.x + v2.w*vert_weights.y + v3.w*vert_weights.z;

  //       if (z_index > z_buffer[SCREEN_WDTH*y + x])
  //       {
  //         z_buffer[SCREEN_WDTH*y + x] = z_index;

  //         u = (Uint16)((vert_weights.x*inverse_u_coords[0] + vert_weights.y*inverse_u_coords[1] + vert_weights.z*inverse_u_coords[2]) / z_index) % tri->texture->w;
  //         v = (Uint16)((vert_weights.x*inverse_v_coords[0] + vert_weights.y*inverse_v_coords[1] + vert_weights.z*inverse_v_coords[2]) / z_index) % tri->texture->h;

  //         u *= tri->texture->format->BytesPerPixel;

  //         red   = pixels + v*tri->texture->pitch + u+2;
  //         green = pixels + v*tri->texture->pitch + u+1;
  //         blue  = pixels + v*tri->texture->pitch + u+0;

  //         pixel(buffer, x, y, *red, *green, *blue);
  //       }
  //     }
  //   }
  // }
}

/*
  // calculate normal tangent and bitangent
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

void triangle_2d_shaded(SDL_Surface *buffer, Polygon *tri)
{
  Vector2 v1 = tri->proj_verts[0];
  Vector2 v2 = tri->proj_verts[1];
  Vector2 v3 = tri->proj_verts[2];

  float inverse_u_coords[3] = {tri->uvs[0].x * v1.w, tri->uvs[1].x * v2.w, tri->uvs[2].x * v3.w};
  float inverse_v_coords[3] = {tri->uvs[0].y * v1.w, tri->uvs[1].y * v2.w, tri->uvs[2].y * v3.w};

  float shade1 = vector3_dot(tri->normals[0], lightsource);
  float shade2 = vector3_dot(tri->normals[1], lightsource);
  float shade3 = vector3_dot(tri->normals[2], lightsource);

  Uint16 lx = MIN(v1.x, MIN(v2.x, v3.x));
  Uint16 hx = MAX(v1.x, MAX(v2.x, v3.x));
  Uint16 ly = MIN(v1.y, MIN(v2.y, v3.y));
  Uint16 hy = MAX(v1.y, MAX(v2.y, v3.y));

  Uint16 u=0, v=0;
  float z_index;

  Vector3 vert_weights, light_weights;
  Uint8 *red, *green, *blue;
  Uint8 *pixels = tri->texture->pixels;
  Uint8 *nmap = tri->normal_map->pixels;

  for (Uint16 x=lx; x<=hx; x++)
  {
    for (Uint16 y=ly; y<=hy; y++)
    {
      vert_weights = calculate_barycentric(x, y, v1, v2, v3);

      if (vert_weights.x >= 0 && vert_weights.y >= 0 && vert_weights.z >= 0)
      {
        z_index = v1.w*vert_weights.x + v2.w*vert_weights.y + v3.w*vert_weights.z;

        if (z_index > z_buffer[SCREEN_WDTH*y + x])
        {
          z_buffer[SCREEN_WDTH*y + x] = z_index;

          u = (Uint16)((vert_weights.x*inverse_u_coords[0] + vert_weights.y*inverse_u_coords[1] + vert_weights.z*inverse_u_coords[2]) / z_index) % tri->texture->w;
          v = (Uint16)((vert_weights.x*inverse_v_coords[0] + vert_weights.y*inverse_v_coords[1] + vert_weights.z*inverse_v_coords[2]) / z_index) % tri->texture->h;

          u *= tri->texture->format->BytesPerPixel;

          red   = pixels + v*tri->texture->pitch + u+2;
          green = pixels + v*tri->texture->pitch + u+1;
          blue  = pixels + v*tri->texture->pitch + u+0;

          float shade = vert_weights.x*shade1 + vert_weights.y*shade2 + vert_weights.z*shade3;
          shade += 1;
          shade /= 2;

          float r = *red;
          float g = *green;
          float b = *blue;

          r *= shade;
          g *= shade; 
          b *= shade; 

          pixel(buffer, x, y, (Uint8)r, (Uint8)g, (Uint8)b);
        }
      }
    }
  }
}

/**
 * @return __m128 where each element is the weighting of each (x, y) with respect to the first input vertex
 */
__m128 SIMD_calculate_barycentric_first(__m128 *_x, __m128 *_y, __m128 *_v3x, __m128 *_v3y, __m128 *_v2y_take_v3y, __m128 *_v3x_take_v2x, float *denom)
{
  // first = ((v2.y-v3.y)*(x-v3.x) + (v3.x-v2.x)*(y-v3.y)) / denom
  __m128 _first = _mm_sub_ps(*_x, *_v3x);
  _first = _mm_mul_ps(*_v2y_take_v3y, _first);
  _first = _mm_add_ps(_first, _mm_mul_ps(*_v3x_take_v2x, _mm_sub_ps(*_y, *_v3y)));
  _first = _mm_div_ps(_first, _mm_load1_ps(denom));
  return _first;
}

/**
 * @return __m128 where each element is the weighting of each (x, y) with respect to the second input vertex
 */
__m128 SIMD_calculate_barycentric_second(__m128 *_x, __m128 *_y, __m128 *_v3x, __m128 *_v3y, __m128 *_v3y_take_v1y, __m128 *_v1x_take_v3x, float *denom)
{
  // second = ((v3.y-v1.y)*(x-v3.x) + (v1.x-v3.x)*(y-v3.y)) / denom
  __m128 _second = _mm_sub_ps(*_x, *_v3x);
  _second = _mm_mul_ps(*_v3y_take_v1y, _second);
  _second = _mm_add_ps(_second, _mm_mul_ps(*_v1x_take_v3x, _mm_sub_ps(*_y, *_v3y)));
  _second = _mm_div_ps(_second, _mm_load1_ps(denom));
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

void SIMD_triangle_2d(SDL_Surface *buffer, Model *model, Polygon *tri)
{
  Vector2 v1 = GE_world_to_screen(&tri->vertices[0]);
  Vector2 v2 = GE_world_to_screen(&tri->vertices[1]);
  Vector2 v3 = GE_world_to_screen(&tri->vertices[2]);

  Vector3 tex_inv_x = (Vector3){tri->uvs[0].x * v1.w, tri->uvs[1].x * v2.w, tri->uvs[2].x * v3.w};
  Vector3 tex_inv_y = (Vector3){tri->uvs[0].y * v1.w, tri->uvs[1].y * v2.w, tri->uvs[2].y * v3.w};
  Uint8 *red, *green, *blue;
  Uint8 *pixels = model->materials[tri->mat_index]->pixels;


  __m128 _vertices_x = _mm_set_ps(v1.x, v2.x, v3.x, 1);
  __m128 _vertices_y = _mm_set_ps(v1.y, v2.y, v3.y, 1);
  __m128 _vertices_w = _mm_set_ps(v1.w, v2.w, v3.w, 1);

  __m128 _v3x = _mm_set1_ps(v3.x);
  __m128 _v3y = _mm_set1_ps(v3.y);

  __m128 _v2y_take_v3y = _mm_set1_ps(v2.y-v3.y);
  __m128 _v3x_take_v2x = _mm_set1_ps(v3.x-v2.x);

  __m128 _v3y_take_v1y = _mm_set1_ps(v3.y-v1.y);
  __m128 _v1x_take_v3x = _mm_set1_ps(v1.x-v3.x);

  float denom = (v2.y-v3.y)*(v1.x-v3.x) + (v3.x-v2.x)*(v1.y-v3.y);

  Uint16 lx = MIN(v1.x, MIN(v2.x, v3.x));
  Uint16 hx = MAX(v1.x, MAX(v2.x, v3.x));
  Uint16 ly = MIN(v1.y, MIN(v2.y, v3.y));
  Uint16 hy = MAX(v1.y, MAX(v2.y, v3.y));

  Uint16 x=0, y=0, u=0, v=0;
  Vector3 vert_weights;
  float z_index;

  __m128 _x, _y;
  __m128 _ones = _mm_set1_ps(1);
  __m128 _zeroes = _mm_set1_ps(0.0f);
  __m128 _threes = _mm_set1_ps(3);

  __m128 _v1w = _mm_set1_ps(v1.w);
  __m128 _v2w = _mm_set1_ps(v2.w);
  __m128 _v3w = _mm_set1_ps(v3.w);
  __m128 _v1w_by_first;
  __m128 _v2w_by_second;
  __m128 _v3w_by_third;
  __m128 _z_index;

  __m128 _first;
  __m128 _second;
  __m128 _third;

  Uint16 y_range = hy-ly;
  Uint8 remainder = y_range % 4;

  for (x=lx; x<=hx; x++)
  {
    for (y=ly; y<=hy-remainder-1; y+=4)
    {
      _x = _mm_set1_ps(x);
      _y = _mm_set_ps(y+0, y+1, y+2, y+3);
      
      // Calculate barycentric coordinates
      _first = SIMD_calculate_barycentric_first(&_x, &_y, &_v3x, &_v3y, &_v2y_take_v3y, &_v3x_take_v2x, &denom);
      _second = SIMD_calculate_barycentric_first(&_x, &_y, &_v3x, &_v3y, &_v3y_take_v1y, &_v1x_take_v3x, &denom);
      SIMD_calculate_barycentric_third(&_ones, &_first, &_second, &_third);

    

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

  if (d0 >= 0)
  {
    inside_verts[inside_count] = &tri_in->vertices[0];
    inside_uvs[inside_count] = &tri_in->uvs[0];
    inside_normals[inside_count++] = &tri_in->normals[0];
    inside_index = 0;
  }
  else
  {
    outside_verts[outside_count] = &tri_in->vertices[0];
    outside_uvs[outside_count] = &tri_in->uvs[0];
    outside_normals[outside_count++] = &tri_in->normals[0];
    outside_index = 0;
  }

  if (d1 >= 0)
  {
    inside_verts[inside_count] = &tri_in->vertices[1];
    inside_uvs[inside_count] = &tri_in->uvs[1];
    inside_normals[inside_count++] = &tri_in->normals[1];
    inside_index = 1;
  }
  else
  {
    outside_verts[outside_count] = &tri_in->vertices[1];
    outside_uvs[outside_count] = &tri_in->uvs[1];
    outside_normals[outside_count++] = &tri_in->normals[1];
    outside_index = 1;
  }

  if (d2 >= 0)
  {
    inside_verts[inside_count] = &tri_in->vertices[2];
    inside_uvs[inside_count] = &tri_in->uvs[2];
    inside_normals[inside_count++] = &tri_in->normals[2];
    inside_index = 2;
  }
  else
  {
    outside_verts[outside_count] = &tri_in->vertices[2];
    outside_uvs[outside_count] = &tri_in->uvs[2];
    outside_normals[outside_count++] = &tri_in->normals[2];
    outside_index = 2;
  }



  int insd_0 = inside_index;
  int insd_1 = (inside_index+1)%3;
  int insd_2 = (inside_index+2)%3;

  int outsd_0 = outside_index;
  int outsd_1 = (outside_index+1)%3;
  int outsd_2 = (outside_index+2)%3;

  switch (inside_count)
  {
    case (0): return 0;

    case (3):
      *tri_out1 = *tri_in;
      return 1;

    case (1): // 1 point inside, 2 outside
      *tri_out1 = *tri_in;

      tri_out1->vertices[insd_0] = *inside_verts[0];
      tri_out1->vertices[insd_1] = line_plane_intersect(*plane_pos, *plane_normal, *inside_verts[0], *outside_verts[0], &t);
      tri_out1->uvs[insd_1].x = inside_uvs[0]->x + t*(outside_uvs[0]->x - inside_uvs[0]->x);
      tri_out1->uvs[insd_1].y = inside_uvs[0]->y + t*(outside_uvs[0]->y - inside_uvs[0]->y);

      tri_out1->vertices[insd_2] = line_plane_intersect(*plane_pos, *plane_normal, *inside_verts[0], *outside_verts[1], &t);
      tri_out1->uvs[insd_2].x = inside_uvs[0]->x + t*(outside_uvs[1]->x - inside_uvs[0]->x);
      tri_out1->uvs[insd_2].y = inside_uvs[0]->y + t*(outside_uvs[1]->y - inside_uvs[0]->y);

      return 1;

    case (2): // 2 points inside, 1 outside

      *tri_out1 = *tri_in;
      tri_out1->vertices[outsd_0] = *inside_verts[0];
      tri_out1->vertices[outsd_1] = *inside_verts[1];
      tri_out1->vertices[outsd_2] = line_plane_intersect(*plane_pos, *plane_normal, *inside_verts[0], *outside_verts[0], &t);

      tri_out1->uvs[outsd_0] = *inside_uvs[0];
      tri_out1->uvs[outsd_1] = *inside_uvs[1];
      tri_out1->uvs[outsd_2].x = outside_uvs[0]->x + (1-t)*(inside_uvs[0]->x - outside_uvs[0]->x);
      tri_out1->uvs[outsd_2].y = outside_uvs[0]->y + (1-t)*(inside_uvs[0]->y - outside_uvs[0]->y);
      

      *tri_out2 = *tri_in;
      tri_out2->vertices[outsd_0] = *inside_verts[1];
      tri_out2->vertices[outsd_1] = tri_out1->vertices[outsd_2];
      tri_out2->vertices[outsd_2] = line_plane_intersect(*plane_pos, *plane_normal, *inside_verts[1], *outside_verts[0], &t);

      tri_out2->uvs[outsd_0] = *inside_uvs[1];
      tri_out2->uvs[outsd_1] = tri_out1->uvs[outsd_2];
      tri_out2->uvs[outsd_2].x = outside_uvs[0]->x + (1-t)*(inside_uvs[1]->x - outside_uvs[0]->x);
      tri_out2->uvs[outsd_2].y = outside_uvs[0]->y + (1-t)*(inside_uvs[1]->y - outside_uvs[0]->y);
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


/** Calculate the slope of the line formed between two points in 2D space
 */
float GE_calc_slope(Vector2 *p1, Vector2 *p2)
{
  return (p1->y - p2->y) / (p1->x - p2->x);
}

/** Calculate the constant c (y-intercept) of the equation y = mx + c
 */
float GE_calc_y_intercept(Vector2 *p1, float *slope)
{
  return p1->y - *slope*p1->x;
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

//             copy data
// input  -->  transformation  -->  clipping     -->  rasterisation
// model  -->  front_faces     -->  front_faces  -->  free(front_faces)

/** Translate all polygons in GE_transform_queue by -GE_cam.pos, rotate them by -GE_cam.rot and move them to GE_clip_queue.
 *  GE_transform_queue will be emptied.
 */
void GE_queue_perform_transformation(void)
{
  int size = GE_transform_queue->size;

  for (int i=0; i<size; i++)
  {
    Polygon tri = *RSR_front(GE_transform_queue);
    RSR_dequeue(GE_transform_queue);

    for (int j=0; j<3; j++)
    {
      tri.vertices[j] = vector3_sub(tri.vertices[j], *GE_cam->pos);
      // tri.og_vertices[j] = vector3_sub(tri.og_vertices[j], *GE_cam->pos);

      rotate_point(&tri.vertices[j], GE_cam->rot.x, GE_cam->rot.y, 0);
      // rotate_point(&tri.og_vertices[j], GE_cam->rot.x, GE_cam->rot.y, 0);
    }
  if (tri.vertices[0].z > 1 || tri.vertices[1].z > 1 || tri.vertices[2].z > 1)
    RSR_enque(GE_clip_queue, &tri);
  }
}

void GE_clip_2D_to_center(Polygon *tri)
{
  Polygon out1 = *tri, out2 = *tri, out3 = *tri;

  Vector2 *left_verts[3]; int left_count = 0;
  Vector2 *right_verts[3]; int right_count = 0;

  int left_index = 0;
  int right_index = 0;

  // get signed distance between each vertex and center
  // negative distance means point is on left of screen,
  // positive means point is on right of screen

  float d0 = tri->proj_verts[0].x - HALF_SCREEN_WDTH;
  float d1 = tri->proj_verts[1].x - HALF_SCREEN_WDTH;
  float d2 = tri->proj_verts[2].x - HALF_SCREEN_WDTH;

  if (d0 >= 0)
  {
    right_verts[right_count] = &tri->proj_verts[0];
    right_count += 1;
    right_index = 0;
  }
  else
  {
    left_verts[left_count] = &tri->proj_verts[0];
    left_count += 1;
    left_index = 0;
  }

  if (d1 >= 0)
  {
    right_verts[right_count] = &tri->proj_verts[1];
    right_count += 1;
    right_index = 1;
  }
  else
  {
    left_verts[left_count] = &tri->proj_verts[1];
    left_count += 1;
    left_index = 1;
  }

  if (d2 >= 0)
  {
    right_verts[right_count] = &tri->proj_verts[2];
    right_count += 1;
    right_index = 2;
  }
  else
  {
    left_verts[left_count] = &tri->proj_verts[2];
    left_count += 1;
    left_index = 2;
  }

  float slope, c;

  switch (left_count)
  {
    case (1): // 1 new tri on left, 2 new tris on right

      // Left triangle 
      int left = left_index;
      int right1 = (left_index+1)%3;
      int right2 = (left_index+2)%3;

      out1.proj_verts[left] = *left_verts[0];

      slope = GE_calc_slope(left_verts[0], right_verts[0]);
      c = GE_calc_y_intercept(left_verts[0], &slope);
      out1.proj_verts[right1].x = HALF_SCREEN_WDTH-1;
      out1.proj_verts[right1].y = slope*(HALF_SCREEN_WDTH-1) + c;
      if (out1.proj_verts[right1].y >= SCREEN_HGHT)
        return;

      slope = GE_calc_slope(left_verts[0], right_verts[1]);
      c = GE_calc_y_intercept(left_verts[0], &slope);
      out1.proj_verts[right2].x = HALF_SCREEN_WDTH-1;
      out1.proj_verts[right2].y = slope*(HALF_SCREEN_WDTH-1) + c;
      if (out1.proj_verts[right2].y >= SCREEN_HGHT)
        return;

      RSR_enque(GE_rasterise_left, &out1);


      // // Right triangle
      slope = GE_calc_slope(left_verts[0], right_verts[0]);
      c = GE_calc_y_intercept(left_verts[0], &slope);
      out2.proj_verts[0] = *right_verts[0];
      out2.proj_verts[1] = *right_verts[1];
      out2.proj_verts[2].x = HALF_SCREEN_WDTH;
      out2.proj_verts[2].y = slope*HALF_SCREEN_WDTH + c;
      if (out2.proj_verts[2].y >= SCREEN_HGHT)
        return;
      RSR_enque(GE_rasterise_right, &out2);


      slope = GE_calc_slope(left_verts[0], right_verts[1]);
      c = GE_calc_y_intercept(left_verts[0], &slope);
      out3.proj_verts[0] = *right_verts[1];
      out3.proj_verts[1] = out2.proj_verts[2];
      out3.proj_verts[2].x = HALF_SCREEN_WDTH;
      out3.proj_verts[2].y = slope*HALF_SCREEN_WDTH + c;
      if (out3.proj_verts[2].y >= SCREEN_HGHT)
        return;
      RSR_enque(GE_rasterise_right, &out3);

      break;

    case (2): // 2 new tris on left, 1 new tri on right

      int right = right_index;
      int left1 = (right_index+1)%3;
      int left2 = (right_index+2)%3;
      
      slope = GE_calc_slope(right_verts[0], left_verts[0]);
      c = GE_calc_y_intercept(right_verts[0], &slope);
      out1.proj_verts[left1].x = HALF_SCREEN_WDTH;
      out1.proj_verts[left1].y = slope*(HALF_SCREEN_WDTH) + c;
      if (out1.proj_verts[right1].y >= SCREEN_HGHT)
        return;

      slope = GE_calc_slope(right_verts[0], left_verts[1]);
      c = GE_calc_y_intercept(right_verts[0], &slope);
      out1.proj_verts[left2].x = HALF_SCREEN_WDTH;
      out1.proj_verts[left2].y = slope*(HALF_SCREEN_WDTH) + c;
      if (out1.proj_verts[right2].y >= SCREEN_HGHT)
        return;
      RSR_enque(GE_rasterise_right, &out1);


      slope = GE_calc_slope(right_verts[0], left_verts[0]);
      c = GE_calc_y_intercept(right_verts[0], &slope);
      out2.proj_verts[0] = *left_verts[0];
      out2.proj_verts[1] = *left_verts[1];
      out2.proj_verts[2].x = HALF_SCREEN_WDTH-1;
      out2.proj_verts[2].y = slope*(HALF_SCREEN_WDTH-1) + c;
      if (out2.proj_verts[2].y >= SCREEN_HGHT)
        return;
      RSR_enque(GE_rasterise_left, &out2);


      slope = GE_calc_slope(right_verts[0], left_verts[1]);
      c = GE_calc_y_intercept(right_verts[0], &slope);
      out3.proj_verts[0] = *left_verts[1];
      out3.proj_verts[1] = out2.proj_verts[2];
      out3.proj_verts[2].x = HALF_SCREEN_WDTH-1;
      out3.proj_verts[2].y = slope*(HALF_SCREEN_WDTH-1) + c;
      if (out3.proj_verts[2].y >= SCREEN_HGHT)
        return;
      RSR_enque(GE_rasterise_left, &out3);

      break;
  }
}

void GE_queue_perform_clipping(void)
{
  GE_clip_queue_3D((Vector3){0, 0, 1}, GE_cam->near_plane, GE_clip_queue, GE_clip_queue);
  GE_clip_queue_3D((Vector3){0, 0, 0}, GE_cam->left_plane, GE_clip_queue, GE_clip_queue);
  GE_clip_queue_3D((Vector3){0, 0, 0}, GE_cam->right_plane, GE_clip_queue, GE_clip_queue);
  GE_clip_queue_3D((Vector3){0, 0, 0}, GE_cam->top_plane, GE_clip_queue, GE_clip_queue);
  GE_clip_queue_3D((Vector3){0, 0, 0}, GE_cam->bottom_plane, GE_clip_queue, GE_rasterise_queue);
}


/** Rasterise all polygons in GE_rasterise_queue
 *  GE_rasterise_queue will be emptied.
 */
void GE_queue_perform_rasterisation(void)
{
  clear_screen(pixel_array, 200, 200, 200);
  int size = GE_rasterise_queue->size;

  for (int i=0; i<size; i++)
  {
    Polygon tri = *RSR_front(GE_rasterise_queue);
    RSR_dequeue(GE_rasterise_queue);
   
    for (int j=0; j<3; j++)
      tri.proj_verts[j] = GE_world_to_screen(&tri.vertices[j]);


    if (tri.proj_verts[0].x < HALF_SCREEN_WDTH && tri.proj_verts[1].x < HALF_SCREEN_WDTH && tri.proj_verts[2].x < HALF_SCREEN_WDTH)
      RSR_enque(GE_rasterise_left, &tri);
    else if (tri.proj_verts[0].x >= HALF_SCREEN_WDTH && tri.proj_verts[1].x >= HALF_SCREEN_WDTH && tri.proj_verts[2].x >= HALF_SCREEN_WDTH)
      RSR_enque(GE_rasterise_right, &tri);
    else
      GE_clip_2D_to_center(&tri);

  }

  pthread_create(&thread_render_1, NULL, render_1, NULL);
  pthread_create(&thread_render_2, NULL, render_2, NULL);

  pthread_join(thread_render_1, NULL);
  pthread_join(thread_render_2, NULL);

}


/** Project a 3D world coordinate onto a 2D screen coordinate.
 * z coordinate is preserved for z-buffer.
 */
Vector2 GE_world_to_screen(Vector3 *pt)
{
  float nearplane_width = HALF_SCREEN_WDTH;
  float nearplane_height = HALF_SCREEN_HGHT;
  float nearplane_z = 1;

  float canvas_x = (nearplane_z/pt->z) * pt->x * nearplane_z * nearplane_width;
  float canvas_y = (nearplane_z/pt->z) * pt->y * nearplane_z * nearplane_height;
  canvas_x += HALF_SCREEN_WDTH - 0.5;
  canvas_y += HALF_SCREEN_HGHT - 0.5;

  return (Vector2){canvas_x, canvas_y, 1/pt->z};
}

