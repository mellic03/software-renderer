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


#include "engine.h"
#include "camera.h"
#include "../math/vector.h"
#include "../screen.h"

SDL_Surface *pixel_array;
float z_buffer[SCREEN_WIDTH * SCREEN_HEIGHT];

Vector3 lightsource = {0, 0, -50};

double delta_time;

pthread_t thread1;
pthread_t thread2;


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

void rotate_z(Model model, float r)
{
  Vector3 model_pos = model.pos;
  translate_model(&model, -model.pos.x, -model.pos.y, -model.pos.z);

  float rot_z[3][3] = {
    { cos(r), -sin(r), 0 },
    { sin(r), cos(r),  0 },
    { 0,      0,       1 }
  };


  float result[3][1];

  for (int i=0; i<model.poly_count; i++)
  {
    // rotate vertices
    for (int j=0; j<3; j++)
    {
      float coord[3][1] = {{model.polygons[i].vertices[j].x}, {model.polygons[i].vertices[j].y}, {model.polygons[i].vertices[j].z}};
      matrix_mult(3, 3, 3, 1, result, rot_z, coord);
      model.polygons[i].vertices[j].x = result[0][0];
      model.polygons[i].vertices[j].y = result[1][0];
      model.polygons[i].vertices[j].z = result[2][0];
    }

    // rotate normals
    float coord[3][1] = {{model.polygons[i].face_normal.x}, {model.polygons[i].face_normal.y}, {model.polygons[i].face_normal.z}};
    matrix_mult(3, 3, 3, 1, result, rot_z, coord);
    model.polygons[i].face_normal.x = result[0][0];
    model.polygons[i].face_normal.y = result[1][0];
    model.polygons[i].face_normal.z = result[2][0];
  }
  translate_model(&model, model_pos.x, model_pos.y, model_pos.z);
}

void scale(Model *model, float alpha)
{
  for (int i=0; i<model->poly_count; i++)
    for (int j=0; j<3; j++)
      model->polygons[i].vertices[j] = vector3_scale(model->polygons[i].vertices[j], alpha);
}

//-------------------------------------------------------------------------------

// DRAWING
//-------------------------------------------------------------------------------
void clear_screen(Uint8 r, Uint8 g, Uint8 b)
{
  for (int i=0; i<SCREEN_WIDTH; i++)
    for (int j=0; j<SCREEN_HEIGHT; j++)
      pixel(i, j, r, g, b);

  for (int i=0; i<SCREEN_WIDTH; i++)
    for (int j=0; j<SCREEN_HEIGHT; j++)
      z_buffer[SCREEN_WIDTH*j + i] = 0;
}

