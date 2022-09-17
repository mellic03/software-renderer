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
void clear_screen(void)
{
  for (int i=0; i<SCREEN_WIDTH; i++)
    for (int j=0; j<SCREEN_HEIGHT; j++)
    {
      pixels[3*SCREEN_WIDTH*j + 3*i + 0] = 109;
      pixels[3*SCREEN_WIDTH*j + 3*i + 1] = 133;
      pixels[3*SCREEN_WIDTH*j + 3*i + 2] = 169;
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

void pixel(SDL_Texture *texture, int x, int y, int r, int g, int b)
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
        pixel(window_texture, (int)p1.x, y, stroke.x, stroke.y, stroke.z);

    else if (p1.y > p2.y)
      for (int y=p2.y; y<p1.y; y++)
        pixel(window_texture, (int)p1.x, y, stroke.x, stroke.y, stroke.z);
  }

  // if gradient is not between -1 and 1
  else if (!in_range(m, -1, 1))
  {
    if (p1.y < p2.y)
      for (int y=p1.y; y<p2.y; y++)
        pixel(window_texture, (int)((y-c)/m), y, stroke.x, stroke.y, stroke.z);

    else if (p1.y > p2.y)
      for (int y=p2.y; y<p1.y; y++)
        pixel(window_texture, (int)((y-c)/m), y, stroke.x, stroke.y, stroke.z);
  }

  // if gradient is between -1 and 1
  else
  {
    if (p1.x < p2.x)
      for (int x=p1.x; x<=p2.x; x++)
        pixel(window_texture, x, (int)(m*x+c), stroke.x, stroke.y, stroke.z);

    else if (p1.x > p2.x)
      for (int x=p2.x; x<=p1.x; x++)
        pixel(window_texture, x, (int)(m*x+c), stroke.x, stroke.y, stroke.z);
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

void triangle_2d(Vector3 fill, Vector2 p1, Vector2 p2, Vector2 p3)
{
  line_2d(fill, p1, p2);
  line_2d(fill, p2, p3);
  line_2d(fill, p3, p1);

  // Fill
  int lx = MIN(p1.x, MIN(p2.x, p3.x));
  int hx = MAX(p1.x, MAX(p2.x, p3.x));
  int ly = MIN(p1.y, MIN(p2.y, p3.y));
  int hy = MAX(p1.y, MAX(p2.y, p3.y));

  for (int x=lx; x<=hx; x++)
    for (int y=ly; y<=hy; y++)
      if (PointInTriangle((Vector2){x, y}, p1, p2, p3))
      {
        pixels[3*SCREEN_WIDTH*y + 3*x + 0] = fill.x;
        pixels[3*SCREEN_WIDTH*y + 3*x + 1] = fill.y;
        pixels[3*SCREEN_WIDTH*y + 3*x + 2] = fill.z;
      }  
}

void triangle_3d(SDL_Renderer *ren, Camera cam, Model *model, Vector3 p1, Vector3 p2, Vector3 p3, Vector3 normal)
{
  // if (vector3_dot(vector3_sub(p1, cam.pos), normal) >= 0)
  //   return;
  // if (vector3_angle(vector3_sub(p1, cam.pos), cam.dir) > 0.8)
  //   return;

  // float angle = vector3_angle(normal, lightsource);
  // float f1 = (model->fill.x - angle*81 < 0) ? 0 : model->fill.x - angle*81;
  // float f2 = (model->fill.y - angle*81 < 0) ? 0 : model->fill.y - angle*81;
  // float f3 = (model->fill.z - angle*81 < 0) ? 0 : model->fill.z - angle*81;
  
  // Vector2 projected1;
  // Vector2 projected2;
  // Vector2 projected3;

  // bool b1 = project_coordinate(cam, p1, &projected1);
  // bool b2 = project_coordinate(cam, p2, &projected2);
  // bool b3 = project_coordinate(cam, p3, &projected3);
  // if (b1 && b2 && b3)
  //   triangle_2d(ren, (Vector3){f1, f2, f3},
  //     projected1,
  //     projected2,
  //     projected3
  //   );
}

void draw_model(Camera cam, Model model)
{
  Vector2 **vertex_array = (Vector2 **)malloc(model.polygon_count * sizeof(Vector2 *));
  for (int i=0; i<model.polygon_count; i++)
    vertex_array[i] = (Vector2 *)malloc(3 * sizeof(Vector2));

  for (int i=0; i<model.polygon_count; i++)
    for (int j=0; j<3; j++)
    {
      // 3D to 3D transform here
      model.polygons[i].vertices[j].x += cam.pos.x;
      model.polygons[i].vertices[j].y -= cam.pos.y;
      model.polygons[i].vertices[j].z += cam.pos.y;
      rotate_point(&model.polygons[i].vertices[j], 0, cam.R.y, 0);
      rotate_point(&model.polygons[i].vertices[j], cam.R.x, 0, 0);

      bool visible_or_something_idk = true;
      if (visible_or_something_idk)
        vertex_array[i][j] = project_coordinate(cam, model.polygons[i].vertices[j]);
    }


  for (int i=0; i<model.polygon_count; i++)
  {
    triangle_2d(
      model.fill,
      vertex_array[i][0],
      vertex_array[i][1],
      vertex_array[i][2]
    );
  }

  for (int i=0; i<model.polygon_count; i++)
    free(vertex_array[i]);
  free(vertex_array);
}
//-------------------------------------------------------------------------------

/** Project a 3D world coordinate onto a 2D screen coordinate.
 */
Vector2 project_coordinate(Camera cam, Vector3 pt)
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

