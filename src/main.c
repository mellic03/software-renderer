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



struct timeval sometime1, sometime2;
double framerate;
int count = 0;

pthread_t thread_physics;
pthread_mutex_t mutex;
pthread_cond_t main_ready;

Player *player;

void *phys_thread()
{
  while (1)
  {
    pthread_cond_wait(&main_ready, &mutex);
    physics_tick();
    gameobject_tick();
    player_collision(player);
  }
}


int main(int argc, char** argv)
{
  srand(clock());

  // SDL setup
  //-------------------------------------------------------
  SDL_Window *win;
  SDL_Event event;

  if (SDL_Init(SDL_INIT_VIDEO) != 0)
  {
    printf("Error\n");
    return 1;
  }

  win = SDL_CreateWindow(
    "Graphics",
    SDL_WINDOWPOS_UNDEFINED,
    SDL_WINDOWPOS_UNDEFINED,
    SCREEN_WDTH,
    SCREEN_HGHT,
    0
  );

  if (win == NULL)
  {
    printf("Error\n");
    return 1;
  }

  SDL_SetRelativeMouseMode(SDL_TRUE);
  //-------------------------------------------------------



  // Load assets
  //------------------------------------------------------------

  GameObject *enemy = gameobject_create();
  gameobject_assign_model(enemy, model_load("src/assets/enemy"));
  gameobject_translate(enemy, 5, -3, 0);


  GameObject *map = gameobject_create();
  gameobject_assign_model(map, model_load("src/assets/benchmark"));


  // GameObject *sphere = gameobject_create();
  // gameobject_assign_model(sphere, model_load("src/assets/sphere"));
  // gameobject_translate(sphere, 15, -3, 0);



  player = player_create();
  player->game_object = gameobject_create();
  player->cam->pos = &player->game_object->phys_object->pos;
  gameobject_translate(player->game_object, 0, -5, 0);
  player->game_object->phys_object->mass = 1;
  player->game_object->phys_object->inv_mass = 1/player->game_object->phys_object->mass;
  player->game_object->phys_object->elasticity = 0;
  player->game_object->phys_object->vel.x = 0.1;
  player->game_object->phys_object->vel.z = 0.1;
  GE_cam = player->cam;
  //------------------------------------------------------------

  pthread_mutex_init(&mutex, NULL);
  pthread_cond_init(&main_ready, NULL);

  pthread_create(&thread_physics, NULL, phys_thread, NULL);
  pthread_detach(thread_physics);

  // Render loop
  //------------------------------------------------------------

  GE_init(win);

  while (1)
  {
    gettimeofday(&sometime1, NULL);

    input(event, GE_cam, player);

    gameobject_draw_all();
  
    map->model->shade_style = toggle;


    GE_queue_perform_transformation();
    GE_queue_perform_clipping();
    GE_queue_perform_rasterisation();


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