void pixel(int x, int y, Uint8 r, Uint8 g, Uint8 b)
{
  Uint8 * const blue  = ((Uint8 *) pixel_array->pixels + (y*4*SCREEN_WIDTH) + (x*4 + 0));
  *blue = b;
  Uint8 * const green = ((Uint8 *) pixel_array->pixels + (y*4*SCREEN_WIDTH) + (x*4 + 1));
  *green = g;
  Uint8 * const red   = ((Uint8 *) pixel_array->pixels + (y*4*SCREEN_WIDTH) + (x*4 + 2));
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

void triangle_2d(Model *model, Polygon tri, SDL_Surface **textures, int texture_index)
{
  Vector2 v1 = project_coordinate(&tri.vertices[0]);
  Vector2 v2 = project_coordinate(&tri.vertices[1]);
  Vector2 v3 = project_coordinate(&tri.vertices[2]);

  Vector2 ov1 = project_coordinate(&tri.og_vertices[0]);
  Vector2 ov2 = project_coordinate(&tri.og_vertices[1]);
  Vector2 ov3 = project_coordinate(&tri.og_vertices[2]);

  Vector3 norm1 = model->vertex_normals[tri.vertex_indices[0]];
  Vector3 norm2 = model->vertex_normals[tri.vertex_indices[1]];
  Vector3 norm3 = model->vertex_normals[tri.vertex_indices[2]];

  Vector3 dir1 = vector3_sub(lightsource, model->vertex_normals[tri.vertex_indices[0]]);
  Vector3 dir2 = vector3_sub(lightsource, model->vertex_normals[tri.vertex_indices[1]]);
  Vector3 dir3 = vector3_sub(lightsource, model->vertex_normals[tri.vertex_indices[2]]);

  float dist1 = vector3_dist(tri.og_vertices[0], model->lightsource);
  float dist2 = vector3_dist(tri.og_vertices[1], model->lightsource);
  float dist3 = vector3_dist(tri.og_vertices[2], model->lightsource);
  

  __m128 _reg_uv_x = _mm_set_ps(tri.uvs[0].x, tri.uvs[1].x, tri.uvs[2].x, 1);
  __m128 _reg_uv_y = _mm_set_ps(tri.uvs[0].y, tri.uvs[1].y, tri.uvs[2].y, 1);
  __m128 _reg_invz = _mm_set_ps(v1.w,         v2.w,         v3.w,         1);

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
  // vector3_normalise(&templight);

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
          int tex_indx = texture_index;
          if (z_index > 35)
            tex_indx += model->mat_count;

          z_buffer[SCREEN_WIDTH*y + x] = z_index;

          light_weights = calculate_barycentric(x, y, ov1, ov2, ov3);

          Vector3 dd = (Vector3){
            dir1.x*light_weights.x + dir2.x*light_weights.y + dir3.x*light_weights.z,
            dir1.y*light_weights.x + dir2.y*light_weights.y + dir3.y*light_weights.z,
            dir1.z*light_weights.x + dir2.z*light_weights.y + dir3.z*light_weights.z
          };
          vector3_normalise(&dd);

          Vector3 nn = (Vector3){
            norm1.x*light_weights.x + norm2.x*light_weights.y + norm3.x*light_weights.z,
            norm1.y*light_weights.x + norm2.y*light_weights.y + norm3.y*light_weights.z,
            norm1.z*light_weights.x + norm2.z*light_weights.y + norm3.z*light_weights.z
          };
          vector3_normalise(&nn);

          float shade = vector3_dot(dd, nn);
          shade += 1;
          shade /= 2;

          u = (Uint16)((vert_weights.x*_reg_uv_x[3] + vert_weights.y*_reg_uv_x[2] + vert_weights.z*_reg_uv_x[1]) / z_index) % textures[tex_indx]->w;
          v = (Uint16)((vert_weights.x*_reg_uv_y[3] + vert_weights.y*_reg_uv_y[2] + vert_weights.z*_reg_uv_y[1]) / z_index) % textures[tex_indx]->h;

          u *= textures[tex_indx]->format->BytesPerPixel;

          Uint8 *red = (Uint8 *)textures[tex_indx]->pixels + v*textures[tex_indx]->pitch + u+2;
          Uint8 *green = (Uint8 *)textures[tex_indx]->pixels + v*textures[tex_indx]->pitch + u+1;
          Uint8 *blue = (Uint8 *)textures[tex_indx]->pixels + v*textures[tex_indx]->pitch + u+0;

          float r = *red;
          float g = *green;
          float b = *blue;

          r *= shade;
          g *= shade;
          b *= shade;

          pixel(x, y, (Uint8)r, (Uint8)g, (Uint8)b);
        }
      }
    }
  }
}

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
  Polygon *clipped_1 = (Polygon *)calloc(in_size*2, sizeof(Polygon));
  int n = clip_against_plane(cam->l_norm, in_size, polygons_in, clipped_1);

  Polygon *clipped_2 = (Polygon *)calloc(n*2, sizeof(Polygon));
  n = clip_against_plane(cam->r_norm, n, clipped_1, clipped_2);

  Polygon *clipped_3 = (Polygon *)calloc(n*2, sizeof(Polygon));
  n = clip_against_plane(cam->t_norm, n, clipped_2, clipped_3);
  
  Polygon *clipped_4 = (Polygon *)calloc(n*2, sizeof(Polygon));
  *out_size = clip_against_plane(cam->b_norm, n, clipped_3, clipped_4);

  free(clipped_1);
  free(clipped_2);
  free(clipped_3);

  return clipped_4;
}

struct wrapper {
  Camera *cam;
  int poly_count;
  Polygon *polygons;
  SDL_Surface **textures;
  Model *model;
  int start, stop;
};

