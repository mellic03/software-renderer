#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdbool.h>

#include "shapes.h"
#include "engine.h"

#define SQ(x) ((x)*(x))

bool in_range(float n, float l, float u)
{
  return (n >= l && n <= u) ? true : false;
}

void draw_line(SDL_Renderer *renderer, float x1, float y1, float x2, float y2)
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

void draw_triangle(SDL_Renderer *renderer, int x1, int y1, int x2, int y2, int x3, int y3)
{
  draw_line(renderer, x1, y1, x2, y2);
  draw_line(renderer, x2, y2, x3, y3);
  draw_line(renderer, x3, y3, x1, y1);
}

void draw_square(SDL_Renderer *renderer, int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4)
{
  draw_triangle(renderer, x1, y1, x2, y2, x3, y3);
  draw_triangle(renderer, x1, y1, x4, y4, x3, y3);
}

void draw_rect(SDL_Renderer *renderer, int x, int y, int w, int h)
{
  draw_triangle(renderer, x, y, x+w, y, x+w, y+h);
  draw_triangle(renderer, x, y, x, y+h, x+w, y+h);
}



void line_3d(SDL_Renderer *renderer, Camera cam, Vector3 p1, Vector3 p2)
{
  Vector2 screen_p1 = project_coordinate(cam, p1);
  Vector2 screen_p2 = project_coordinate(cam, p2);

  draw_line(renderer, screen_p1.x, screen_p1.y, screen_p2.x, screen_p2.y);
}


void triangle_3d(SDL_Renderer *renderer, Camera cam, Vector3 p1, Vector3 p2, Vector3 p3)
{

  Vector2 screen_p1 = project_coordinate(cam, p1);
  Vector2 screen_p2 = project_coordinate(cam, p2);
  Vector2 screen_p3 = project_coordinate(cam, p3);


  draw_line(renderer, screen_p1.x, screen_p1.y, screen_p2.x, screen_p2.y);
  draw_line(renderer, screen_p2.x, screen_p2.y, screen_p3.x, screen_p3.y);
  draw_line(renderer, screen_p3.x, screen_p3.y, screen_p1.x, screen_p1.y);

}

