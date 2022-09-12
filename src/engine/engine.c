#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <cblas.h>
#include <stdbool.h>

#include "vector.h"
#include "camera.h"
#include "engine.h"

float XOFFSET = 0;
float YOFFSET = 0;
float ZOFFSET = 0;

float XROTATION = 0;
float YROTATION = 0;
float ZROTATION = 0;


// TRANSFORMATIONS
//-------------------------------------------------------------------------------
void translate_world(float x, float y, float z)
{
  XOFFSET += x;
  YOFFSET += y;
  ZOFFSET += z;
}

void rotate_world(float x, float y, float z)
{
  XROTATION += x;
  YROTATION += y;
  ZROTATION += z;
}

void translate_point(Vector3 *point, float x, float y, float z)
{
  point->x += x;
  point->y += y;
  point->z += z;
}

void rotate_point_x(Vector3 *point, float x)
{
  float rot_x[3][3] = {
    { 1,      0,       0 },
    { 0, cos(x), -sin(x) },
    { 0, sin(x),  cos(x) }
  };

  float result[3][1];
  float point_as_array[3][1] = {
    {point->x},
    {point->y},
    {point->z}
  };
  matrix_mult(3, 3, 3, 1, result, rot_x, point_as_array);
  *point = (Vector3){result[0][0], result[1][0], result[2][0]};
}

void rotate_point_y(Vector3 *point, float y)
{
    float rot_y[3][3] = {
    { cos(y),  0, sin(y) },
    { 0,       1,      0 },
    { -sin(y), 0, cos(y) }
  };
  
  float result[3][1];
  float point_as_array[3][1] = {
    {point->x},
    {point->y},
    {point->z}
  };
  matrix_mult(3, 3, 3, 1, result, rot_y, point_as_array);
  *point = (Vector3){result[0][0], result[1][0], result[2][0]};
}

void rotate_point_z(Vector3 *point, float z)
{

  float rot_z[3][3] = {
    { cos(z),  -sin(z), 0 },
    { sin(z),   cos(z), 0 },
    { 0,             0, 1 }
  };

  float result[3][1];
  float point_as_array[3][1] = {
    {point->x},
    {point->y},
    {point->z}
  };
  matrix_mult(3, 3, 3, 1, result, rot_z, point_as_array);
  *point = (Vector3){result[0][0], result[1][0], result[2][0]};
}


void reset_translation()
{

}

void reset_rotation()
{

}


void rotate_x(Model model, float r)
{
  float rot_x[3][3] = {
    { 1,      0,       0 },
    { 0, cos(r), -sin(r) },
    { 0, sin(r),  cos(r) }
  };

  float result[3][1];

  for (int i=0; i<model.vertex_count; i++)
  {
    float coord[3][1] = {{model.vertices[i].x}, {model.vertices[i].y}, {model.vertices[i].z}};
    matrix_mult(3, 3, 3, 1, result, rot_x, coord);
    model.vertices[i].x = result[0][0];
    model.vertices[i].y = result[1][0];
    model.vertices[i].z = result[2][0];
  }
}

void rotate_y(Model model, float r)
{
  float rot_y[3][3] = {
    { cos(r),  0, sin(r) },
    { 0,       1,      0 },
    { -sin(r), 0, cos(r) }
  };

  float result[3][1];

  for (int i=0; i<model.vertex_count; i++)
  {
    float coord[3][1] = {{model.vertices[i].x}, {model.vertices[i].y}, {model.vertices[i].z}};
    matrix_mult(3, 3, 3, 1, result, rot_y, coord);
    model.vertices[i].x = result[0][0];
    model.vertices[i].y = result[1][0];
    model.vertices[i].z = result[2][0];
  }
}

void rotate_z(Model model, float r)
{
  float rot_z[3][3] = {
    { cos(r),  -sin(r), 0 },
    { sin(r),   cos(r), 0 },
    { 0,             0, 1 }
  };

  float result[3][1];

  for (int i=0; i<model.vertex_count; i++)
  {
    float coord[3][1] = {{model.vertices[i].x}, {model.vertices[i].y}, {model.vertices[i].z}};
    matrix_mult(3, 3, 3, 1, result, rot_z, coord);
    model.vertices[i].x = result[0][0];
    model.vertices[i].y = result[1][0];
    model.vertices[i].z = result[2][0];
  }
}
//-------------------------------------------------------------------------------


// DRAWING
//-------------------------------------------------------------------------------
bool in_range(float n, float l, float u)
{
  return (n >= l && n <= u) ? true : false;
}

