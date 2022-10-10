#ifdef __unix__
  #include <SDL2/SDL.h>
#elif defined(_WIN32) || defined(WIN32)
  #define SDL_MAIN_HANDLED
  #include <SDL.h>
#endif

#include "model.h"
#include "graphics.h"

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
}

void load_polygons(FILE *fh, Model *model)
{
  char buffer[64];

  char space[] = " ";
  char slash[] = "/";
  char *token;

  Vector3 *vertices = (Vector3 *)malloc(model->vertex_count * sizeof(Vector3));
  int vertex_index = 0;

  Vector3 *normals = (Vector3 *)malloc(model->normal_count * sizeof(Vector3));
  int normal_index = 0;

  Vector2 *tex_coords = (Vector2 *)malloc(model->uv_count * sizeof(Vector2));
  int tex_coord_index = 0;

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
  }
  rewind(fh);

  // SDL_Surface *test = SDL_LoadBMP("./heightmap.bmp");

  // for (int i=0; i<65; i++)
  // {
  //   for (int j=0; j<65; j++)
  //   {
  //     int indx = (int)(vertices[i*65 + j].x*1.2);
  //     int indz = (int)(vertices[i*65 + j].z/2.5);
      
  //     printf("%d %d\n", indx, indz);

  //     Uint8 *red = (Uint8 *)test->pixels + indz*test->pitch + indx;
  //     float r = (float)*red;
  //     vertices[i*65 + j].y = -r/20;
  //   }
  // }


  // Create polygons
  int polygon_index = 0;
  int mat_index = -1; // usemtl always before vertices
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
        model->polygons[polygon_index].vertices[i] = vertices[temp[0]-1];
        model->polygons[polygon_index].og_vertices[i] = vertices[temp[0]-1];
        model->polygons[polygon_index].vertex_indices[i] = temp[0]-1;
        model->polygons[polygon_index].uvs[i] = tex_coords[temp[1]-1];
      }
      
      Vector3 v1 = vector3_sub(model->polygons[polygon_index].vertices[0], model->polygons[polygon_index].vertices[1]);
      Vector3 v2 = vector3_sub(model->polygons[polygon_index].vertices[0], model->polygons[polygon_index].vertices[2]);
      model->polygons[polygon_index].face_normal = vector3_cross(v1, v2);
      vector3_normalise(&model->polygons[polygon_index].face_normal);

      model->polygons[polygon_index].mat_index = mat_index;
      polygon_index += 1;
    }

    // line with new material
    else if (buffer[0] == 'u' && buffer[1] == 's')
      mat_index += 1;
  }

  // Calculate vertex normals from face normals
  rewind(fh);
  // An array of normals where index n corresponds to vertex n
  Vector3 *vertex_normals = (Vector3 *)malloc(model->vertex_count * sizeof(Vector3));

  // For each polygon, add face normal to vertex_normals[index of vertex]
  for (int i=0; i<model->poly_count; i++)
    for (int j=0; j<3; j++)
      vertex_normals[model->polygons[i].vertex_indices[j]] = vector3_add(vertex_normals[model->polygons[i].vertex_indices[j]], model->polygons[i].face_normal);

  for (int i=0; i<model->vertex_count; i++)
    vector3_normalise(&vertex_normals[i]);

  model->vertex_normals = (Vector3 *)malloc(model->vertex_count * sizeof(Vector3));
  for (int i=0; i<model->vertex_count; i++)
    model->vertex_normals[i] = vertex_normals[i];

  model->vertices = (Vector3 *)malloc(model->vertex_count * sizeof(Vector3));
  for (int i=0; i<model->vertex_count; i++)
    model->vertices[i] = vertices[i];

  free(vertex_normals);
  free(vertices);
  free(normals);
  free(tex_coords);
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
      // strcpy(filepath_copy, filepath);
      // strcat(filepath_copy, "/");
      // strcat(filepath_copy, token);
      // strcat(filepath_copy, "50");
      // model->materials[mat_index + model->mat_count] = SDL_LoadBMP(filepath_copy);

      // load normal map
      // strcpy(filepath_copy, filepath);
      // strcat(filepath_copy, "/");
      // strcat(filepath_copy, token);
      // strcat(filepath_copy, "nmap");
      // printf("FILE: %s\n", filepath_copy);
      // model->materials[mat_index + model->mat_count] = SDL_LoadBMP(filepath_copy);


      mat_index -= 1;
    }
  }

  free(filepath_copy);
}

Model *model_load(char *filepath)
{
  Model *model = (Model *)calloc(1, sizeof(Model));
  model->textured = 1;
  model->visible = 1;
  model->shade_style = SHADE_NONE;

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

  count_polygons(fh, model);

  model->polygons = (Polygon *)malloc(model->poly_count * sizeof(Polygon)); // Array of polygons
  model->materials = (SDL_Surface **)malloc(model->mat_count*2 * sizeof(SDL_Surface *)); // Array of sdl surfaces

  load_polygons(fh, model);
  fclose(fh);

  FILE *fh2 = fopen(filepath_mtl, "r");
  if (fh2 == NULL)
    printf("Error opening %s\n", filepath_mtl);  
  load_material(fh2, filepath_slash, model);
  fclose(fh2);


  for (int i=0; i<model->poly_count; i++)
  {
    for (int j=0; j<3; j++)
    {
      // model->polygons[i].uvs[j].x = 1 - model->polygons[i].uvs[j].x;
      model->polygons[i].uvs[j].y = 1 - model->polygons[i].uvs[j].y;
      model->polygons[i].uvs[j].x *= model->materials[model->polygons[i].mat_index]->w;
      model->polygons[i].uvs[j].y *= model->materials[model->polygons[i].mat_index]->h;
    }
  }

  free(filepath_obj);
  free(filepath_mtl);
  free(filepath_slash);

  return model;
}
