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

struct timeval sometime1, sometime2;
double framerate;
int count = 0;

pthread_t thread_physics;
pthread_t thread_render_1, thread_render_2;
pthread_mutex_t mutex, mutex_render_1, mutex_render_2;
pthread_cond_t main_ready;

Player *player;

void *phys_thread()
{
  clock_t t;

  while (1)
  {
    pthread_cond_wait(&main_ready, &mutex);

    t = clock();
    physics_tick();
    gameobject_tick();
    player_collision(player);

    phys_delta_time = (double)(clock() - t) / CLOCKS_PER_SEC;
  }
}

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

  // Load assets
  //------------------------------------------------------------

  // GameObject *cube = gameobject_create();
  // gameobject_assign_model(cube, model_load("src/assets/cube"));
  // physobject_give_plane_collider(cube->phys_object, (Vector3){0, -1, 0});
  // gameobject_scale(cube, 50, 0.1, 50);
  // gameobject_translate(cube, 0, 10, 0);
  // gameobject_rotate_z(cube, 0.05);

  GameObject *sphere = gameobject_create();
  gameobject_assign_model(sphere, model_load("src/assets/sphere"));
  gameobject_scale(sphere, 1, 1, 1);
  sphere->model->shade_style = SHADE_PHONG;
  gameobject_translate(sphere, 0, 2, 0);

  GameObject *map = gameobject_create();
  gameobject_assign_model(map, model_load("src/assets/benchmark"));
  map->model->shade_style = SHADE_NONE;


  player = player_create();
  player->game_object = gameobject_create();
  player->cam->pos = &player->game_object->phys_object->pos;
  gameobject_translate(player->game_object, 0, 0, -5);
  player->game_object->phys_object->mass = 1;
  player->game_object->phys_object->inv_mass = 1/player->game_object->phys_object->mass;
  player->game_object->phys_object->elasticity = 0;
  graphicsengine_cam = player->cam;
  physobject_give_sphere_collider(player->game_object->phys_object, 2);

  //------------------------------------------------------------

  pthread_mutex_init(&mutex, NULL);
  pthread_mutex_init(&mutex_render_1, NULL);
  pthread_cond_init(&main_ready, NULL);

  pthread_create(&thread_physics, NULL, phys_thread, NULL);
  pthread_detach(thread_physics);

  // Render loop
  //------------------------------------------------------------
  while (1)
  {
    gettimeofday(&sometime1, NULL);
    // clear_screen(109, 133, 169);
    clear_screen(0, 0, 0);

    input(event, graphicsengine_cam, player);

    gameobject_draw_all(graphicsengine_cam);


    pthread_cond_broadcast(&main_ready);

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


