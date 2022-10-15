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

#include "../sockets/client.h"

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
int player_id;
playerpositions server_players;
playerpositions server_players_last_frame;

void *phys_thread()
{
  clock_t t;

  while (1)
  {
    pthread_cond_wait(&main_ready, &mutex);
    physics_tick();
    gameobject_tick();
    player_collision(player);
  }
}

void *socket_thread()
{
  // while (1)
  //   send_position(player_id, player->game_object->pos.x, player->game_object->pos.y, player->game_object->pos.z, player->cam->rot.x, player->cam->rot.y, &server_players);
}

int main(int argc, char** argv)
{
  for (int i=0; i<10; i++)
  {
    server_players.x_positions[i] = 0;
    server_players.y_positions[i] = 0;
    server_players.z_positions[i] = 0;
  }

  srand(clock());
  player_id = rand()%1000000;

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

  GameObject *map = gameobject_create();
  gameobject_assign_model(map, model_load("src/assets/benchmark"));
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
  graphicsengine_cam = player->cam;


  
  //------------------------------------------------------------

  pthread_mutex_init(&mutex, NULL);
  pthread_mutex_init(&mutex_render_1, NULL);
  pthread_cond_init(&main_ready, NULL);

  pthread_create(&thread_physics, NULL, phys_thread, NULL);
  pthread_detach(thread_physics);

  pthread_create(&thread_socket, NULL, socket_thread, NULL);
  pthread_detach(thread_socket);


  // Render loop
  //------------------------------------------------------------
  while (1)
  {
    gettimeofday(&sometime1, NULL);
    clear_screen(109, 133, 169);

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
