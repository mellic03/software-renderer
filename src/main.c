#include <SDL2/SDL.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#include "engine/engine.h"
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
  SDL_Event event; // no need for new/delete, stack is fine

  pixel_array = SDL_GetWindowSurface(win);


  Camera cam = create_camera();

  // Load models
  //------------------------------------------------------------
  Model cube = load_model("./cube.obj", "./cube.mtl");
  // fill_model(&cube, 150, 120, 75);
  translate_model(&cube, 0, -2, 0);

  Model plane = load_model("./plane.obj", "./plane.mtl");
  fill_model(&plane, 150, 120, 75);

  //------------------------------------------------------------

  // Render loop
  //------------------------------------------------------------
  clock_t time;
  double framerate;

  int count = 0;
  while (1)
  {
    time = clock();

    clear_screen(109, 133, 169);
    input(event, &cam);

    // draw_model(cam, &plane);
    draw_model(cam, &cube);

    cam.pos = vector3_add(cam.pos, cam.vel);
    cam.vel = vector3_scale(cam.vel, 0.7);

    SDL_UpdateWindowSurface(win);
    
    framerate = 1 / ((double)(clock() - time) / CLOCKS_PER_SEC);
    if (count > 500)
    {
      count = 0;
      printf("FPS: %lf\n", framerate);
    }
    delta_time =  1/framerate;
    count++;
  }
  //------------------------------------------------------------

  //Clean Up
  SDL_DestroyWindow(win);
  SDL_Quit();

  return 0;
}


