#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <stdbool.h>

#include "vector.h"
#include "camera.h"
#include "engine.h"
#include "screen.h"

SDL_Surface *pixel_array;
float z_buffer[SCREEN_WIDTH * SCREEN_HEIGHT];

Vector3 lightsource = {0, -100, -100};

double delta_time;

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
    }
}

void set_position_model(Model *model, Vector3 pos)
{

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
      float coord[3][1] = {{model->polygons[i].vertices[j].x}, {model->polygons[i].vertices[j].y}, {model->polygons[i].vertices[j].z}};
      matrix_mult(3, 3, 3, 1, result, rot_x, coord);
      model->polygons[i].vertices[j].x = result[0][0];
      model->polygons[i].vertices[j].y = result[1][0];
      model->polygons[i].vertices[j].z = result[2][0];
    }

    // rotate normals
    float coord[3][1] = {{model->polygons[i].normal_vector.x}, {model->polygons[i].normal_vector.y}, {model->polygons[i].normal_vector.z}};
    matrix_mult(3, 3, 3, 1, result, rot_x, coord);
    model->polygons[i].normal_vector.x = result[0][0];
    model->polygons[i].normal_vector.y = result[1][0];
    model->polygons[i].normal_vector.z = result[2][0];
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
      float coord[3][1] = {{model->polygons[i].vertices[j].x}, {model->polygons[i].vertices[j].y}, {model->polygons[i].vertices[j].z}};
      matrix_mult(3, 3, 3, 1, result, rot_y, coord);
      model->polygons[i].vertices[j].x = result[0][0];
      model->polygons[i].vertices[j].y = result[1][0];
      model->polygons[i].vertices[j].z = result[2][0];
    }

    // rotate normals
    float coord[3][1] = {{model->polygons[i].normal_vector.x}, {model->polygons[i].normal_vector.y}, {model->polygons[i].normal_vector.z}};
    matrix_mult(3, 3, 3, 1, result, rot_y, coord);
    model->polygons[i].normal_vector.x = result[0][0];
    model->polygons[i].normal_vector.y = result[1][0];
    model->polygons[i].normal_vector.z = result[2][0];
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
    float coord[3][1] = {{model.polygons[i].normal_vector.x}, {model.polygons[i].normal_vector.y}, {model.polygons[i].normal_vector.z}};
    matrix_mult(3, 3, 3, 1, result, rot_z, coord);
    model.polygons[i].normal_vector.x = result[0][0];
    model.polygons[i].normal_vector.y = result[1][0];
    model.polygons[i].normal_vector.z = result[2][0];
  }
  translate_model(&model, model_pos.x, model_pos.y, model_pos.z);
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

float sign(Vector2 p1, Vector2 *p2, Vector2 *p3)
{
  return (p1.x - p3->x) * (p2->y - p3->y) - (p2->x - p3->x) * (p1.y - p3->y);
}

bool point_in_triangle (Vector2 pt, Vector2 *v1, Vector2 *v2, Vector2 *v3)
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

Vector3 calculate_barycentric(Vector2 pt, Vector2 v1, Vector2 v2, Vector2 v3)
{
  float weight_v1 = ((v2.y-v3.y)*(pt.x-v3.x) + (v3.x-v2.x)*(pt.y-v3.y)) / ((v2.y-v3.y)*(v1.x-v3.x) + (v3.x-v2.x)*(v1.y-v3.y));
  float weight_v2 = ((v3.y-v1.y)*(pt.x-v3.x) + (v1.x-v3.x)*(pt.y-v3.y)) / ((v2.y-v3.y)*(v1.x-v3.x) + (v3.x-v2.x)*(v1.y-v3.y));
  float weight_v3 = 1 - weight_v1 - weight_v2;
  return (Vector3){weight_v1, weight_v2, weight_v3};
}

