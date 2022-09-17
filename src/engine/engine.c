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

float sign(Vector2 p1, Vector2 p2, Vector2 p3)
{
  return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
}

bool PointInTriangle (Vector2 pt, Vector2 v1, Vector2 v2, Vector2 v3)
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

void triangle_2d(Camera cam, Vector3 fill, Vector3 p1, Vector3 p2, Vector3 p3)
{

  Vector2 c1 = project_coordinate(p1);
  Vector2 c2 = project_coordinate(p2);
  Vector2 c3 = project_coordinate(p3);

  line_2d(fill, c1, c2);
  line_2d(fill, c2, c3);
  line_2d(fill, c3, c1);

  int lx = MIN(c1.x, MIN(c2.x, c3.x));
  int hx = MAX(c1.x, MAX(c2.x, c3.x));
  int ly = MIN(c1.y, MIN(c2.y, c3.y));
  int hy = MAX(c1.y, MAX(c2.y, c3.y));

  

}

Vector3 CLIP_point_of_intersect(Vector3 plane_normal, Vector3 p1, Vector3 p2)
{
  float ad = vector3_dot(p1, plane_normal);
  float bd = vector3_dot(p2, plane_normal);
  float t = -ad / (bd - ad);

  Vector3 lste = vector3_sub(p2, p1);
  Vector3 lti = vector3_scale(lste, t);
  return vector3_add(p1, lti);
}

void CLIP_two_outside( Vector3 plane_normal, Polygon *tri)
{
  // find which point is inside plane
  int index_of_inside_point = 0;
  for (int i=0; i<3; i++)
  {
    // if positive, in front of plane.
    float n = vector3_dot(plane_normal, tri->vertices[i]);
    if (n > 0)
    {
      index_of_inside_point = i;
      break;
    }
  }

  // inside point found, now get points where lines intersect plane
  Vector3 point1;
  Vector3 point2;
  int count = 0;
  for (int i=0; i<3; i++)
  {
    if (i != index_of_inside_point)
    {
      if (count == 0)
      {
        tri->vertices[i] = CLIP_point_of_intersect(plane_normal, tri->vertices[index_of_inside_point], tri->vertices[i]);
        count = 1;
      }
      else if (count == 1)
        tri->vertices[i] = CLIP_point_of_intersect(plane_normal, tri->vertices[index_of_inside_point], tri->vertices[i]);
    }
  }
}

void CLIP_one_outside( Vector3 plane_normal, Polygon tri_in,
                              Polygon *tri_out_1, Polygon *tri_out_2 )
{
  // find which point is outside plane
  int index_of_outside_point = 0;
  for (int i=0; i<3; i++)
  {
    // if positive, in front of plane.
    float n = vector3_dot(plane_normal, tri_in.vertices[i]);
    if (n < 0)
    {
      index_of_outside_point = i;
      break;
    }
  }

  // find the two points of intersection
  Vector3 point1;
  Vector3 point2;
  int count = 0;
  for (int i=0; i<3; i++)
  {
    if (i != index_of_outside_point)
    {
      if (count == 0)
      {
        point1 = CLIP_point_of_intersect(plane_normal, tri_in.vertices[index_of_outside_point], tri_in.vertices[i]);
        count = 1;
      }
      else if (count == 1)
        point2 = CLIP_point_of_intersect(plane_normal, tri_in.vertices[index_of_outside_point], tri_in.vertices[i]);
    }
  }

  for (int i=0; i<3; i++)
  {
    if (i != index_of_outside_point)
      tri_out_1->vertices[i] = tri_in.vertices[i];
    else
      tri_out_1->vertices[i] = point1;
  }
  for (int i=0; i<3; i++)
  {
    if (i != index_of_outside_point)
      tri_out_2->vertices[i] = tri_in.vertices[i];
    else
      tri_out_2->vertices[i] = point2;
  }
}

/** Return the number of points that lay outside the clipping plane.
 */
int CLIP_points_outside(Vector3 plane_normal, Polygon tri)
{
  float dot1 = vector3_dot(plane_normal, tri.vertices[0]);
  float dot2 = vector3_dot(plane_normal, tri.vertices[1]);
  float dot3 = vector3_dot(plane_normal, tri.vertices[2]);

  // get number of vertices on outside of plane.
  int number_of_points = 0;
  if (dot1 < 0) number_of_points += 1;
  if (dot2 < 0) number_of_points += 1;
  if (dot3 < 0) number_of_points += 1;

  return number_of_points;
}

void draw_polygon(  Camera cam, Polygon tri,
                    Vector3 l_norm, Vector3 r_norm,
                    Vector3 t_norm, Vector3 b_norm )
{
  Vector3 normals[4] = {l_norm, r_norm, t_norm, b_norm};

  Polygon out1;
  Polygon out2;

  for (int i=0; i<4; i++)
  {
    switch (CLIP_points_outside(normals[i], tri))
    {
      case (0):
        triangle_2d(cam, tri.fill, tri.vertices[0], tri.vertices[1], tri.vertices[2]);
        break;

      case (1):
        CLIP_one_outside(normals[i], tri, &out1, &out2);
        triangle_2d(cam, tri.fill, out1.vertices[0], out1.vertices[1], out1.vertices[2]);
        triangle_2d(cam, tri.fill, out2.vertices[0], out2.vertices[1], out2.vertices[2]);
        break;
      
      case (2):
        CLIP_two_outside(normals[i], &tri);
        triangle_2d(cam, tri.fill, tri.vertices[0], tri.vertices[1], tri.vertices[2]);
        break;
    }
  }
}

void draw_model(Camera cam, Model model)
{
  Polygon *polygons = (Polygon *)malloc(model.polygon_count * sizeof(Polygon));
  for (int i=0; i<model.polygon_count; i++)
    polygons[i] = model.polygons[i];

  for (int i=0; i<model.polygon_count; i++)
  {
    for (int j=0; j<3; j++)
    {
      polygons[i].vertices[j].x -= cam.pos.x;
      polygons[i].vertices[j].y -= cam.pos.y;
      polygons[i].vertices[j].z -= cam.pos.z;
      rotate_point(&polygons[i].vertices[j], 0, cam.R.y, 0);
      rotate_point(&polygons[i].vertices[j], cam.R.x, 0, 0);
    }
      float angle = vector3_angle(polygons[i].normal_vector, lightsource);
      polygons[i].fill.x -= 20*angle;
      polygons[i].fill.y -= 20*angle;
      polygons[i].fill.z -= 20*angle;

    // if (vector3_dot(vector3_sub(polygons[i].vertices[0], cam.pos), polygons[i].normal_vector) < 0)
      draw_polygon(cam, polygons[i], cam.l_norm, cam.r_norm, cam.t_norm, cam.b_norm);
  }

  free(polygons);
}
//-------------------------------------------------------------------------------

/** Project a 3D world coordinate onto a 2D screen coordinate.
 */
Vector2 project_coordinate(Vector3 pt)
{
  // 3d to 2d transform
  //-----------------------------------
  float nearplane_width = 500;
  float nearplane_height = 500;
  float nearplane_z = 1;

  float canvas_x = (1/pt.z) * pt.x * nearplane_z * nearplane_width;
  float canvas_y = (1/pt.z) * pt.y * nearplane_z * nearplane_height;
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
  model.fill = (Vector3){0, 0, 0};
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

