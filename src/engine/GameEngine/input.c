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

  if (state[SDL_SCANCODE_W])
  {
    player->game_object->phys_object->vel.x -= sin(cam->rot.y) * cam->speed;
    player->game_object->phys_object->vel.z += cos(cam->rot.y) * cam->speed;
  }
  else if (state[SDL_SCANCODE_S])
  {
    player->game_object->phys_object->vel.x += sin(cam->rot.y) * cam->speed;
    player->game_object->phys_object->vel.z -= cos(cam->rot.y) * cam->speed;
  }

  if (state[SDL_SCANCODE_A])
  {
    player->game_object->phys_object->vel.x -= cos(cam->rot.y) * cam->speed;
    player->game_object->phys_object->vel.z -= sin(cam->rot.y) * cam->speed;
  }
  else if (state[SDL_SCANCODE_D])
  {
    player->game_object->phys_object->vel.x += cos(cam->rot.y) * cam->speed;
    player->game_object->phys_object->vel.z += sin(cam->rot.y) * cam->speed;
  }

  if (state[SDL_SCANCODE_LEFT])
  {
    vector3_roty(&lightsource_red.dir, 0.05);
    vector3_roty(&lightsource_blue.dir, 0.05);
    vector3_roty(&lightsource_green.dir, 0.05);

  }
  if (state[SDL_SCANCODE_RIGHT])
  {
    vector3_roty(&lightsource_red.dir, -0.05);
    vector3_roty(&lightsource_blue.dir, -0.05);
    vector3_roty(&lightsource_green.dir, -0.05);
  }

  // if (state[SDL_SCANCODE_UP])
  //   lightsource.pos.x += 0.1; 
  // if (state[SDL_SCANCODE_DOWN])
  //   lightsource.pos.x -= 0.1;
  // if (state[SDL_SCANCODE_PAGEUP])
  //   lightsource.pos.y -= 0.1;
  // if (state[SDL_SCANCODE_PAGEDOWN])
  //   lightsource.pos.y += 0.1;

  if (state[SDL_SCANCODE_KP_PLUS])
    calculate_frustum(cam, cam->vfov-0.005);
  if (state[SDL_SCANCODE_KP_MINUS])
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
    
      if (cam->rot.x <= -1.5)
        cam->rot.x = -1.499;
      else if (cam->rot.x >= 1.5)
        cam->rot.x = 1.499;

      cam->dir.x = -sin(cam->rot.y)*cos(cam->rot.x);
      cam->dir.y = sin(cam->rot.x);
      cam->dir.z = cos(cam->rot.y)*cos(cam->rot.x);
      
      vector3_normalise(&cam->dir);

      vector3_roty(&player->ray_left, event.motion.xrel * 0.001);
      vector3_roty(&player->ray_right, event.motion.xrel * 0.001);
      vector3_roty(&player->ray_front, event.motion.xrel * 0.001);
      vector3_roty(&player->ray_back, event.motion.xrel * 0.001);
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