void *render_polygons_pthread(void *ptr)
{
  struct wrapper *w1 = (struct wrapper *)ptr;
  for (int i=w1->start; i<w1->stop; i++)
    triangle_2d(w1->model, w1->polygons[i], w1->textures, w1->polygons[i].mat_index);

  pthread_exit(NULL);
}

pthread_t thread1, thread2, thread3, thread4;
struct wrapper wrap1;
struct wrapper wrap2;
struct wrapper wrap3;
struct wrapper wrap4;

Model modelcpy(Model src)
{
  Model output;
  for (int i=0; i<src.poly_count; i++)
    output.polygons[i] = src.polygons[i];
}

void draw_model(Camera cam, Model *model)
{
  int *frontface_indices = (int *)calloc(model->poly_count, sizeof(int));
  int frontface_count = 0;

  model->lightsource = lightsource;
  model->lightsource.x -= cam.pos.x;
  model->lightsource.y -= cam.pos.y;
  model->lightsource.z -= cam.pos.z;
  rotate_point(&model->lightsource, 0, cam.rot.y, 0);
  rotate_point(&model->lightsource, cam.rot.x, 0, 0);
  
  for (int i=0; i<model->poly_count; i++)
    if (vector3_dot(vector3_sub(model->polygons[i].vertices[0], cam.pos), model->polygons[i].face_normal) < 0)
      frontface_indices[frontface_count++] = i;
  
  Polygon *front_faces = (Polygon *)calloc(frontface_count, sizeof(Polygon));
  for (int i=0; i<frontface_count; i++)
    memcpy(&front_faces[i], &model->polygons[frontface_indices[i]], sizeof(Polygon));

  for (int i=0; i<frontface_count; i++)
  {
    for (int j=0; j<3; j++)
    {
      front_faces[i].vertices[j].x -= cam.pos.x;
      front_faces[i].vertices[j].y -= cam.pos.y;
      front_faces[i].vertices[j].z -= cam.pos.z;

      front_faces[i].og_vertices[j].x -= cam.pos.x;
      front_faces[i].og_vertices[j].y -= cam.pos.y;
      front_faces[i].og_vertices[j].z -= cam.pos.z;

      rotate_point(&front_faces[i].vertices[j], 0, cam.rot.y, 0);
      rotate_point(&front_faces[i].vertices[j], cam.rot.x, 0, 0);

      rotate_point(&front_faces[i].og_vertices[j], 0, cam.rot.y, 0);
      rotate_point(&front_faces[i].og_vertices[j], cam.rot.x, 0, 0);

      rotate_point(&front_faces[i].normals[j], 0, cam.rot.y, 0);
      rotate_point(&front_faces[i].normals[j], cam.rot.x, 0, 0);
    }
  }

  int clipped_count;
  Polygon *clipped_polygons = clip_against_planes(&cam, frontface_count, front_faces, &clipped_count);

  // Multithreading
  //------------------------------------------------------------------
  wrap1.cam = (Camera *)malloc(sizeof(Camera));
  wrap1.poly_count = clipped_count;
  wrap1.polygons = (Polygon *)malloc(clipped_count * sizeof(Polygon));
  wrap1.textures = model->materials;
  wrap1.start = 0;
  wrap1.stop = clipped_count/4;
  wrap1.model = model;
  memcpy(wrap1.cam, &cam, sizeof(Camera));
  memcpy(wrap1.polygons, clipped_polygons, clipped_count * sizeof(Polygon));

  wrap2.cam = (Camera *)malloc(sizeof(Camera));
  wrap2.poly_count = clipped_count;
  wrap2.polygons = (Polygon *)malloc(clipped_count * sizeof(Polygon));
  wrap2.textures = model->materials;
  wrap2.start = clipped_count/4;
  wrap2.stop = clipped_count/2;
  wrap2.model = model;
  memcpy(wrap2.cam, &cam, sizeof(Camera));
  memcpy(wrap2.polygons, clipped_polygons, clipped_count * sizeof(Polygon));

  wrap3.cam = (Camera *)malloc(sizeof(Camera));
  wrap3.poly_count = clipped_count;
  wrap3.polygons = (Polygon *)malloc(clipped_count * sizeof(Polygon));
  wrap3.textures = model->materials;
  wrap3.start = clipped_count/2;
  wrap3.stop = 3*clipped_count/4;
  wrap3.model = model;
  memcpy(wrap3.cam, &cam, sizeof(Camera));
  memcpy(wrap3.polygons, clipped_polygons, clipped_count * sizeof(Polygon));

  wrap4.cam = (Camera *)malloc(sizeof(Camera));
  wrap4.poly_count = clipped_count;
  wrap4.polygons = (Polygon *)malloc(clipped_count * sizeof(Polygon));
  wrap4.textures = model->materials;
  wrap4.start = 3*clipped_count/4;
  wrap4.stop = clipped_count;
  wrap4.model = model;
  memcpy(wrap4.cam, &cam, sizeof(Camera));
  memcpy(wrap4.polygons, clipped_polygons, clipped_count * sizeof(Polygon));

  pthread_create(&thread1, NULL, render_polygons_pthread, &wrap1);
  pthread_create(&thread2, NULL, render_polygons_pthread, &wrap2);
  pthread_create(&thread3, NULL, render_polygons_pthread, &wrap3);
  pthread_create(&thread4, NULL, render_polygons_pthread, &wrap4);

  pthread_join(thread1, NULL);
  pthread_join(thread2, NULL);
  pthread_join(thread3, NULL);
  pthread_join(thread4, NULL);

  free(wrap1.polygons);
  free(wrap2.polygons);
  free(wrap3.polygons);
  free(wrap4.polygons);
  //------------------------------------------------------------------

  // for (int i=0; i<clipped_count; i++)
  //   triangle_2d(&cam, clipped_polygons[i], model->materials[clipped_polygons[i].mat_index]);

  free(frontface_indices);
  free(front_faces);
  free(clipped_polygons);
}
//-------------------------------------------------------------------------------

