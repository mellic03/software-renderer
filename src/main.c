#ifdef __unix__
  #include <SDL2/SDL.h>
#elif defined(_WIN32) || defined(WIN32)
  #define SDL_MAIN_HANDLED
  #include <SDL.h>
#endif

#include <math.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <stdio.h>
#include <pthread.h>

#include "engine/engine.h"

SDL_Window *win;
SDL_Event event;

struct timeval sometime1;
struct timeval sometime2;
double framerate;
int count = 0;



int main(int argc, char** argv)
{
  // SDL setup
  //-------------------------------------------------------
  if (SDL_Init(SDL_INIT_VIDEO) != 0)
  {
    printf("Error\n");
    return 1;
  }

  win = SDL_CreateWindow(
    "Graphics",
    SDL_WINDOWPOS_UNDEFINED,
    SDL_WINDOWPOS_UNDEFINED,
    SCREEN_WIDTH,
    SCREEN_HEIGHT,
    0
  );

  if (win == NULL)
  {
    printf("Error\n");
    return 1;
  }

  SDL_SetRelativeMouseMode(SDL_TRUE);

  pixel_array = SDL_GetWindowSurface(win);

  cam = create_camera();
  cam.pos.y = -5;
  cam.pos.z = -10;

  // Load models
  //------------------------------------------------------------
  GameObject *sphere1 = gameobject_create();
  gameobject_assign_model(sphere1, model_load("src/assets/sphere"));
  gameobject_give_sphere_collider(sphere1, 2);
  sphere1->model->shade_style = SHADE_SMOOTH;
  gameobject_translate(sphere1, 0, -10, 0);
  sphere1->mass = 1;

  GameObject *plane = gameobject_create();
  gameobject_assign_model(plane, model_load("src/assets/plane"));
  gameobject_give_plane_collider(plane, (Vector3){0, -1, 0});
  rotate_z(plane->model, 0.2);
  rotate_point(&plane->plane_collider->dir, 0, 0, 0.2);
  //------------------------------------------------------------


  // Render loop
  //------------------------------------------------------------
  while (1)
  {
    gettimeofday(&sometime1, NULL);
    input(event, &cam);
    clear_screen(109, 133, 169);

    gameobject_draw_all(&cam);
    gameobject_tick();



    if (toggle == 1)
    {
      Vector3 dir = (Vector3){0, 10, 0};
      vector3_normalise(&dir);
      dir = vector3_scale(dir, 0.002);
      sphere1->vel = vector3_add(sphere1->vel, dir);
    }

    SDL_UpdateWindowSurface(win);

    gettimeofday(&sometime2, NULL);
    if (sometime2.tv_usec < sometime1.tv_usec)
      sometime2.tv_usec += 1000000;
    delta_time = (double)(sometime2.tv_usec - sometime1.tv_usec) / 1000000;

    framerate = 1 / delta_time;
    if (count > 100)
    {
      count = 0;
      printf("FPS: %lf\n", framerate);
    }
    count++;
  }
  //------------------------------------------------------------

  SDL_DestroyWindow(win);
  SDL_Quit();

  return 0;
}


