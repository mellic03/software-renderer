#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <SDL2/SDL.h>
#include "vector.h"
#include "camera.h"
#include "engine.h"
/** Load vertices from an obj file.
 * 
 */
Vector3 *load_vertices(int *polygon_count, char filepath[])
{
  // Determine number of vertices
  FILE *fh = fopen(filepath, "r");
  if (fh == NULL)
    printf("Error opening %s\n", filepath);

  char buffer[64];
  int line_count = 0;
  *polygon_count = 0;
  while (fgets(buffer, 64, fh) != NULL)
  {
    // Only looking at vertex data
    if (buffer[0] == 'v' && buffer[1] == ' ')
      line_count += 1;
    if (buffer[0] == 'f' && buffer[1] == ' ')
    {
      // printf("%s\n", buffer);
      *polygon_count += 1;
    }
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


Model load_model(char *filepath)
{
  Model model;
  int polygon_count;
  model.vertices = load_vertices(&polygon_count, filepath);
  model.polygon_count = polygon_count;

  model.polygon_order = (int **)malloc(model.polygon_count * sizeof(int *));
  for (int i=0; i<model.polygon_count; i++)
    model.polygon_order[i] = (int *)malloc(3 * sizeof(int)); // three vertices per polygon.

  load_vertex_order(polygon_count, model.polygon_order, filepath);

  for (int i=0; i<polygon_count; i++)
  {
    for (int j=0; j<3; j++)
      printf("%d ", model.polygon_order[i][j]);
    printf("\n"); 
  }

  return model;
}

void draw_model(SDL_Renderer *ren, Camera cam, Model model)
{
  for (int i=0; i<model.polygon_count; i++)
  {
    line_3d(ren, cam, model.vertices[model.polygon_order[i][0]-1], model.vertices[model.polygon_order[i][1]-1]);
    line_3d(ren, cam, model.vertices[model.polygon_order[i][1]-1], model.vertices[model.polygon_order[i][2]-1]);
    line_3d(ren, cam, model.vertices[model.polygon_order[i][2]-1], model.vertices[model.polygon_order[i][0]-1]);
  }
}

/** Project a 3D world coordinate onto a 2D screen coordinate.
 */
Vector2 project_coordinate(Camera cam, Vector3 pt)
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
    { pt.x - cam.pos.x },
    { pt.y - cam.pos.y },
    { pt.z - cam.pos.z }
  };

  float d_0[3][3];
  float d_1[3][1];
  float d[3][1];

  matrix_mult(3, 3, 3, 3, d_0, m1, m2);
  matrix_mult(3, 3, 3, 1, d_1, m3, m4);
  matrix_mult(3, 3, 3, 1, d, d_0, d_1);

  d[2][0] = (d[2][0] == 0) ? 1 : d[2][0];
  
  Vector2 screen_point;
  screen_point.x = (500/d[2][0]) * d[0][0] + 640;
  screen_point.y = (500/d[2][0]) * d[1][0] + 360;

  return screen_point;
}