/** Project a 3D world coordinate onto a 2D screen coordinate.
 * z coordinate is preserved for z-buffering.
 */
Vector2 project_coordinate(Vector3 *pt)
{
  float nearplane_width = HALF_SCREEN_WIDTH;
  float nearplane_height = HALF_SCREEN_HEIGHT;
  float nearplane_z = 0.999;

  float canvas_x = (nearplane_z/pt->z) * pt->x * nearplane_z * nearplane_width;
  float canvas_y = (nearplane_z/pt->z) * pt->y * nearplane_z * nearplane_height;
  canvas_x += HALF_SCREEN_WIDTH;
  canvas_y += HALF_SCREEN_HEIGHT;

  return (Vector2){canvas_x, canvas_y, 1/pt->z};
}

// FILE I/O
//-------------------------------------------------------------------------------
void count_polygons(FILE *fh, Model *model)
{
  model->poly_count = 0;
  model->vertex_count = 0;
  model->normal_count = 0;
  model->uv_count = 0;
  model->mat_count = 0;

  char buffer[64];
  while (fgets(buffer, 64, fh) != NULL)
  {
    if (buffer[0] == 'f' && buffer[1] == ' ')
      model->poly_count += 1;
    else if (buffer[0] == 'v' && buffer[1] == ' ')
      model->vertex_count += 1;
    else if (buffer[0] == 'v' && buffer[1] == 'n')
      model->normal_count += 1;
    else if (buffer[0] == 'v' && buffer[1] == 't')
      model->uv_count += 1;
    else if (buffer[0] == 'u' && buffer[1] == 's')
      model->mat_count += 1;
  }
  rewind(fh);
}

void extract_vert_text_norm(int dest[3], char *src)
{
  for (size_t i=0; i<strlen(src)-1; i++)
    if (src[i] == '/')
      src[i] == ' ';

  sscanf(src, "%d/%d/%d", &dest[0], &dest[1], &dest[2]);
  // printf("%d, %d, %d\n", vertex, texture, normal);
}

