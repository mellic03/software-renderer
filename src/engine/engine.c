#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <cblas.h>
#include <stdbool.h>
#include <pthread.h>

#include "vector.h"
#include "camera.h"
#include "engine.h"
#include "screen.h"

uint8_t pixels[SCREEN_WIDTH * SCREEN_HEIGHT * 4] = { 0 };
int z_buffer[SCREEN_WIDTH * SCREEN_HEIGHT] = { 0 };
SDL_Texture *window_texture;

Vector3 lightsource = {20, 20, -10};
Vector3 camera_pos; // Needed for qsort()

// TRANSFORMATIONS
//-------------------------------------------------------------------------------
void translate_model(Model *model, float x, float y, float z)
{
  for (int i=0; i<model->polygon_count; i++)
    for (int j=0; j<3; j++)
    {
      model->polygons[i].vertices[j].x += x;
      model->polygons[i].vertices[j].y += y;
      model->polygons[i].vertices[j].z += z;
    }

}

void translate_point(Vector3 *point, float x, float y, float z)
{
  point->x += x;
  point->y += y;
  point->z += z;
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

  float pt_as_arr[3][1] = {{pt->x}, {pt->y}, {pt->z}};
  matrix_mult(3, 3, 3, 1, output, rot_x, pt_as_arr);
  matrix_mult(3, 3, 3, 1, output2, rot_y, output);

  pt->x = output2[0][0];
  pt->y = output2[1][0];
  pt->z = output2[2][0];
}

void rotate_x(Model model, float r)
{
  // float rot_x[3][3] = {
  //   { 1, 0,       0      },
  //   { 0, cos(r), -sin(r) },
  //   { 0, sin(r),  cos(r) }
  // };

  // float result[3][1];

  // for (int i=0; i<model.vertex_count; i++)
  // {
  //   float coord[3][1] = {{model.vertices[i].x}, {model.vertices[i].y}, {model.vertices[i].z}};
  //   matrix_mult(3, 3, 3, 1, result, rot_x, coord);
  //   model.vertices[i].x = result[0][0];
  //   model.vertices[i].y = result[1][0];
  //   model.vertices[i].z = result[2][0];
  // }
}

void rotate_y(Model model, float r)
{
  float rot_y[3][3] = {
    { cos(r),  0, sin(r) },
    { 0,       1, 0      },
    { -sin(r), 0, cos(r) }
  };

  float result[3][1];

  for (int i=0; i<model.polygon_count; i++)
  {
    // rotate vertices
    for (int j=0; j<3; j++)
    {
      float coord[3][1] = {{model.polygons[i].vertices[j].x}, {model.polygons[i].vertices[j].y}, {model.polygons[i].vertices[j].z}};
      matrix_mult(3, 3, 3, 1, result, rot_y, coord);
      model.polygons[i].vertices[j].x = result[0][0];
      model.polygons[i].vertices[j].y = result[1][0];
      model.polygons[i].vertices[j].z = result[2][0];
    }

    // rotate normals
    float coord[3][1] = {{model.polygons[i].normal_vector.x}, {model.polygons[i].normal_vector.y}, {model.polygons[i].normal_vector.z}};
    matrix_mult(3, 3, 3, 1, result, rot_y, coord);
    model.polygons[i].normal_vector.x = result[0][0];
    model.polygons[i].normal_vector.y = result[1][0];
    model.polygons[i].normal_vector.z = result[2][0];
  }
}

void rotate_z(Model model, float r)
{
  // float rot_z[3][3] = {
  //   { cos(r), -sin(r), 0 },
  //   { sin(r), cos(r),  0 },
  //   { 0,      0,       1 }
  // };

  // float result[3][1];

  // for (int i=0; i<model.vertex_count; i++)
  // {
  //   float coord[3][1] = {{model.vertices[i].x}, {model.vertices[i].y}, {model.vertices[i].z}};
  //   matrix_mult(3, 3, 3, 1, result, rot_z, coord);
  //   model.vertices[i].x = result[0][0];
  //   model.vertices[i].y = result[1][0];
  //   model.vertices[i].z = result[2][0];
  // }
}
//-------------------------------------------------------------------------------

