#include <math.h>
#include "player.h"
#include "../GraphicsEngine/graphics.h"

int toggle = 0;
int speed = 5;
int jumping = 0;

void input(SDL_Event event, Camera *cam, Player *player)
{
  const Uint8 *state = SDL_GetKeyboardState(NULL);

  player->game_object->phys_object->vel.x *= 0.9;
  player->game_object->phys_object->vel.z *= 0.9;

  // 1, 0, 0, x,
  // 0, 1, 0, y,
  // 0, 0, 1, z,
  // 0, 0, 0, 1

  cam->translation_matrix[3] = -player->game_object->pos.x;
  cam->translation_matrix[7] = -player->game_object->pos.y;
  cam->translation_matrix[11] = -player->game_object->pos.z;

  if (state[SDL_SCANCODE_W])
  {
    player->game_object->phys_object->vel.x += cam->dir.x * cam->speed;
    player->game_object->phys_object->vel.z += cam->dir.z * cam->speed;
  }
  else if (state[SDL_SCANCODE_S])
  {
    player->game_object->phys_object->vel.x -= cam->dir.x * cam->speed;
    player->game_object->phys_object->vel.z -= cam->dir.z * cam->speed;
  }

  if (state[SDL_SCANCODE_A])
  {
    player->game_object->phys_object->vel.x -= cam->dir.z * cam->speed;
    player->game_object->phys_object->vel.z += cam->dir.x * cam->speed;
  }
  else if (state[SDL_SCANCODE_D])
  {
    player->game_object->phys_object->vel.x += cam->dir.z * cam->speed;
    player->game_object->phys_object->vel.z -= cam->dir.x * cam->speed;
  }

  if (state[SDL_SCANCODE_LEFT])
    lightsource.z += 0.1;
  if (state[SDL_SCANCODE_RIGHT])
    lightsource.z -= 0.1;

  if (state[SDL_SCANCODE_UP])
    // lightsource.x += 0.1;
    calculate_frustum(cam, cam->vfov-0.005);
  if (state[SDL_SCANCODE_DOWN])
    // lightsource.x -= 0.1;
    calculate_frustum(cam, cam->vfov+0.005);


  while(SDL_PollEvent(&event))
  {
    if ((event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE) ||
      (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)) 
    {
      exit(0);
    }
    if (event.type == SDL_MOUSEMOTION)
    {
      cam->rot.y -= event.motion.xrel * 0.001;
      cam->rot.x += event.motion.yrel * 0.001;
      // if (cam->rot.x < -0.2)
      //   cam->rot.x = -0.199;
      // else if (cam->rot.x > 0.2)
      //   cam->rot.x = 0.199;
      // printf("%f\n", cam->rot.x);
      if (cam->rot.x > 6.28)
        cam->rot.x -= 6.28;
      else if (cam->rot.x < 0)
        cam->rot.x += 6.28;
      cam->dir.x = sin(-cam->rot.y);
      cam->dir.y = sin(cam->rot.x);
      cam->dir.z = cos(-cam->rot.y);
      rotate_point(&player->ray_left, 0, event.motion.xrel * 0.001, 0);
      rotate_point(&player->ray_right, 0, event.motion.xrel * 0.001, 0);
      rotate_point(&player->ray_front, 0, event.motion.xrel * 0.001, 0);
      rotate_point(&player->ray_back, 0, event.motion.xrel * 0.001, 0);
    }

    if (event.type == SDL_KEYDOWN)
      if (event.key.keysym.sym == SDLK_SPACE)
      {
        if (!jumping)
        {
          jumping = 1;
          player->game_object->phys_object->vel.y -= 20;
        }
      }

    if (event.type == SDL_KEYUP)
    {

      if (event.key.keysym.sym == SDLK_SPACE)
      {
        jumping = 0;
      }

      if (event.key.keysym.sym == SDLK_KP_ENTER)
        toggle = !toggle;
    }

    if (event.type == SDL_MOUSEBUTTONDOWN)
    toggle = 1;
    else if (event.type == SDL_MOUSEBUTTONUP)
    toggle = 0;
  }
}