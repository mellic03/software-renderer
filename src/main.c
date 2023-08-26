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

  SDL_Event event;
  pixel_array = SDL_GetWindowSurface(win);

  Camera cam = create_camera();

  // Load models
  //------------------------------------------------------------
  Model floor = load_model("./floor.obj", "./floor.mtl");
  translate_model(&floor, 0, 0, 20);
  rotate_y(&floor, 3.14);

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

    draw_model(cam, &floor);


    // if (toggle == 1 && vector3_dist((vector3_add(cam.pos, cam.dir)), cube.pos) < 2)
    // {
    //   Vector3 t = vector3_add(cam.pos, vector3_scale((Vector3){cam.dir.x, -cam.dir.y, cam.dir.z}, 2));
    //   Vector3 dir = vector3_sub(t, cube.pos);
    //   translate_model(&cube, dir.x, dir.y, dir.z);
    // }


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

  SDL_DestroyWindow(win);
  SDL_Quit();

  return 0;
}


