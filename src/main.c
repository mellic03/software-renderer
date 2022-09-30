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

#include "engine/engine.h"

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
  cam.pos = (Vector3){0, 0, -15};

  // Load models
  //------------------------------------------------------------
  // Model cube1 = load_model("src/assets/cube");
  // translate_model(&cube1, -10, 0, 0);

  // Model cube2 = load_model("src/assets/cube");
  // translate_model(&cube2, 10, 0, 0);

  // GameObject *test1 = gameobject_create();
  // test1->model = load_model("src/assets/cube");
  // translate_model(test1->model, -10, 0, 0);

  // GameObject *test2 = gameobject_create();
  // test2->model = load_model("src/assets/cube");
  // translate_model(test2->model, 10, 0, 0);

  GameObject *map = gameobject_create();
  map->model = load_model("src/assets/plane");
  map->model->shade_style = SHADE_SMOOTH;
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
    clear_screen(109, 133, 169);
    input(event, &cam);

    gameobject_draw_all(cam);

    // for (int i=0; i<map.poly_count; i+=1)
    // {
    //   for (int j=0; j<3; j++)
    //   {
    //     float dist = vector3_dist(map.polygons[i].vertices[j], cube.pos);
    //     if (dist < 5)
    //       map.polygons[i].vertices[j].y = sin(dist);
    //   }
    // }

    // if (toggle == 1 && vector3_dist((vector3_add(cam.pos, cam.dir)), cube.pos) < 8)
    // {
    //   Vector3 t = vector3_add(cam.pos, vector3_scale((Vector3){cam.dir.x, cam.dir.y, cam.dir.z}, 7));
    //   Vector3 dir = vector3_sub(t, cube.pos);
    //   translate_model(&cube, dir.x, dir.y, dir.z);
    // }

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