void triangle_2d(Camera *cam, Polygon *tri, SDL_Surface *texture)
{
  Vector2 v1 = project_coordinate(&tri->vertices[0]);
  Vector2 v2 = project_coordinate(&tri->vertices[1]);
  Vector2 v3 = project_coordinate(&tri->vertices[2]);

  float invu1 = tri->uvs[0].x / v1.w;
  float invv1 = tri->uvs[0].y / v1.w;
  float invz1 = 1/v1.w;

  float invu2 = tri->uvs[1].x / v2.w;
  float invv2 = tri->uvs[1].y / v2.w;
  float invz2 = 1/v2.w;
  
  float invu3 = tri->uvs[2].x / v3.w;
  float invv3 = tri->uvs[2].y / v3.w;
  float invz3 = 1/v3.w;

  // line_2d(tri->fill, (Vector2){v1.x, v1.y, 1}, (Vector2){v2.x, v2.y, 1});
  // line_2d(tri->fill, (Vector2){v2.x, v2.y, 1}, (Vector2){v3.x, v3.y, 1});
  // line_2d(tri->fill, (Vector2){v3.x, v3.y, 1}, (Vector2){v1.x, v1.y, 1});

  Uint8 fill = tri->fill.x;

  int lx = MIN(v1.x, MIN(v2.x, v3.x));
  int hx = MAX(v1.x, MAX(v2.x, v3.x));
  int ly = MIN(v1.y, MIN(v2.y, v3.y));
  int hy = MAX(v1.y, MAX(v2.y, v3.y));

  Uint8 count = 0;

  for (int x=lx; x<=hx; x++)
  {
    for (int y=ly; y<=hy; y++)
    {
      if (point_in_triangle((Vector2){x, y, 1}, &v1, &v2, &v3))
      {
        Vector2 pt = (Vector2){x, y, 1};

        // barycentric
        Vector3 weights = calculate_barycentric(pt, v1, v2, v3);
        float z_index = weights.x*(1/tri->vertices[0].z) + weights.y*(1/tri->vertices[1].z) + weights.z*(1/tri->vertices[2].z);

        if (z_index > z_buffer[SCREEN_WIDTH*y + x])
        {
          z_buffer[SCREEN_WIDTH*y + x] = z_index;

          float uf = weights.x*invu1 + weights.y*invu2 + weights.z*invu3;
          float vf = weights.x*invv1 + weights.y*invv2 + weights.z*invv3;
          float invz = weights.x*invz1 + weights.y*invz2 + weights.z*invz3;

          uf /= invz;
          vf /= invz;
          
          int u = (int)uf;
          int v = (int)vf;

          u %= texture->w, u = abs(u);
          v %= texture->h, v = abs(v);

          Uint8 *blue  = ((Uint8 *)texture->pixels + (v * texture->pitch) + (u * texture->format->BytesPerPixel + 0));
          Uint8 *green = ((Uint8 *)texture->pixels + (v * texture->pitch) + (u * texture->format->BytesPerPixel + 1));
          Uint8 *red   = ((Uint8 *)texture->pixels + (v * texture->pitch) + (u * texture->format->BytesPerPixel + 2));

          pixel(x, y, *red, *green, *blue);
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

void draw_model(Camera cam, Model *model)
{
  int *frontface_indices = (int *)calloc(model->poly_count, sizeof(int));
  int frontface_count = 0;

  for (int i=0; i<model->poly_count; i++)
    if (vector3_dot(vector3_sub(model->polygons[i].vertices[0], cam.pos), model->polygons[i].normal_vector) < 0)
      frontface_indices[frontface_count++] = i;
  
  Polygon *front_faces = (Polygon *)calloc(frontface_count, sizeof(Polygon));
  for (int i=0; i<frontface_count; i++)
    front_faces[i] = model->polygons[frontface_indices[i]];

  for (int i=0; i<frontface_count; i++)
  {
    for (int j=0; j<3; j++)
    {
      front_faces[i].vertices[j].x -= cam.pos.x;
      front_faces[i].vertices[j].y -= cam.pos.y;
      front_faces[i].vertices[j].z -= cam.pos.z;
      rotate_point(&front_faces[i].vertices[j], 0, cam.rot.y, 0);
      rotate_point(&front_faces[i].vertices[j], cam.rot.x, 0, 0);
    }
  }

  int clipped_count;
  Polygon *clipped_polygons = clip_against_planes(&cam, frontface_count, front_faces, &clipped_count);

  for (int i=0; i<clipped_count; i++)
    triangle_2d(&cam, &clipped_polygons[i], model->materials[clipped_polygons[i].mat_index]);

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

  return (Vector2){canvas_x, canvas_y, pt->z};
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

void load_polygons(FILE *fh, Model model, Polygon *polygons)
{
  char buffer[64];

  char space[] = " ";
  char slash[] = "/";
  char *token;

  Vector3 *vertices = (Vector3 *)calloc(model.vertex_count, sizeof(Vector3));
  int vertex_index = 0;

  Vector3 *normals = (Vector3 *)calloc(model.normal_count, sizeof(Vector3));
  int normal_index = 0;

  Vector2 *tex_coords = (Vector2 *)calloc(model.uv_count, sizeof(Vector2));
  int tex_coord_index = 0;

  char **mat_names = (char **)malloc(model.mat_count * sizeof(char *));
  for (int i=0; i<model.mat_count; i++)
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
        polygons[polygon_index].uvs[i] = tex_coords[temp[1]-1];
      }
      polygons[polygon_index].normal_vector =normals[temp[2]-1];
      polygons[polygon_index].mat_index = mat_index;
      polygon_index += 1;
    }

    // line with new material
    else if (buffer[0] == 'u' && buffer[1] == 's')
    {
      mat_index += 1;
    }

  }

  free(vertices);
  free(normals);
  free(tex_coords);
  free(mat_names);
}

void load_material(FILE *fh, Model *model)
{
  char space[] = " ";
  char buffer[64];
  int mat_index = model->mat_count-1;
  while (fgets(buffer, 64, fh) != NULL)
  {

    if (buffer[0] == 'm' && buffer[1] == 'a') // filepath to texture
    {
      char *token = strtok(buffer, space);
      token = strtok(NULL, space);
      for (size_t i=0; i<strlen(token); i++)
        if (token[i] == '\n')
          token[i] = '\0';

      model->materials[mat_index] = SDL_LoadBMP(token);
      mat_index -= 1;
      printf("FILE: %s\n", token);
    }
  }
}

/** Load an obj file
 */
Model load_model(char *filepath, char *material)
{
  Model model;
  model.pos = (Vector3){0, 0, 0};
  model.normal_count = 0;
  model.poly_count = 0;
  model.uv_count = 0;
  model.vertex_count = 0;

  FILE *fh = fopen(filepath, "r");
  if (fh == NULL)
    printf("Error opening %s\n", filepath);

  count_polygons(fh, &model);

  model.polygons = (Polygon *)calloc(model.poly_count, sizeof(Polygon)); // Array of polygons
  model.materials = (SDL_Surface **)malloc(model.mat_count * sizeof(SDL_Surface *)); // Array of sdl surfaces

  load_polygons(fh, model, model.polygons);
  fclose(fh);

  FILE *fh2 = fopen(material, "r");
  if (fh2 == NULL)
    printf("Error opening %s\n", material);
  load_material(fh2, &model);
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