// DRAWING
//-------------------------------------------------------------------------------
void clear_screen(uint8_t r, uint8_t g, uint8_t b)
{
  for (int i=0; i<SCREEN_WIDTH; i++)
    for (int j=0; j<SCREEN_HEIGHT; j++)
    {
      pixels[3*SCREEN_WIDTH*j + 3*i + 0] = r;
      pixels[3*SCREEN_WIDTH*j + 3*i + 1] = g;
      pixels[3*SCREEN_WIDTH*j + 3*i + 2] = b;
    }

  for (int i=0; i<SCREEN_WIDTH; i++)
    for (int j=0; j<SCREEN_HEIGHT; j++)
      z_buffer[SCREEN_WIDTH*j + i] = RENDER_DISTANCE*2;
}

void render_screen(SDL_Renderer *ren)
{
  int texture_pitch = 0;
  void *texture_pixels = NULL;
  if (SDL_LockTexture(window_texture, NULL, &texture_pixels, &texture_pitch) != 0)
      SDL_Log("Unable to lock texture: %s", SDL_GetError());
  else
    memcpy(texture_pixels, pixels, texture_pitch * SCREEN_HEIGHT);
  SDL_UnlockTexture(window_texture);
  SDL_RenderClear(ren);
  SDL_RenderCopy(ren, window_texture, NULL, NULL);
  SDL_RenderPresent(ren);
}