void line_2d(SDL_Renderer *renderer, float x1, float y1, float x2, float y2)
{
  float m = (float)(y1-y2) / (float)(x1-x2); // slope
  float c = y1 - m*x1; // constant

  // If vertical
  if (m < -100 || m > 100)
  {
    if (y1 < y2)
      for (int y=y1; y<y2; y++)
        SDL_RenderDrawPoint(renderer, x1, y);
    else if (y1 > y2)
      for (int y=y2; y<y1; y++)
        SDL_RenderDrawPoint(renderer, x1, y);
  }

  // if gradient is not between -1 and 1
  else if (!in_range(m, -1, 1))
  {
    if (y1 < y2)
      for (int y=y1; y<y2; y++)
        SDL_RenderDrawPoint(renderer, (y-c) / m, y);
    else if (y1 > y2)
      for (int y=y2; y<y1; y++)
        SDL_RenderDrawPoint(renderer, (y-c) / m, y);
  }

  // if gradient is between -1 and 1
  else
  {
    if (x1 < x2)
      for (int x=x1; x<=x2; x++)
        SDL_RenderDrawPoint(renderer, x, m*x + c);

    else if (x1 > x2)
      for (int x=x2; x<=x1; x++)
        SDL_RenderDrawPoint(renderer, x, m*x + c);
  }
}

bool in_viewport(Camera cam, Vector3 point)
{
  float dist = vector3_dist(point, cam.pos);
  if (dist > 30 || dist < 1)
    return false;
  Vector3 temp = vector3_sub(cam.pos, (Vector3){XOFFSET, YOFFSET, ZOFFSET});
  Vector3 trans_point = vector3_sub(point, temp);
  float angle = vector3_angle(cam.dir, trans_point);

  // printf("cam-obj: %f\n", angle);
  if (angle < 0.4)
    return true;
  else
    return false;

}

void line_3d(SDL_Renderer *renderer, Camera cam, Vector3 p1, Vector3 p2)
{
  if (in_viewport(cam, p1) && in_viewport(cam, p2))
  {
    Vector2 screen_p1 = project_coordinate(cam, p1);
    Vector2 screen_p2 = project_coordinate(cam, p2);
    line_2d(renderer, screen_p1.x, screen_p1.y, screen_p2.x, screen_p2.y);
  }
}

void draw_model(SDL_Renderer *ren, Camera cam, Model model)
{
  translate_world(model.pos.x, model.pos.y, model.pos.z);
  for (int i=0; i<model.polygon_count; i++)
  {
    line_3d(ren, cam, model.vertices[model.polygon_order[i][0]-1], model.vertices[model.polygon_order[i][1]-1]);
    line_3d(ren, cam, model.vertices[model.polygon_order[i][1]-1], model.vertices[model.polygon_order[i][2]-1]);
    line_3d(ren, cam, model.vertices[model.polygon_order[i][2]-1], model.vertices[model.polygon_order[i][0]-1]);
  }
  translate_world(-model.pos.x, -model.pos.y, -model.pos.z);

}
//-------------------------------------------------------------------------------


/** Project a 3D world coordinate onto a 2D screen coordinate.
 */
