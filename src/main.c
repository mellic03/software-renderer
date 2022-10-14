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
pthread_t thread_socket;
pthread_mutex_t mutex, mutex_render_1, mutex_render_2;
pthread_cond_t main_ready;

Player *player;

void *phys_thread()
{
  clock_t t;

  while (1)
  {
    pthread_cond_wait(&main_ready, &mutex);

    // physics_tick();
    gameobject_tick();
    player_collision(player);
  }
}

int main(int argc, char** argv)
{
  srand(clock());

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

  precomputed_sine = precompute_sine();

  SDL_SetRelativeMouseMode(SDL_TRUE);
  pixel_array = SDL_GetWindowSurface(win);

  GE_transform_queue = RSR_queue_create();
  GE_clip_queue = RSR_queue_create();
  GE_rasterise_queue = RSR_queue_create();

  front_faces = (Polygon *)calloc(1, sizeof(Polygon));

  // Load assets
  //------------------------------------------------------------
  GameObject *map = gameobject_create();
  gameobject_assign_model(map, model_load("src/assets/enemy"));
  map->model->shade_style = SHADE_NONE;

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
  pthread_mutex_init(&mutex_render_1, NULL);
  pthread_cond_init(&main_ready, NULL);

  pthread_create(&thread_physics, NULL, phys_thread, NULL);
  pthread_detach(thread_physics);

  // Render loop
  //------------------------------------------------------------
  while (1)
  {
    gettimeofday(&sometime1, NULL);
    clear_screen(109, 133, 169);
    // clear_screen(0, 0, 0);

    input(event, GE_cam, player);

    gameobject_draw_all(GE_cam);
  
    map->model->shade_style = toggle;


    GE_queue_rotate();
    GE_queue_clip();
    GE_queue_rasterise();


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
