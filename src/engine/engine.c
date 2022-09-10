#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "vector.h"
#include "camera.h"

/** Load vertices from an obj file.
 * 
 */
Vector3 *load_vertices(int *face_count, char filepath[])
{
  // Determine number of vertices
  FILE *fh = fopen(filepath, "r");
  if (fh == NULL)
    printf("Error opening %s\n", filepath);

  char buffer[64];
  int line_count = 0;
  *face_count = 0;
  while (fgets(buffer, 64, fh) != NULL)
  {
    if (feof(fh))
      break;

    // Only looking at vertex data
    if (buffer[0] == 'v' && buffer[1] == ' ')
      line_count += 1;
    if (buffer[0] == 'f' && buffer[1] == ' ')
    {
      printf("%s\n", buffer);
      *face_count += 1;
    }
  }

  rewind(fh);

  // Allocate Vector3 array
  Vector3 *vertices = (Vector3 *)malloc(*face_count * sizeof(Vector3));

  // Load vertices
  int index = 0;
  while (fgets(buffer, 64, fh) != NULL)
  {
    if (feof(fh))
      break;

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

void load_vertex_order(int face_count, int **triangles, char filepath[])
{
  // Get face count

  FILE *fh = fopen(filepath, "r");
  if (fh == NULL)
    printf("Error opening %s\n", filepath);

  char buffer[64];
  int index = 0;
  char delim[] = " ";

  while (1)
  {
    if (feof(fh))
      break;
    fgets(buffer, 64, fh);

    // Only looking at face data
    if (buffer[0] == 'f' && buffer[1] == ' ')
    {
      char *token = strtok(buffer, delim);
      for (int i=0; i<3; i++) // three vertices per face in triangulated obj file
      {
        char temp[5] = "    \0";
        token = strtok(NULL, delim);
        substr(temp, token);
        // printf("Token: %s, temp: %s\n", token, temp);
        triangles[index][i] = atoi(temp);
      }
      index += 1;
    }
  }

  fclose(fh);
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
  screen_point.x = (500/d[2][0]) * d[0][0] + 500;
  screen_point.y = (500/d[2][0]) * d[1][0] + 500;

  return screen_point;
}