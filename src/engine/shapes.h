#include <SDL2/SDL.h>

#include "vector.h"
#include "camera.h"

#ifndef SHAPES_H
#define SHAPES_H

typedef struct Triangle {
  Vector3 p1, p2, p3;
} Triangle;

typedef struct Rect {
  Triangle t1, t2;
} Rect;

void line_2d(SDL_Renderer *renderer, float x1, float y1, float x2, float y2);
void draw_triangle(SDL_Renderer *renderer, int x1, int y1, int x2, int y2, int x3, int y3);
void draw_square(SDL_Renderer *renderer, int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4);
void draw_rect(SDL_Renderer *renderer, int x, int y, int w, int h);
void triangle_3d(SDL_Renderer *renderer, Camera cam, Vector3 p1, Vector3 p2, Vector3 p3);
void line_3d(SDL_Renderer *renderer, Camera cam, Vector3 p1, Vector3 p2);
void cube_3d(SDL_Renderer *renderer, Camera cam, Vector3 pos, int w);

#endif /* SHAPES_H */