void pixel(int x, int y, int r, int g, int b)
{
  pixels[3*SCREEN_WIDTH*y + 3*x + 0] = r;
  pixels[3*SCREEN_WIDTH*y + 3*x + 1] = g;
  pixels[3*SCREEN_WIDTH*y + 3*x + 2] = b;
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

float sign(Vector2 p1, Vector2 *p2, Vector2 *p3)
{
  return (p1.x - p3->x) * (p2->y - p3->y) - (p2->x - p3->x) * (p1.y - p3->y);
}

bool PointInTriangle (Vector2 pt, Vector2 *v1, Vector2 *v2, Vector2 *v3)
{
  float d1, d2, d3;
  bool has_neg, has_pos;

  d1 = sign(pt, v1, v2);
  d2 = sign(pt, v2, v3);
  d3 = sign(pt, v3, v1);

  has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
  has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

  return !(has_neg && has_pos);
}

void triangle_2d(Camera *cam, Polygon *tri, float z)
{
  Vector2 p1 = project_coordinate(&tri->vertices[0]);
  Vector2 p2 = project_coordinate(&tri->vertices[1]);
  Vector2 p3 = project_coordinate(&tri->vertices[2]);

  // line_2d((Vector3){0, 0, 0}, p1, p2);
  // line_2d((Vector3){0, 0, 0}, p2, p3);
  // line_2d((Vector3){0, 0, 0}, p3, p1);

  int lx = MIN(p1.x, MIN(p2.x, p3.x));
  int hx = MAX(p1.x, MAX(p2.x, p3.x));
  int ly = MIN(p1.y, MIN(p2.y, p3.y));
  int hy = MAX(p1.y, MAX(p2.y, p3.y));
  
  for (int x=lx; x<=hx; x++)
    for (int y=ly; y<=hy; y++)
      if (PointInTriangle((Vector2){x, y}, &p1, &p2, &p3) && tri->vertices[0].z)
      {
        pixel(x, y, tri->fill.x, tri->fill.y, tri->fill.z);
      }
}

Vector3 CLIP_point_of_intersect(Vector3 plane_normal, Vector3 p1, Vector3 p2)
{
  float ad = vector3_dot(p1, plane_normal);
  float bd = vector3_dot(p2, plane_normal);
  float t = ( - ad) / (bd - ad);

  Vector3 lste = vector3_sub(p2, p1);
  Vector3 lti = vector3_scale(lste, t);
  return vector3_add(p1, lti);
}

/** Return the number of points that lay outside the clipping plane.
 */
int CLIP_points_outside(Vector3 *plane_normal, Polygon *tri)
{
  float dot1 = vector3_dot(*plane_normal, tri->vertices[0]);
  float dot2 = vector3_dot(*plane_normal, tri->vertices[1]);
  float dot3 = vector3_dot(*plane_normal, tri->vertices[2]);
  int number_of_points = 0;
  if (dot1 < 0) number_of_points += 1;
  if (dot2 < 0) number_of_points += 1;
  if (dot3 < 0) number_of_points += 1;

  return number_of_points;
}

/** Return the number of points with positive signed distance to a plane normal
 * @param index_of_inside index position of point that has +ve distance (if one point is +ve)
 * @param index_of_outside index position of point that has -ve distance (if one point is -ve)
 */
int CLIP_points_inside(Vector3 plane_normal, Polygon *tri, int *index_of_inside, int *index_of_outside)
{
  float dot1 = vector3_dot(plane_normal, tri->vertices[0]);
  float dot2 = vector3_dot(plane_normal, tri->vertices[1]);
  float dot3 = vector3_dot(plane_normal, tri->vertices[2]);
  int number_of_inside = 0;
  
  if (dot1 > 0) number_of_inside += 1;
  if (dot2 > 0) number_of_inside += 1;
  if (dot3 > 0) number_of_inside += 1;

  // printf("d0: %f, d1: %f, d2: %f\n", dot1, dot2, dot3);

  if (dot1 > dot2 && dot1 > dot3) *index_of_inside = 0;
  if (dot2 > dot1 && dot2 > dot3) *index_of_inside = 1;
  if (dot3 > dot1 && dot3 > dot2) *index_of_inside = 2;
  if (dot1 < dot2 && dot1 < dot3) *index_of_outside = 0;
  if (dot2 < dot1 && dot2 < dot3) *index_of_outside = 1;
  if (dot3 < dot1 && dot3 < dot2) *index_of_outside = 2;

  return number_of_inside;
}

/** clip a polygon against a plane
 * 
 * @param tris array of polygons to clip. altered by function
 * @param clipped_triangles array of clipped polygons
 * @return number of polygons formed due to clipping
 */
int CLIP_against_plane(Vector3 plane_normal, int poly_count, Polygon *unclipped_triangles, Polygon *clipped_triangles)
{
  int unclipped_index = 0;
  int clipped_index = 0;

  while (unclipped_index < poly_count)  
  {
    Polygon tri1;
    Polygon tri2;

    // n == number of triangles formed due to clipping
    int n = CLIP_poly(plane_normal, &unclipped_triangles[unclipped_index], &tri1, &tri2);
    switch (n)
    {
      case (0):
        break;

      case (1):
        memcpy(&clipped_triangles[clipped_index], &unclipped_triangles[unclipped_index], sizeof(Polygon));
        clipped_index += 1;
        break;
      
      case (2):
        memcpy(&clipped_triangles[clipped_index], &tri1, sizeof(Polygon));
        memcpy(&clipped_triangles[clipped_index+1], &tri2, sizeof(Polygon));
        clipped_index += 2;
        break;
    }
    unclipped_index += 1;
  }
  
  return clipped_index;
}


/** Clip triangles to a plane
 * @param tri_in input triangle
 * @param tri_out1 first possible output triangle
 * @param tri_out2 second possible output triangle
 * @return number of triangles formed due to clipping
 */
int CLIP_poly(Vector3 plane_normal, Polygon *tri_in, Polygon *tri_out1, Polygon *tri_out2)
{
  int index_of_inside = 0;
  int index_of_outside = 0;

  // n ==  number of points inside of plane
  int n = CLIP_points_inside(plane_normal, tri_in, &index_of_inside, &index_of_outside);
  Vector3 A, B, C;
  Vector3 A_prime, B_prime, C_prime;
  bool flag;
  // printf("in: tris[%d], out: tris[%d]\n", index_of_inside, index_of_outside);

  switch (n)
  {
    case (3):
      return 1;
    
    case (0):
      return 0;
    
    case (1):
      A = tri_in->vertices[index_of_inside];
      flag = true;
      for (int i=0; i<3; i++)
      {
        if (i != index_of_inside)
        {
          if (flag)
          {
            flag = false;
            B_prime = CLIP_point_of_intersect(plane_normal, A, tri_in->vertices[i]);
          }
          else if (!flag)
          {
            C_prime = CLIP_point_of_intersect(plane_normal, A, tri_in->vertices[i]); 
          }
        }
      }
      tri_in->vertices[0] = A;
      tri_in->vertices[1] = B_prime;
      tri_in->vertices[2] = C_prime;
      return 1;

    case (2):
      C = tri_in->vertices[index_of_outside];

      switch (index_of_outside)
      {
        case (0):
          A = tri_in->vertices[1];
          B = tri_in->vertices[2];
          A_prime = CLIP_point_of_intersect(plane_normal, C, A );
          B_prime = CLIP_point_of_intersect(plane_normal, C, B );
          break;

        case (1):
          A = tri_in->vertices[0];
          B = tri_in->vertices[2];
          A_prime = CLIP_point_of_intersect(plane_normal, C, A );
          B_prime = CLIP_point_of_intersect(plane_normal, C, B );
          break;

        case (2):
          A = tri_in->vertices[0];
          B = tri_in->vertices[1];
          A_prime = CLIP_point_of_intersect(plane_normal, C, A );
          B_prime = CLIP_point_of_intersect(plane_normal, C, B );
          break;
      }

      memcpy(&tri_out1->vertices[0], &A, sizeof(Vector3));
      memcpy(&tri_out1->vertices[1], &A_prime, sizeof(Vector3));
      memcpy(&tri_out1->vertices[2], &B_prime, sizeof(Vector3));
      
      memcpy(&tri_out2->vertices[0], &A, sizeof(Vector3));
      memcpy(&tri_out2->vertices[1], &B, sizeof(Vector3));
      memcpy(&tri_out2->vertices[2], &B_prime, sizeof(Vector3));

      return 2;
  }
}


void draw_model(Camera cam, Model *model, int fill)
{
  Polygon *unclipped_polygons = (Polygon *)malloc(model->polygon_count * sizeof(Polygon));
  for (int i=0; i<model->polygon_count; i++)
    memcpy(&unclipped_polygons[i], &model->polygons[i], sizeof(Polygon));

  for (int i=0; i<model->polygon_count; i++)
  {
    for (int j=0; j<3; j++)
    {
      unclipped_polygons[i].vertices[j].x -= cam.pos.x;
      unclipped_polygons[i].vertices[j].y -= cam.pos.y;
      unclipped_polygons[i].vertices[j].z -= cam.pos.z;
      rotate_point(&unclipped_polygons[i].vertices[j], 0, cam.R.y, 0);
      rotate_point(&unclipped_polygons[i].vertices[j], cam.R.x, 0, 0);
    }
  }

  Polygon *clipped_polygons = (Polygon *)malloc(model->polygon_count*2 * sizeof(Polygon));
  int n = CLIP_against_plane(cam.l_norm, model->polygon_count, unclipped_polygons, clipped_polygons);

  Polygon *clipped_2 = (Polygon *)malloc(n*2 * sizeof(Polygon));
  n = CLIP_against_plane(cam.r_norm, n, clipped_polygons, clipped_2);

  Polygon *clipped_3 = (Polygon *)malloc(n*2 * sizeof(Polygon));
  n = CLIP_against_plane(cam.t_norm, n, clipped_2, clipped_3);
  
  Polygon *clipped_4 = (Polygon *)malloc(n*2 * sizeof(Polygon));
  n = CLIP_against_plane(cam.b_norm, n, clipped_3, clipped_4);
  
  float angle;
  for (int i=0; i<n; i++)
  {
    triangle_2d(&cam, &clipped_4[i], clipped_4->vertices[i].z);
  }



  free(unclipped_polygons);
  free(clipped_polygons);
  free(clipped_2);
  free(clipped_3);
  free(clipped_4);
}
//-------------------------------------------------------------------------------

/** Project a 3D world coordinate onto a 2D screen coordinate.
 */
Vector2 project_coordinate(Vector3 *pt)
{
  // 3d to 2d transform
  //-----------------------------------
  float nearplane_width = 500;
  float nearplane_height = 500;
  float nearplane_z = 1;

  float canvas_x = (nearplane_z/pt->z) * pt->x * nearplane_z * nearplane_width;
  float canvas_y = (nearplane_z/pt->z) * pt->y * nearplane_z * nearplane_height;
  canvas_x += HALF_SCREEN_WIDTH;
  canvas_y += HALF_SCREEN_HEIGHT;
  //-----------------------------------

  return (Vector2){canvas_x, canvas_y};
}

Vector2 project_coordinate_cblas(Camera cam, Vector3 pt)
{
  float m1[9] = {
    1, 0,            0,
    0, cos(cam.R.x), -sin(cam.R.x),
    0, sin(cam.R.x), cos(cam.R.x) 
  };

  float m2[9] = {
    cos(cam.R.y),  0,  -sin(cam.R.y),
    0,             1,  0,
    sin(cam.R.y),  0,  cos(cam.R.y)  
  };

  float m3[9] = {
    cos(cam.R.z),  sin(cam.R.z), 0,
    -sin(cam.R.z), cos(cam.R.z), 0,
    0,             0,            1
  };

  float m4[3] = {
    pt.x - cam.pos.x,
    pt.y - cam.pos.y, 
    pt.z - cam.pos.z
  };

  float d_0[9];
  float d_1[3];
  float d[3];

  cblas_sgemm(CblasColMajor, CblasNoTrans, CblasNoTrans,
  3, // number of rows in matrix a
  3, // number of columns in matrix b
  3, // number of columns in matrix a
  1, // scalar for matrix a
  m1, // matrix a
  3, // first dimension of matrix a
  m2, // matrix b
  3, // first dimension of matrix b
  0, // scalar for matrix c
  d_0, // matrix c (output)
  3 // number of rows for matrix c
  );

  cblas_sgemm(CblasColMajor, CblasNoTrans, CblasNoTrans,
  3, // number of rows in matrix a
  1, // number of columns in matrix b
  3, // number of columns in matrix a
  1, // scalar for matrix a
  m3, // matrix a
  3, // first dimension of matrix a
  m4, // matrix b
  3, // first dimension of matrix b
  0, // scalar for matrix c
  d_1, // matrix c (output)
  3 // number of rows for matrix c
  );

  cblas_sgemm(CblasColMajor, CblasNoTrans, CblasNoTrans,
  3, // number of rows in matrix a
  1, // number of columns in matrix b
  3, // number of columns in matrix a
  1, // scalar for matrix a
  d_0, // matrix a 
  3, // first dimension of matrix a
  d_1, // matrix b
  3, // first dimension of matrix b
  0, // scalar for matrix c
  d, // matrix c (output)
  3 // number of rows for matrix c
  );

  Vector2 screen_point;
  screen_point.x = (cam.fov/d[2]) * d[0] + HALF_SCREEN_WIDTH;
  screen_point.y = (-cam.fov/d[2]) * d[1] + HALF_SCREEN_HEIGHT;

  return screen_point;
}

// FILE I/O
//-------------------------------------------------------------------------------
void count_polygons(FILE *fh, Model *model)
{
  model->polygon_count = 0;
  model->vertex_count = 0;
  model->normal_count = 0;

  char buffer[64];
  while (fgets(buffer, 64, fh) != NULL)
  {
    if (buffer[0] == 'f' && buffer[1] == ' ')
      model->polygon_count += 1;
    else if (buffer[0] == 'v' && buffer[1] == ' ')
      model->vertex_count += 1;
    else if (buffer[0] == 'v' && buffer[1] == 'n')
      model->normal_count += 1;
  }
  rewind(fh);
}

void extract_vert_text_norm(int dest[3], char *src)
{
  for (int i=0; i<strlen(src); i++)
    if (src[i] == '/')
      src[i] == ' ';

  sscanf(src, "%d/%d/%d", &dest[0], &dest[1], &dest[2]);
  // printf("%d, %d, %d\n", vertex, texture, normal);
}

void load_polygons(FILE *fh, Model model, Polygon *polygons)
{
  char buffer[64];

  char space[] = " ";
  char slash[] = "/";

  Vector3 *vertices = (Vector3 *)malloc(model.vertex_count * sizeof(Vector3));
  int vertex_index = 0;

  Vector3 *normals = (Vector3 *)malloc(model.normal_count * sizeof(Vector3));
  int normal_index = 0;

  // load all vertices and normals into memory first
  while (fgets(buffer, 64, fh) != NULL)
  {
    // Line with vertex
    if (buffer[0] == 'v' && buffer[1] == ' ')
    {
      char *token = strtok(buffer, space);
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
      char *token = strtok(buffer, space);
      float temp[3];
      for (int i=0; i<3; i++)
      {
        token = strtok(NULL, space);
        temp[i] = atof(token);
      }
      memcpy(&normals[normal_index++], &(Vector3){temp[0], temp[1], temp[2]}, sizeof(Vector3));
    }
  }
  rewind(fh);

  // Create polygons
  int polygon_index = 0;
  while (fgets(buffer, 64, fh) != NULL)
  {
    if (buffer[0] == 'f' && buffer[1] == ' ')
    {
      char *token = strtok(buffer, space); // token == "f"
      int temp[3];

      for (int i=0; i<3; i++)
      {
        token = strtok(NULL, space); // token == "xxx/xxx/xxx"
        extract_vert_text_norm(temp, token);
        memcpy(&polygons[polygon_index].vertices[i], &vertices[temp[0]-1], sizeof(Vector3));
      }
      memcpy(&polygons[polygon_index].normal_vector, &normals[temp[2]-1], sizeof(Vector3));
      polygons[polygon_index].fill = (Vector3){200, 200, 200};
      polygon_index += 1;
    }
  }

  free(vertices);
  free(normals);

  // for (int i=0; i<model.polygon_count; i++)
  // {
  //   for (int j=0; j<3; j++)
  //     printf("V: %f %f %f, ", polygons[i].vertices[j].x, polygons[i].vertices[j].y, polygons[i].vertices[j].z);
  //   printf("\n");
  // }
}

/** Load an obj file
 */
Model load_model(char *filepath)
{
  Model model;
  model.fill = (Vector3){255, 255, 255};
  model.stroke = (Vector3){255, 255, 255};
  model.pos = (Vector3){0, 0, 0};

  FILE *fh = fopen(filepath, "r");
  if (fh == NULL)
    printf("Error opening %s\n", filepath);

  count_polygons(fh, &model);

  model.polygons = (Polygon *)malloc(model.polygon_count * sizeof(Polygon)); // Array of polygons

  load_polygons(fh, model, model.polygons);

  fclose(fh);
  return model;
}
//-------------------------------------------------------------------------------

