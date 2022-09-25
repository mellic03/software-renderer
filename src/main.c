#include <math.h>
#include <pthread.h>
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#include "engine/graphics/engine.h"
#include "engine/input.h"
#include "engine/screen.h"


int main(int argc, char** argv)
{
  // SDL setup
  //-------------------------------------------------------
  if (SDL_Init(SDL_INIT_VIDEO) != 0)
  {
    printf("Error\n");
    return 1;
  }

  SDL_Window* win = SDL_CreateWindow(
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
  SDL_Event event;
  pixel_array = SDL_GetWindowSurface(win);

  Camera cam = create_camera();

  // Load models
  //------------------------------------------------------------
  Model map = load_model("src/assets/map");
  translate_model(&map, 0, 0, 20);
  rotate_y(&map, 3.14);
  // Model sphere = load_model("./sphere.obj", "./sphere.mtl");
  // translate_model(&sphere, 0, 0, 50);

  //------------------------------------------------------------


  // Render loop
  //------------------------------------------------------------

  struct timeval sometime1;
  struct timeval sometime2;
  double framerate;
  int count = 0;

  while (1)
  {
    gettimeofday(&sometime1, NULL);

    // time = clock();
    clear_screen(109, 133, 169);
    input(event, &cam);

    draw_model(cam, &map);


    // if (toggle == 1 && vector3_dist((vector3_add(cam.pos, cam.dir)), cube.pos) < 2)
    // {
    //   Vector3 t = vector3_add(cam.pos, vector3_scale((Vector3){cam.dir.x, -cam.dir.y, cam.dir.z}, 2));
    //   Vector3 dir = vector3_sub(t, cube.pos);
    //   translate_model(&cube, dir.x, dir.y, dir.z);
    // }


    SDL_UpdateWindowSurface(win);

    gettimeofday(&sometime2, NULL);
    if (sometime2.tv_usec < sometime1.tv_usec)
      sometime2.tv_usec += 1000000;
    delta_time = (double)(sometime2.tv_usec - sometime1.tv_usec) / 1000000;

    framerate = 1 / delta_time;
    if (count > 500)
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