Vector2 project_coordinate(Camera cam, Vector3 pt)
{
  float m1[9] = {
    1, 0,              0,
    0, cos(cam.R.x), sin(cam.R.x),
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
    pt.x - cam.pos.x + XOFFSET,
    pt.y - cam.pos.y + YOFFSET, 
    pt.z - cam.pos.z + ZOFFSET
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
  screen_point.x = (cam.fov/d[2]) * d[0] + 500;
  screen_point.y = (-cam.fov/d[2]) * d[1] + 500;

  // printf("d.x: %f, d.y: %f, d.z: %f\n", d[0], d[1], d[2]);
  // printf("x: %f, y: %f\n", screen_point.x, screen_point.y);

  return screen_point;
}

Vector2 project_coordinate_without_cblas(Camera cam, Vector3 pt)
{
  float m1[3][3] = {
    { 1, 0,            0            },
    { 0, cos(cam.R.x), sin(cam.R.x) },
    { 0, sin(cam.R.x), cos(cam.R.x) }
  };

  float m2[3][3] = {
    { cos(cam.R.y),  0, -sin(cam.R.y) },
    { 0,             1, 0             },
    { sin(cam.R.y),  0, cos(cam.R.y)  }
  };

  float m3[3][3] = {
    { cos(cam.R.z),  sin(cam.R.z), 0 },
    { -sin(cam.R.z), cos(cam.R.z), 0 },
    { 0,             0,            1 }
  };

  float m4[3][1] = {
    { pt.x - cam.pos.x + XOFFSET },
    { pt.y - cam.pos.y + YOFFSET },
    { pt.z - cam.pos.z + ZOFFSET }
  };

  float d_0[3][3];
  float d_1[3][1];
  float d[3][1];

  matrix_mult(3, 3, 3, 3, d_0, m1, m2);
  matrix_mult(3, 3, 3, 1, d_1, m3, m4);
  matrix_mult(3, 3, 3, 1, d, d_0, d_1);


  Vector2 screen_point;
  screen_point.x = (cam.fov/d[2][0]) * d[0][0] + 1000;
  screen_point.y = (-cam.fov/d[2][0]) * d[1][0] + 500;

  // printf("d.x: %f, d.y: %f, d.z: %f\n", d[0], d[1], d[2]);
  // printf("x: %f, y: %f\n", screen_point.x, screen_point.y);

  return screen_point;
}

// FILE I/O
//-------------------------------------------------------------------------------
Vector3 *load_vertices(int *vertex_count, int *polygon_count, char filepath[])
{
  // Determine number of vertices
  FILE *fh = fopen(filepath, "r");
  if (fh == NULL)
    printf("Error opening %s\n", filepath);

  char buffer[64];
  *vertex_count = 0;
  *polygon_count = 0;
  while (fgets(buffer, 64, fh) != NULL)
  {
    // Only looking at vertex data
    if (buffer[0] == 'v' && buffer[1] == ' ')
      *vertex_count += 1;
    if (buffer[0] == 'f' && buffer[1] == ' ')
      *polygon_count += 1;
  }

  rewind(fh);

  // Allocate Vector3 array
  Vector3 *vertices = (Vector3 *)malloc(*polygon_count * sizeof(Vector3));

  // Load vertices
  int index = 0;
  while (fgets(buffer, 64, fh) != NULL)
  {
    // Only looking at vertex data
    if (buffer[0] == 'v' && buffer[1] == ' ')
    {
      float temp[3];
      char delim[] = " ";
      char *token = strtok(buffer, delim);
      for (int i=0; i<3; i++)
      {
        token = strtok(NULL, delim);
        temp[i] = atof(token);
      }
      vertices[index++] = (Vector3){temp[0], temp[1], temp[2]};
    }
  }
  fclose(fh);

  // for (int i=0; i<line_count; i++)
  //   printf("%f %f %f\n", vertices[i].x, vertices[i].y, vertices[i].z);
  // printf("\n");

  return vertices;
}

void substr(char dest[], char src[])
{
  for (int i=0; i<=strlen(src); i++)
  {
    // printf("str[%d]: %c\n", i, src[i]);
    if (src[i] == '/')
      break;
    dest[i] = src[i];
  }
}

void load_vertex_order(int face_count, int **polygon_order, char filepath[])
{
  FILE *fh = fopen(filepath, "r");
  if (fh == NULL)
    printf("Error opening %s\n", filepath);



  char buffer[64];
  int index = 0;
  char delim[] = " ";
  while (fgets(buffer, 64, fh) != NULL)
  {
    if (buffer[0] == 'f' && buffer[1] == ' ')
    {
      char *token = strtok(buffer, delim);
      for (int i=0; i<3; i++)
      {
        token = strtok(NULL, delim);
        
        char temp[5] = "    \0";
        substr(temp, token);  
        
        // printf("%s == %d?\n", temp, atoi(temp));

        polygon_order[index][i] = atoi(temp);
        // printf("%d\n", polygon_order[index][i]);
      }
      index += 1; 
    }
  }


  fclose(fh);
}

/** Load an obj file
 */
Model load_model(char *filepath)
{
  Model model;
  int vertex_count;
  int polygon_count;
  model.vertices = load_vertices(&vertex_count, &polygon_count, filepath);
  model.vertex_count = vertex_count;
  model.polygon_count = polygon_count;

  model.polygon_order = (int **)malloc(model.polygon_count * sizeof(int *));
  for (int i=0; i<model.polygon_count; i++)
    model.polygon_order[i] = (int *)malloc(3 * sizeof(int)); // three vertices per polygon.

  load_vertex_order(polygon_count, model.polygon_order, filepath);

  // for (int i=0; i<polygon_count; i++)
  // {
  //   for (int j=0; j<3; j++)
  //     printf("%d ", model.polygon_order[i][j]);
  //   printf("\n"); 
  // }

  return model;
}
//-------------------------------------------------------------------------------