void load_polygons(FILE *fh, Model *model, Polygon *polygons)
{
  char buffer[64];

  char space[] = " ";
  char slash[] = "/";
  char *token;

  Vector3 *vertices = (Vector3 *)calloc(model->vertex_count, sizeof(Vector3));
  int vertex_index = 0;

  Vector3 *normals = (Vector3 *)calloc(model->normal_count, sizeof(Vector3));
  int normal_index = 0;

  Vector2 *tex_coords = (Vector2 *)calloc(model->uv_count, sizeof(Vector2));
  int tex_coord_index = 0;

  char **mat_names = (char **)malloc(model->mat_count * sizeof(char *));
  for (int i=0; i<model->mat_count; i++)
    mat_names[i] = (char *)malloc(64 * sizeof(char)); // max 64 chars for filename
  int mat_index = 0;

  // load all vertices and normals into memory first
  while (fgets(buffer, 64, fh) != NULL)
  {
    // Line with vertex
    if (buffer[0] == 'v' && buffer[1] == ' ')
    {
      token = strtok(buffer, space);
      float temp[3];
      for (int i=0; i<3; i++)
      {
        token = strtok(NULL, space);
        temp[i] = atof(token);
      }
      vertices[vertex_index].x = temp[0];
      vertices[vertex_index].y = temp[1];
      vertices[vertex_index].z = temp[2];
      vertex_index++;
    }

    // Line with vertex normal
    else if (buffer[0] == 'v' && buffer[1] == 'n')
    {
      token = strtok(buffer, space);
      float temp[3];
      for (int i=0; i<3; i++)
      {
        token = strtok(NULL, space);
        temp[i] = atof(token);
      }
      normals[normal_index++] = (Vector3){temp[0], temp[1], temp[2]};
    }

    // Line with texture coordinate
    else if (buffer[0] == 'v' && buffer[1] == 't')
    {
      token = strtok(buffer, space);
      token = strtok(NULL, space);
      tex_coords[tex_coord_index].x = atof(token);
      token = strtok(NULL, space);
      tex_coords[tex_coord_index].y = atof(token);
      tex_coord_index += 1;
    }

    // line with new material
    else if (buffer[0] == 'u' && buffer[1] == 's')
    {
      token = strtok(buffer, space);
      token = strtok(buffer, space);
      strcpy(mat_names[mat_index], token);
      mat_index += 1;
    }
  }
  rewind(fh);

  // Create polygons
  int polygon_index = 0;
  mat_index = -1; // usemtl always before vertices
  while (fgets(buffer, 64, fh) != NULL)
  {
    if (buffer[0] == 'f' && buffer[1] == ' ')
    {
      token = strtok(buffer, space); // token == "f"
      int temp[3];

      for (int i=0; i<3; i++)
      {
        token = strtok(NULL, space); // token == "xxx/xxx/xxx"
        extract_vert_text_norm(temp, token);
        polygons[polygon_index].vertices[i] = vertices[temp[0]-1];
        polygons[polygon_index].og_vertices[i] = vertices[temp[0]-1];
        polygons[polygon_index].vertex_indices[i] = temp[0]-1;
        polygons[polygon_index].uvs[i] = tex_coords[temp[1]-1];
      }
      polygons[polygon_index].face_normal = normals[temp[2]-1];
      polygons[polygon_index].mat_index = mat_index;
      polygon_index += 1;
    }

    // line with new material
    else if (buffer[0] == 'u' && buffer[1] == 's')
      mat_index += 1;
  }

  // Calculate vertex normals from face normals
  rewind(fh);
  // An array of normals where index n corresponds to vertex n
  Vector3 *vertex_normals = (Vector3 *)calloc(model->vertex_count, sizeof(Vector3));

  // For each polygon, add face normal to vertex_normals[index of vertex]
  for (int i=0; i<model->poly_count; i++)
    for (int j=0; j<3; j++)
      vertex_normals[polygons[i].vertex_indices[j]] = vector3_add(vertex_normals[polygons[i].vertex_indices[j]], polygons[i].face_normal);

  for (int i=0; i<model->vertex_count; i++)
    vector3_normalise(&vertex_normals[i]);

  model->vertex_normals = (Vector3 *)malloc(model->vertex_count * sizeof(Vector3));
  for (int i=0; i<model->vertex_count; i++)
    model->vertex_normals[i] = vertex_normals[i];

  for (int i=0; i<model->vertex_count; i++)
    for (int j=0; j<3; j++)
    {
      model->polygons[i].normals[j].x = model->vertex_normals[model->polygons[i].vertex_indices[j]].x;
      model->polygons[i].normals[j].y = model->vertex_normals[model->polygons[i].vertex_indices[j]].y;
      model->polygons[i].normals[j].z = model->vertex_normals[model->polygons[i].vertex_indices[j]].z;
    }

  free(vertex_normals);
  free(vertices);
  free(normals);
  free(tex_coords);
  free(mat_names);
}

