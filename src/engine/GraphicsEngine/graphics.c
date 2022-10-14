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

SDL_Surface *pixel_array;

float z_buffer[SCREEN_WIDTH * SCREEN_HEIGHT];

Vector3 lightsource = {0, -1, 0};

double delta_time = 0.001;
Camera *GE_cam;

RSR_queue_t *GE_transform_queue;  // Queue of polygons waiting to be rotated
RSR_queue_t *GE_clip_queue;       // Queue of polygons waiting to be clipped
RSR_queue_t *GE_rasterise_queue;  // Queue of polygons waiting to be rasterised

Polygon *GE_clipped_polygons;
Polygon *front_faces;


float *precomputed_sine;

float *precompute_sine(void)
{
  float *sine_arr = (float *)malloc(360 * sizeof(float));
  for (int i=0; i<360; i++)
    sine_arr[i] = sin((i*3.1415) / 180);
  return sine_arr;
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
  matrix_mult(3, 3, 3, 1, output, rot_x, pt_as_arr);
  matrix_mult(3, 3, 3, 1, output2, rot_y, output);
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
void clear_screen(Uint8 r, Uint8 g, Uint8 b)
{
  // for (int i=0; i<SCREEN_WIDTH; i++)
  //   for (int j=0; j<SCREEN_HEIGHT; j++)
  //     pixel(i, j, r, g, b);

  Uint32 red = r << 16;
  Uint16 green = g << 8;
  SDL_FillRect(pixel_array, NULL, red + green + b);

  for (int i=0; i<SCREEN_WIDTH; i++)
    for (int j=0; j<SCREEN_HEIGHT; j++)
      z_buffer[SCREEN_WIDTH*j + i] = 0;
}

inline void pixel(int x, int y, Uint8 r, Uint8 g, Uint8 b)
{
  // printf("%d %d\n", x, y);
  Uint8 *const blue  = ((Uint8 *) pixel_array->pixels + (y*4*SCREEN_WIDTH) + (x*4 + 0));
  *blue = b;
  Uint8 *const green = ((Uint8 *) pixel_array->pixels + (y*4*SCREEN_WIDTH) + (x*4 + 1));
  *green = g;
  Uint8 *const red   = ((Uint8 *) pixel_array->pixels + (y*4*SCREEN_WIDTH) + (x*4 + 2));
  *red = r;
}

bool in_range(float n, float l, float u)
{
  return (n >= l && n <= u) ? true : false;
}

void line_2d(Vector3 stroke, Vector2 p1, Vector2 p2)
{
  float m = (p1.y-p2.y) / (p1.x-p2.x); // slope
  float c = p1.y - m*p1.x; // constant

  // If vertical
  if (m < -100 || m > 100)
  {
    if (p1.y < p2.y)
      for (int y=p1.y; y<p2.y; y++)
        pixel((int)p1.x, y, stroke.x, stroke.y, stroke.z);

    else if (p1.y > p2.y)
      for (int y=p2.y; y<p1.y; y++)
        pixel((int)p1.x, y, stroke.x, stroke.y, stroke.z);
  }

  // if gradient is not between -1 and 1
  else if (!in_range(m, -1, 1))
  {
    if (p1.y < p2.y)
      for (int y=p1.y; y<p2.y; y++)
        pixel((int)((y-c)/m), y, stroke.x, stroke.y, stroke.z);

    else if (p1.y > p2.y)
      for (int y=p2.y; y<p1.y; y++)
        pixel((int)((y-c)/m), y, stroke.x, stroke.y, stroke.z);
  }

  // if gradient is between -1 and 1
  else
  {
    if (p1.x < p2.x)
      for (int x=p1.x; x<=p2.x; x++)
        pixel(x, (int)(m*x+c), stroke.x, stroke.y, stroke.z);

    else if (p1.x > p2.x)
      for (int x=p2.x; x<=p1.x; x++)
        pixel(x, (int)(m*x+c), stroke.x, stroke.y, stroke.z);
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

void triangle_2d_gouraud(Model *model, Polygon *tri)
{
  Vector2 v1 = project_coordinate(&tri->vertices[0]);
  Vector2 v2 = project_coordinate(&tri->vertices[1]);
  Vector2 v3 = project_coordinate(&tri->vertices[2]);

  Vector2 ov1 = project_coordinate(&tri->og_vertices[0]);
  Vector2 ov2 = project_coordinate(&tri->og_vertices[1]);
  Vector2 ov3 = project_coordinate(&tri->og_vertices[2]);

  Vector3 norm1 = tri->normals[0];
  Vector3 norm2 = tri->normals[1];
  Vector3 norm3 = tri->normals[2];

  float ilum1 = (vector3_dot(norm1, lightsource) + 1) / 2;
  float ilum2 = (vector3_dot(norm2, lightsource) + 1) / 2;
  float ilum3 = (vector3_dot(norm3, lightsource) + 1) / 2;

  __m128 _reg_uv_x = _mm_set_ps(tri->uvs[0].x, tri->uvs[1].x, tri->uvs[2].x, 1);
  __m128 _reg_uv_y = _mm_set_ps(tri->uvs[0].y, tri->uvs[1].y, tri->uvs[2].y, 1);
  __m128 _reg_invz = _mm_set_ps(v1.w,          v2.w,          v3.w,          1);

  _reg_uv_x = _mm_mul_ps(_reg_uv_x, _reg_invz);
  _reg_uv_y = _mm_mul_ps(_reg_uv_y, _reg_invz);

  // line_2d((Vector3){0, 0, 0}, (Vector2){v1.x, v1.y, 1}, (Vector2){v2.x, v2.y, 1});
  // line_2d((Vector3){0, 0, 0}, (Vector2){v2.x, v2.y, 1}, (Vector2){v3.x, v3.y, 1});
  // line_2d((Vector3){0, 0, 0}, (Vector2){v3.x, v3.y, 1}, (Vector2){v1.x, v1.y, 1});

  Uint16 lx = MIN(v1.x, MIN(v2.x, v3.x));
  Uint16 hx = MAX(v1.x, MAX(v2.x, v3.x));
  Uint16 ly = MIN(v1.y, MIN(v2.y, v3.y));
  Uint16 hy = MAX(v1.y, MAX(v2.y, v3.y));

  Uint16 x=0, y=0, u=0, v=0;
  float z_index;

  Vector3 vert_weights;
  Vector3 light_weights;
  vector3_normalise(&lightsource);
  float shade;

  for (x=lx; x<=hx; x++)
  {
    for (y=ly; y<=hy-0; y+=1)
    {
      vert_weights = calculate_barycentric(x, y, v1, v2, v3);

      if (vert_weights.x >= 0 && vert_weights.y >= 0 && vert_weights.z >= 0)
      {
        z_index = v1.w*vert_weights.x + v2.w*vert_weights.y + v3.w*vert_weights.z;

        if (z_index > z_buffer[SCREEN_WIDTH*y + x])
        {
          z_buffer[SCREEN_WIDTH*y + x] = z_index;

          light_weights = calculate_barycentric(x, y, ov1, ov2, ov3);
          
          shade = light_weights.x*ilum1 + light_weights.y*ilum2 + light_weights.z*ilum3;

          u = (Uint16)((vert_weights.x*_reg_uv_x[3] + vert_weights.y*_reg_uv_x[2] + vert_weights.z*_reg_uv_x[1]) / z_index);
          v = (Uint16)((vert_weights.x*_reg_uv_y[3] + vert_weights.y*_reg_uv_y[2] + vert_weights.z*_reg_uv_y[1]) / z_index);

          u *= model->materials[tri->mat_index]->format->BytesPerPixel;

          Uint8 *red = (Uint8 *)model->materials[tri->mat_index]->pixels + v*model->materials[tri->mat_index]->pitch + u+2;
          Uint8 *green = (Uint8 *)model->materials[tri->mat_index]->pixels + v*model->materials[tri->mat_index]->pitch + u+1;
          Uint8 *blue = (Uint8 *)model->materials[tri->mat_index]->pixels + v*model->materials[tri->mat_index]->pitch + u+0;

          float r = *red * shade;
          float g = *green * shade;
          float b = *blue * shade;

          pixel(x, y, (Uint8)r, (Uint8)g, (Uint8)b);
        }
      }
    }
  }
}

void triangle_2d_phong(Model *model, Polygon *tri)
{
  Vector2 v1 = project_coordinate(&tri->vertices[0]);
  Vector2 v2 = project_coordinate(&tri->vertices[1]);
  Vector2 v3 = project_coordinate(&tri->vertices[2]);

  Vector2 ov1 = project_coordinate(&tri->og_vertices[0]);
  Vector2 ov2 = project_coordinate(&tri->og_vertices[1]);
  Vector2 ov3 = project_coordinate(&tri->og_vertices[2]);

  Vector3 norm1 = tri->normals[0];
  Vector3 norm2 = tri->normals[1];
  Vector3 norm3 = tri->normals[2];

  __m128 _reg_uv_x = _mm_set_ps(tri->uvs[0].x, tri->uvs[1].x, tri->uvs[2].x, 1);
  __m128 _reg_uv_y = _mm_set_ps(tri->uvs[0].y, tri->uvs[1].y, tri->uvs[2].y, 1);
  __m128 _reg_invz = _mm_set_ps(v1.w,          v2.w,          v3.w,          1);

  _reg_uv_x = _mm_mul_ps(_reg_uv_x, _reg_invz);
  _reg_uv_y = _mm_mul_ps(_reg_uv_y, _reg_invz);

  // line_2d((Vector3){0, 0, 0}, (Vector2){v1.x, v1.y, 1}, (Vector2){v2.x, v2.y, 1});
  // line_2d((Vector3){0, 0, 0}, (Vector2){v2.x, v2.y, 1}, (Vector2){v3.x, v3.y, 1});
  // line_2d((Vector3){0, 0, 0}, (Vector2){v3.x, v3.y, 1}, (Vector2){v1.x, v1.y, 1});

  Uint16 lx = MIN(v1.x, MIN(v2.x, v3.x));
  Uint16 hx = MAX(v1.x, MAX(v2.x, v3.x));
  Uint16 ly = MIN(v1.y, MIN(v2.y, v3.y));
  Uint16 hy = MAX(v1.y, MAX(v2.y, v3.y));

  Uint16 x=0, y=0, u=0, v=0;
  float z_index;

  Vector3 vert_weights;
  Vector3 light_weights;
  vector3_normalise(&lightsource);
  float shade;

  for (x=lx; x<=hx; x++)
  {
    for (y=ly; y<=hy-0; y+=1)
    {
      vert_weights = calculate_barycentric(x, y, v1, v2, v3);

      if (vert_weights.x >= 0 && vert_weights.y >= 0 && vert_weights.z >= 0)
      {
        z_index = v1.w*vert_weights.x + v2.w*vert_weights.y + v3.w*vert_weights.z;

        if (z_index > z_buffer[SCREEN_WIDTH*y + x])
        {
          z_buffer[SCREEN_WIDTH*y + x] = z_index;

          light_weights = calculate_barycentric(x, y, ov1, ov2, ov3);
          
          Vector3 N1 = vector3_scale(norm1, light_weights.x);
          Vector3 N2 = vector3_scale(norm2, light_weights.y);
          Vector3 N3 = vector3_scale(norm3, light_weights.z);
          
          Vector3 nn = vector3_add(N1, vector3_add(N2, N3));
          vector3_normalise(&nn);

          shade = (vector3_dot(lightsource, nn) + 1) / 2;

          u = (Uint16)((vert_weights.x*_reg_uv_x[3] + vert_weights.y*_reg_uv_x[2] + vert_weights.z*_reg_uv_x[1]) / z_index);
          v = (Uint16)((vert_weights.x*_reg_uv_y[3] + vert_weights.y*_reg_uv_y[2] + vert_weights.z*_reg_uv_y[1]) / z_index);

          u *= model->materials[tri->mat_index]->format->BytesPerPixel;

          Uint8 *red = (Uint8 *)model->materials[tri->mat_index]->pixels + v*model->materials[tri->mat_index]->pitch + u+2;
          Uint8 *green = (Uint8 *)model->materials[tri->mat_index]->pixels + v*model->materials[tri->mat_index]->pitch + u+1;
          Uint8 *blue = (Uint8 *)model->materials[tri->mat_index]->pixels + v*model->materials[tri->mat_index]->pitch + u+0;

          float r = *red * shade;
          float g = *green * shade;
          float b = *blue * shade;

          pixel(x, y, (Uint8)r, (Uint8)g, (Uint8)b);
        }
      }
    }
  }
}

void triangle_2d_flat(Model *model, Polygon *tri)
{
  Vector2 v1 = project_coordinate(&tri->vertices[0]);
  Vector2 v2 = project_coordinate(&tri->vertices[1]);
  Vector2 v3 = project_coordinate(&tri->vertices[2]);

  __m128 _reg_uv_x = _mm_set_ps(tri->uvs[0].x, tri->uvs[1].x, tri->uvs[2].x, 1);
  __m128 _reg_uv_y = _mm_set_ps(tri->uvs[0].y, tri->uvs[1].y, tri->uvs[2].y, 1);
  __m128 _reg_invz = _mm_set_ps(v1.w,          v2.w,          v3.w,          1);

  _reg_uv_x = _mm_mul_ps(_reg_uv_x, _reg_invz);
  _reg_uv_y = _mm_mul_ps(_reg_uv_y, _reg_invz);

  Uint16 lx = MIN(v1.x, MIN(v2.x, v3.x));
  Uint16 hx = MAX(v1.x, MAX(v2.x, v3.x));
  Uint16 ly = MIN(v1.y, MIN(v2.y, v3.y));
  Uint16 hy = MAX(v1.y, MAX(v2.y, v3.y));

  Uint16 x=0, y=0, u=0, v=0;
  float z_index;

  Vector3 vert_weights;
  float shade = (vector3_dot(tri->face_normal, lightsource) + 1) / 2;

  Uint8 *pixels_start = (Uint8 *)model->materials[tri->mat_index]->pixels;
  int pitch = model->materials[tri->mat_index]->pitch;

  for (x=lx; x<=hx; x++)
  {
    for (y=ly; y<=hy-0; y+=1)
    {
      vert_weights = calculate_barycentric(x, y, v1, v2, v3);

      if (vert_weights.x >= 0 && vert_weights.y >= 0 && vert_weights.z >= 0)
      {
        z_index = v1.w*vert_weights.x + v2.w*vert_weights.y + v3.w*vert_weights.z;

        if (z_index > z_buffer[SCREEN_WIDTH*y + x])
        {
          z_buffer[SCREEN_WIDTH*y + x] = z_index;

          u = (Uint16)((vert_weights.x*_reg_uv_x[3] + vert_weights.y*_reg_uv_x[2] + vert_weights.z*_reg_uv_x[1]) / z_index);
          v = (Uint16)((vert_weights.x*_reg_uv_y[3] + vert_weights.y*_reg_uv_y[2] + vert_weights.z*_reg_uv_y[1]) / z_index);

          u *= model->materials[tri->mat_index]->format->BytesPerPixel;

          Uint8 *red = pixels_start + v*pitch + u+2;
          Uint8 *green = pixels_start + v*pitch + u+1;
          Uint8 *blue = pixels_start + v*pitch + u+0;

          float r = *red * shade;
          float g = *green * shade;
          float b = *blue * shade;

          pixel(x, y, (Uint8)r, (Uint8)g, (Uint8)b);
        }
      }
    }
  }
}

void triangle_2d(Polygon *tri)
{
  Vector2 v1 = project_coordinate(&tri->vertices[0]);
  Vector2 v2 = project_coordinate(&tri->vertices[1]);
  Vector2 v3 = project_coordinate(&tri->vertices[2]);

  // line_2d((Vector3){0, 0, 0}, (Vector2){v1.x, v1.y, 1}, (Vector2){v2.x, v2.y, 1});
  // line_2d((Vector3){0, 0, 0}, (Vector2){v2.x, v2.y, 1}, (Vector2){v3.x, v3.y, 1});
  // line_2d((Vector3){0, 0, 0}, (Vector2){v3.x, v3.y, 1}, (Vector2){v1.x, v1.y, 1});

  Vector3 tex_inv_x = (Vector3){tri->uvs[0].x * v1.w, tri->uvs[1].x * v2.w, tri->uvs[2].x * v3.w};
  Vector3 tex_inv_y = (Vector3){tri->uvs[0].y * v1.w, tri->uvs[1].y * v2.w, tri->uvs[2].y * v3.w};

  Uint16 lx = MIN(v1.x, MIN(v2.x, v3.x));
  Uint16 hx = MAX(v1.x, MAX(v2.x, v3.x));
  Uint16 ly = MIN(v1.y, MIN(v2.y, v3.y));
  Uint16 hy = MAX(v1.y, MAX(v2.y, v3.y));


  Uint16 u=0, v=0;
  float z_index;

  Vector3 vert_weights;
  Uint8 *red, *green, *blue;
  Uint8 *pixels = tri->texture->pixels;


  for (Uint16 x=lx; x<=hx; x++)
  {
    for (Uint16 y=ly; y<=hy; y++)
    {
      vert_weights = calculate_barycentric(x, y, v1, v2, v3);

      if (vert_weights.x >= 0 && vert_weights.y >= 0 && vert_weights.z >= 0)
      {
        z_index = v1.w*vert_weights.x + v2.w*vert_weights.y + v3.w*vert_weights.z;

        if (z_index > z_buffer[SCREEN_WIDTH*y + x])
        {
          z_buffer[SCREEN_WIDTH*y + x] = z_index;

          u = (Uint16)((vert_weights.x*tex_inv_x.x + vert_weights.y*tex_inv_x.y + vert_weights.z*tex_inv_x.z) / z_index) % tri->texture->w;
          v = (Uint16)((vert_weights.x*tex_inv_y.x + vert_weights.y*tex_inv_y.y + vert_weights.z*tex_inv_y.z) / z_index) % tri->texture->h;

          u *= tri->texture->format->BytesPerPixel;

          red   = pixels + v*tri->texture->pitch + u+2;
          green = pixels + v*tri->texture->pitch + u+1;
          blue  = pixels + v*tri->texture->pitch + u+0;

          pixel(x, y, *red, *green, *blue);
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

void SIMD_triangle_2d(Model *model, Polygon *tri)
{
  Vector2 v1 = project_coordinate(&tri->vertices[0]);
  Vector2 v2 = project_coordinate(&tri->vertices[1]);
  Vector2 v3 = project_coordinate(&tri->vertices[2]);

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
  __m128 _identity = _mm_set1_ps(1);
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
      SIMD_calculate_barycentric_third(&_identity, &_first, &_second, &_third);

      _v1w_by_first  = _mm_mul_ps(1/_v1w, _first);
      _v2w_by_second = _mm_mul_ps(1/_v2w, _second);
      _v3w_by_third  = _mm_mul_ps(1/_v3w, _third);

      _z_index = _mm_add_ps(_v1w_by_first, _mm_add_ps(_v2w_by_second, _v3w_by_third));


      // Check whether barycentric coordinates are all >= 0
      //------------------------------------------------------
      __m128 _cmp_first  = _mm_cmpgt_ps(_first,  _zeroes);
      __m128 _cmp_second = _mm_cmpgt_ps(_second, _zeroes);
      __m128 _cmp_third  = _mm_cmpgt_ps(_third,  _zeroes);

      Uint8 _out_first  = _mm_movemask_ps(_cmp_first);        // 0000 1111
      Uint8 _out_second = _mm_movemask_ps(_cmp_second);       // 0000 1101
      Uint8 _out_third  = _mm_movemask_ps(_cmp_third);        // 0000 1011

      Uint8 _out_all = _out_first & _out_second & _out_third; // 0000 1001

      Uint8 masks[4] = {0b00001000, 0b00000100, 0b00000010, 0b00000001};
      //------------------------------------------------------

      __m128 _z_buffer = _mm_set_ps(z_buffer[SCREEN_WIDTH*y + x], z_buffer[SCREEN_WIDTH*(y+1) + x], z_buffer[SCREEN_WIDTH*(y+2) + x], z_buffer[SCREEN_WIDTH*(y+3) + x]);
      __m128 _zindex_greaterthan_zbuffer = _mm_cmpgt_ps(_z_index, _z_buffer);
      Uint8 _out_z = _mm_movemask_ps(_zindex_greaterthan_zbuffer);

      for (int i=0; i<4; i++)
      {
        if (masks[i] & _out_all) // If in triangle
        {
          if (masks[i] & _out_z) // If z-index is greater than value in z-buffer
          {
            _z_buffer[SCREEN_WIDTH*(y+i) + x] = _z_index[i];

            u = (Uint16)((_first[3-i]*tex_inv_x.x + _second[3-i]*tex_inv_x.y + _third[3-i]*tex_inv_x.z) / z_index) % tri->texture->w;
            v = (Uint16)((_first[3-i]*tex_inv_y.x + _second[3-i]*tex_inv_y.y + _third[3-i]*tex_inv_y.z) / z_index) % tri->texture->h;

            u *= tri->texture->format->BytesPerPixel;

            red   = pixels + v*tri->texture->pitch + u+2;
            green = pixels + v*tri->texture->pitch + u+1;
            blue  = pixels + v*tri->texture->pitch + u+0;

            pixel(x, y+i, *red, *green, *blue);
          }
        }
      }

    }

    for (y=hy-remainder; y<=hy; y++)
    {
      vert_weights = calculate_barycentric(x, y, v1, v2, v3);

      if (vert_weights.x >= 0 && vert_weights.y >= 0 && vert_weights.z >= 0)
      {
        z_index = v1.w*vert_weights.x + v2.w*vert_weights.y + v3.w*vert_weights.z;

        if (z_index > z_buffer[SCREEN_WIDTH*y + x])
        {
          u = (Uint16)((vert_weights.x*tex_inv_x.x + vert_weights.y*tex_inv_x.y + vert_weights.z*tex_inv_x.z) / z_index) % model->materials[tri->mat_index]->w;
          v = (Uint16)((vert_weights.x*tex_inv_y.x + vert_weights.y*tex_inv_y.y + vert_weights.z*tex_inv_y.z) / z_index) % model->materials[tri->mat_index]->h;

          u *= model->materials[tri->mat_index]->format->BytesPerPixel;

          red   = pixels + v*model->materials[tri->mat_index]->pitch + u+2;
          green = pixels + v*model->materials[tri->mat_index]->pitch + u+1;
          blue  = pixels + v*model->materials[tri->mat_index]->pitch + u+0;

          pixel(x, y, *red, *green, *blue);
          // pixel(x, y, 0, 0, 0);
        }
      }
    }
  }
}


/**
 * @param p1 start of line
 * @param p2 end of line
 */
Vector3 line_plane_intersect(Vector3 plane_normal, Vector3 p1, Vector3 p2, float *t)
{
  float ad = vector3_dot(p1, plane_normal);
  float bd = vector3_dot(p2, plane_normal);
  *t = (-ad) / (bd - ad);
  // printf("t: %f\n", *t);
  Vector3 lste = vector3_sub(p2, p1);
  Vector3 lti = vector3_scale(lste, *t);
  return vector3_add(p1, lti);
}

/** Return the number of points with positive signed distance to a plane normal
 * @param index_of_inside index position of point that has +ve distance (if one point is +ve)
 * @param index_of_outside index position of point that has -ve distance (if one point is -ve)
 */
int points_inside_plane(Vector3 plane_normal, Polygon *tri, int *index_of_inside, int *index_of_outside)
{
  float dot1 = vector3_dot(plane_normal, tri->vertices[0]);
  float dot2 = vector3_dot(plane_normal, tri->vertices[1]);
  float dot3 = vector3_dot(plane_normal, tri->vertices[2]);
  int number_of_inside = 0;
  
  if (dot1 > 0) number_of_inside += 1;
  if (dot2 > 0) number_of_inside += 1;
  if (dot3 > 0) number_of_inside += 1;

  // printf("d0: %f, d1: %f, d2: %f, count: %d\n", dot1, dot2, dot3, number_of_inside);

  if (dot1 > dot2 && dot1 > dot3) *index_of_inside = 0;
  else if (dot2 > dot1 && dot2 > dot3) *index_of_inside = 1;
  else if (dot3 > dot1 && dot3 > dot2) *index_of_inside = 2;
  if (dot1 < dot2 && dot1 < dot3) *index_of_outside = 0;
  else if (dot2 < dot1 && dot2 < dot3) *index_of_outside = 1;
  else if (dot3 < dot1 && dot3 < dot2) *index_of_outside = 2;

  return number_of_inside;
}

/** Clip triangles to a plane
 * @param tri_in input triangle
 * @param tri_out1 first possible output triangle
 * @param tri_out2 second possible output triangle
 * @return number of triangles formed due to clipping
 */
int clip_polygon(Vector3 plane_normal, Polygon *tri_in, Polygon *tri_out1, Polygon *tri_out2)
{
  int index_of_inside = 0;
  int index_of_outside = 0;

  Vector3 A, B, C;
  Vector3 A_prime, B_prime, C_prime;

  float t1, t2;

  // number of points inside of plane
  int n = points_inside_plane(plane_normal, tri_in, &index_of_inside, &index_of_outside);
  // printf("n: %d\n", n);
  Vector2 tex_Aprime, tex_Bprime, tex_Cprime;

  switch (n)
  {
    case (0): return 0; // all outside
    case (3): return 1; // all inside

    case (1): // two outside, one inside
      switch (index_of_inside)
      {
        case (0):
          A = tri_in->vertices[0];
          B = tri_in->vertices[1];
          C = tri_in->vertices[2];

          B_prime = line_plane_intersect(plane_normal, A, B, &t1);
          C_prime = line_plane_intersect(plane_normal, A, C, &t2);
          tex_Bprime = (Vector2){ tri_in->uvs[0].x + t1*(tri_in->uvs[1].x - tri_in->uvs[0].x),
                                  tri_in->uvs[0].y + t1*(tri_in->uvs[1].y - tri_in->uvs[0].y),
                                  1 };
          tex_Cprime = (Vector2){ tri_in->uvs[0].x + t2*(tri_in->uvs[2].x - tri_in->uvs[0].x),
                                  tri_in->uvs[0].y + t2*(tri_in->uvs[2].y - tri_in->uvs[0].y),
                                  1 };

          tri_in->uvs[1] = tex_Bprime;
          tri_in->uvs[2] = tex_Cprime;
          tri_in->vertices[0] = A;
          tri_in->vertices[1] = B_prime;
          tri_in->vertices[2] = C_prime;
          break;

        case (1):
          A = tri_in->vertices[1];
          B = tri_in->vertices[0];
          C = tri_in->vertices[2];

          B_prime = line_plane_intersect(plane_normal, A, B, &t1);
          C_prime = line_plane_intersect(plane_normal, A, C, &t2);
          tex_Bprime = (Vector2){ tri_in->uvs[1].x + t1*(tri_in->uvs[0].x - tri_in->uvs[1].x),
                                  tri_in->uvs[1].y + t1*(tri_in->uvs[0].y - tri_in->uvs[1].y),
                                  1 };
          tex_Cprime = (Vector2){ tri_in->uvs[1].x + t2*(tri_in->uvs[2].x - tri_in->uvs[1].x),
                                  tri_in->uvs[1].y + t2*(tri_in->uvs[2].y - tri_in->uvs[1].y),
                                  1 };

          tri_in->uvs[2] = tex_Bprime;
          tri_in->uvs[0] = tex_Cprime;
          tri_in->vertices[1] = A;
          tri_in->vertices[2] = B_prime;
          tri_in->vertices[0] = C_prime;
          break;

        case (2):
          A = tri_in->vertices[2];
          B = tri_in->vertices[0];
          C = tri_in->vertices[1];

          B_prime = line_plane_intersect(plane_normal, A, B, &t1);
          C_prime = line_plane_intersect(plane_normal, A, C, &t2);
          tex_Bprime = (Vector2){ tri_in->uvs[2].x + t1*(tri_in->uvs[0].x - tri_in->uvs[2].x),
                                  tri_in->uvs[2].y + t1*(tri_in->uvs[0].y - tri_in->uvs[2].y),
                                  1 };
          tex_Cprime = (Vector2){ tri_in->uvs[2].x + t2*(tri_in->uvs[1].x - tri_in->uvs[2].x),
                                  tri_in->uvs[2].y + t2*(tri_in->uvs[1].y - tri_in->uvs[2].y),
                                  1 };

          tri_in->uvs[0] = tex_Bprime;
          tri_in->uvs[1] = tex_Cprime;

          tri_in->vertices[2] = A;
          tri_in->vertices[0] = B_prime;
          tri_in->vertices[1] = C_prime;
          break;
      }

      return 1;


    case (2): // one outside, two inside
      switch (index_of_outside)
      {
        case (0):
          C = tri_in->vertices[0];
          A = tri_in->vertices[1];
          B = tri_in->vertices[2];

          A_prime = line_plane_intersect(plane_normal, A, C, &t1);
          B_prime = line_plane_intersect(plane_normal, B, C, &t2);
          
          tex_Aprime = (Vector2){  tri_in->uvs[1].x + t1*(tri_in->uvs[0].x - tri_in->uvs[1].x),
                                  tri_in->uvs[1].y + t1*(tri_in->uvs[0].y - tri_in->uvs[1].y),
                                  1 };

          tex_Bprime = (Vector2){  tri_in->uvs[2].x + t2*(tri_in->uvs[0].x - tri_in->uvs[2].x),
                                  tri_in->uvs[2].y + t2*(tri_in->uvs[0].y - tri_in->uvs[2].y),
                                  1 };

          tri_out1->uvs[0] = tri_in->uvs[1];
          tri_out1->uvs[1] = tri_in->uvs[2];
          tri_out1->uvs[2] = tex_Aprime;

          tri_out2->uvs[0] = tex_Aprime;
          tri_out2->uvs[1] = tri_in->uvs[2];
          tri_out2->uvs[2] = tex_Bprime;

          break;

        case (1):
          C = tri_in->vertices[1];
          A = tri_in->vertices[0];
          B = tri_in->vertices[2];

          A_prime = line_plane_intersect(plane_normal, A, C, &t1);
          B_prime = line_plane_intersect(plane_normal, B, C, &t2);
          
          tex_Aprime = (Vector2){  tri_in->uvs[0].x + t1*(tri_in->uvs[1].x - tri_in->uvs[0].x),
                                  tri_in->uvs[0].y + t1*(tri_in->uvs[1].y - tri_in->uvs[0].y),
                                  1 };

          tex_Bprime = (Vector2){  tri_in->uvs[2].x + t2*(tri_in->uvs[1].x - tri_in->uvs[2].x),
                                  tri_in->uvs[2].y + t2*(tri_in->uvs[1].y - tri_in->uvs[2].y),
                                  1 };

          tri_out1->uvs[0] = tri_in->uvs[0];
          tri_out1->uvs[1] = tri_in->uvs[2];
          tri_out1->uvs[2] = tex_Aprime;

          tri_out2->uvs[0] = tex_Aprime;
          tri_out2->uvs[1] = tri_in->uvs[2];
          tri_out2->uvs[2] = tex_Bprime;
          break;


        case (2):
          C = tri_in->vertices[2];
          A = tri_in->vertices[0];
          B = tri_in->vertices[1];

          A_prime = line_plane_intersect(plane_normal, A, C, &t1);
          B_prime = line_plane_intersect(plane_normal, B, C, &t2);

          tex_Aprime = (Vector2){  tri_in->uvs[0].x + t1*(tri_in->uvs[2].x - tri_in->uvs[0].x),
                                  tri_in->uvs[0].y + t1*(tri_in->uvs[2].y - tri_in->uvs[0].y),
                                  1 };

          tex_Bprime = (Vector2){  tri_in->uvs[1].x + t2*(tri_in->uvs[2].x - tri_in->uvs[1].x),
                                  tri_in->uvs[1].y + t2*(tri_in->uvs[2].y - tri_in->uvs[1].y),
                                  1 };

          tri_out1->uvs[0] = tri_in->uvs[0];
          tri_out1->uvs[1] = tri_in->uvs[1];
          tri_out1->uvs[2] = tex_Aprime;

          tri_out2->uvs[0] = tex_Aprime;
          tri_out2->uvs[1] = tri_in->uvs[1];
          tri_out2->uvs[2] = tex_Bprime;
          
          break;
      }

      // u' = u1 + t(u2 - u1);

      tri_out1->vertices[0] = A;
      tri_out1->vertices[1] = B;
      tri_out1->vertices[2] = A_prime;

      tri_out2->vertices[0] = A_prime;
      tri_out2->vertices[1] = B;
      tri_out2->vertices[2] = B_prime;
      return 2;
  }

  return -1;
}

/** clip a polygon against a plane
 * 
 * @param tris array of polygons to clip. altered by function
 * @param clipped_triangles array of clipped polygons
 * @return number of polygons formed due to clipping
 */
int clip_against_plane(Vector3 plane_normal, int poly_count, Polygon *unclipped_triangles, Polygon *clipped_triangles)
{
  int unclipped_index = 0;
  int clipped_index = 0;

  while (unclipped_index < poly_count)  
  {
    Polygon tri1 = unclipped_triangles[unclipped_index];
    Polygon tri2 = unclipped_triangles[unclipped_index];

    // n == number of triangles formed due to clipping
    int n = clip_polygon(plane_normal, &unclipped_triangles[unclipped_index], &tri1, &tri2);
    switch (n)
    {
      case (0):
        break;

      case (1):
        clipped_triangles[clipped_index] = unclipped_triangles[unclipped_index];
        clipped_index += 1;
        break;
      
      case (2):
        clipped_triangles[clipped_index] = tri1;
        clipped_triangles[clipped_index+1] = tri2;
        clipped_index += 2;
        break;
    }
    unclipped_index += 1;
  }
  
  return clipped_index;
}

Polygon *clip_against_planes(Camera *cam, int in_size, Polygon *polygons_in, int *out_size)
{
  Polygon *clipped_1 = (Polygon *)malloc(in_size*2 * sizeof(Polygon));
  int n = clip_against_plane(cam->l_norm, in_size, polygons_in, clipped_1);

  Polygon *clipped_2 = (Polygon *)malloc(n*2 * sizeof(Polygon));
  n = clip_against_plane(cam->r_norm, n, clipped_1, clipped_2);

  Polygon *clipped_3 = (Polygon *)malloc(n*2 * sizeof(Polygon));
  n = clip_against_plane(cam->t_norm, n, clipped_2, clipped_3);
  
  Polygon *clipped_4 = (Polygon *)malloc(n*2 * sizeof(Polygon));
  *out_size = clip_against_plane(cam->b_norm, n, clipped_3, clipped_4);

  free(clipped_1);
  free(clipped_2);
  free(clipped_3);

  return clipped_4;
}

/** Enque a model's polygons in the GE_transform_queue
 */
void GE_model_enque(Model *model)
{

  // Only queue front faces
  for (int i=0; i<model->poly_count; i++)
    if (vector3_dot(vector3_sub(model->polygons[i].vertices[0], *GE_cam->pos), model->polygons[i].face_normal) < 0)
      RSR_enque(GE_transform_queue, &model->polygons[0]);


  // for (int i=0; i<frontface_count; i++)
  // {
  //   for (int j=0; j<3; j++)
  //   {
  //     front_faces[i].vertices[j].x -= cam->pos->x;
  //     front_faces[i].vertices[j].y -= cam->pos->y;
  //     front_faces[i].vertices[j].z -= cam->pos->z;

  //     front_faces[i].og_vertices[j].x -= cam->pos->x;
  //     front_faces[i].og_vertices[j].y -= cam->pos->y;
  //     front_faces[i].og_vertices[j].z -= cam->pos->z;

  //     rotate_point(&front_faces[i].vertices[j], 0, cam->rot.y, 0);
  //     rotate_point(&front_faces[i].vertices[j], cam->rot.x, 0, 0);

  //     rotate_point(&front_faces[i].og_vertices[j], 0, cam->rot.y, 0);
  //     rotate_point(&front_faces[i].og_vertices[j], cam->rot.x, 0, 0);
  //   }
  // }

  // int clipped_count;
  // Polygon *clipped_polygons = clip_against_planes(cam, frontface_count, front_faces, &clipped_count);


  // free(frontface_indices);
  // free(front_faces);
  // free(clipped_polygons);
}
//-------------------------------------------------------------------------------


//             copy
// input --> rotation --> clipping --> rasterisation

/** Rotate all polygons in GE_transform_queue by -cam.rot and move them to GE_clip_queue.
 *  GE_transform_queue will be emptied.
 */
void GE_queue_rotate(void)
{
  int size = GE_transform_queue->size;

  free(front_faces);
  front_faces = (Polygon *)malloc(size * sizeof(Polygon));

  RSR_dequeue(GE_transform_queue);

  for (int i=0; i<size; i++)
  {
    front_faces[i] = *RSR_front(GE_transform_queue);
    printf("%f\n", front_faces[i].vertices[0].x);
    RSR_dequeue(GE_transform_queue);
  }

  for (int i=0; i<size; i++)
  {
    for (int j=0; j<3; j++)
    {
      front_faces[i].vertices[j].x -= GE_cam->pos->x;
      front_faces[i].vertices[j].y -= GE_cam->pos->y;
      front_faces[i].vertices[j].z -= GE_cam->pos->z;

      front_faces[i].og_vertices[j].x -= GE_cam->pos->x;
      front_faces[i].og_vertices[j].y -= GE_cam->pos->y;
      front_faces[i].og_vertices[j].z -= GE_cam->pos->z;

      rotate_point(&front_faces[i].vertices[j], 0, GE_cam->rot.y, 0);
      rotate_point(&front_faces[i].vertices[j], GE_cam->rot.x, 0, 0);

      rotate_point(&front_faces[i].og_vertices[j], 0, GE_cam->rot.y, 0);
      rotate_point(&front_faces[i].og_vertices[j], GE_cam->rot.x, 0, 0);
    }
  }

  // for (int i=0; i<size; i++)
  //   RSR_enque(GE_rasterise_queue, &front_faces[i]);

  int clipped_count;
  Polygon *clipped_polygons = clip_against_planes(GE_cam, size, front_faces, &clipped_count);

  for (int i=0; i<clipped_count; i++)
  {
    triangle_2d(&clipped_polygons[i]);
    // triangle_2d(RSR_front(GE_rasterise_queue));
    // RSR_dequeue(GE_rasterise_queue);
  }

}

/** Clip all polygons in GE_clip_queue and move them to GE_rasterise_queue
 *  GE_clip_queue will be emptied.
 */
void GE_queue_clip(void)
{

}

/** Rasterise all polygons in GE_rasterise_queue
 *  GE_rasterise_queue will be emptied.
 */
void GE_queue_rasterise(void)
{
  for (int i=0; i<GE_rasterise_queue->size; i++)
  {
    triangle_2d(RSR_front(GE_rasterise_queue));
    RSR_dequeue(GE_rasterise_queue);
  }

}





















/** Project a 3D world coordinate onto a 2D screen coordinate.
 * z coordinate is preserved for z-buffering.
 */
Vector2 project_coordinate(Vector3 *pt)
{
  float nearplane_width = HALF_SCREEN_WIDTH;
  float nearplane_height = HALF_SCREEN_HEIGHT;
  float nearplane_z = 1;

  float canvas_x = (nearplane_z/pt->z) * pt->x * nearplane_z * nearplane_width;
  float canvas_y = (nearplane_z/pt->z) * pt->y * nearplane_z * nearplane_height;
  canvas_x += HALF_SCREEN_WIDTH - 0.5;
  canvas_y += HALF_SCREEN_HEIGHT - 0.5;

  return (Vector2){canvas_x, canvas_y, 1/pt->z};
}