void load_material(FILE *fh, char *filepath, Model *model)
{
  char space[] = " ";
  char buffer[64];
  int mat_index = model->mat_count-1;

  char *filepath_copy = (char *)malloc(128 * sizeof(char));

  while (fgets(buffer, 64, fh) != NULL)
  {
    if (buffer[0] == 'm' && buffer[1] == 'a') // filepath to texture
    {
      char *token = strtok(buffer, space);
      token = strtok(NULL, space);
      for (size_t i=0; i<strlen(token); i++)
        if (token[i] == '\n')
          token[i] = '\0';

      // load 100% image
      strcpy(filepath_copy, filepath);
      strcat(filepath_copy, "/");
      strcat(filepath_copy, token);
      printf("FILE: %s\n", filepath_copy);
      model->materials[mat_index] = SDL_LoadBMP(filepath_copy);

      // load 50% image
      strcpy(filepath_copy, filepath);
      strcat(filepath_copy, "/");
      strcat(filepath_copy, token);
      strcat(filepath_copy, "50");
      model->materials[mat_index + model->mat_count] = SDL_LoadBMP(filepath_copy);

      mat_index -= 1;
    }
  }

  free(filepath_copy);
}

/** Load an obj file
 */
Model load_model(char *filepath)
{
  Model model;
  model.pos = (Vector3){0, 0, 0};
  model.normal_count = 0;
  model.poly_count = 0;
  model.uv_count = 0;
  model.vertex_count = 0;

  char *last = strrchr(filepath, '/');

  char *filepath_obj = (char *)malloc(128 * sizeof(char));
  char *filepath_mtl = (char *)malloc(128 * sizeof(char));
  char *filepath_slash = (char *)malloc(128 * sizeof(char));
  strcpy(filepath_slash, filepath);

  strcpy(filepath_obj, filepath);
  strcpy(filepath_mtl, filepath);
  strcat(filepath_obj, last);
  strcat(filepath_obj, ".obj");
  strcat(filepath_mtl, last);
  strcat(filepath_mtl, ".mtl");

  FILE *fh = fopen(filepath_obj, "r");
  if (fh == NULL)
    printf("Error opening %s\n", filepath_obj);

  count_polygons(fh, &model);

  model.polygons = (Polygon *)calloc(model.poly_count, sizeof(Polygon)); // Array of polygons
  model.materials = (SDL_Surface **)malloc(model.mat_count*2 * sizeof(SDL_Surface *)); // Array of sdl surfaces

  load_polygons(fh, &model, model.polygons);
  fclose(fh);

  FILE *fh2 = fopen(filepath_mtl, "r");
  if (fh2 == NULL)
    printf("Error opening %s\n", filepath_mtl);  
  load_material(fh2, filepath_slash, &model);
  fclose(fh2);


  // for each polygon, if tex coord < 0, add 1 until greater than 0.
  // if > 1, subtract 1 until < 1.
  // then multiply u by width and v by height.
  for (int i=0; i<model.poly_count; i++)
  {
    for (int j=0; j<3; j++)
    {
      model.polygons[i].uvs[j].x *= model.materials[model.polygons[i].mat_index]->w;
      model.polygons[i].uvs[j].y *= model.materials[model.polygons[i].mat_index]->h;
    }
  }

  free(filepath_obj);
  free(filepath_mtl);
  free(filepath_slash);


  for (int i=0; i<model.vertex_count; i++)
    for (int j=0; j<3; j++)
      printf("%.2f %.2f %.2f\n", model.polygons[i].normals[j].x, model.polygons[i].normals[j].y, model.polygons[i].normals[j].z);

  // for (int i=0; i<model.vertex_count; i++)
  //   printf("%.2f %.2f %.2f\n", model.vertex_normals[i].x, model.vertex_normals[i].y, model.vertex_normals[i].z);

  return model;
}

void fill_model(Model *model, int r, int g, int b)
{
  for (int i=0; i<model->poly_count; i++)
  {
    model->polygons[i].fill.x = r;
    model->polygons[i].fill.y = g;
    model->polygons[i].fill.z = b;
  }
}
//-------------------------------------------------------------------------------

