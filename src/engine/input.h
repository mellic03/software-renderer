#ifndef INPUT_H
#define INPUT_H

#include <SDL2/SDL.h>
#include "engine.h"

int toggle = 0;
float something = 0.0;


void input(SDL_Event event, Camera *cam)
{
  const Uint8 *state = SDL_GetKeyboardState(NULL);

  cam->pos = vector3_add(cam->pos, cam->vel);
  cam->vel = vector3_scale(cam->vel, 0.9);

  // cam->pos.y = 0.2*(sin(something)) + height;
  // something += cam->vel.z + cam->vel.x;

  // if (cam->pos.y < -5)
  //   cam->vel.y += delta_time;
  // else if (cam->pos.y > -5)
  //   cam->vel.y -= delta_time;

  if (state[SDL_SCANCODE_W])
    cam->vel = vector3_add(cam->vel, vector3_scale((Vector3){cam->speed * cos(cam->rot.y + 3.1415 / 2), 0, cam->speed * sin(cam->rot.y + 3.1415 / 2)}, delta_time));
  else if (state[SDL_SCANCODE_S])
    cam->vel = vector3_add(cam->vel, vector3_scale((Vector3){-cam->speed * cos(cam->rot.y+3.14/2), 0, -cam->speed * sin(cam->rot.y+3.14/2)}, delta_time));

  if (state[SDL_SCANCODE_A])
    cam->vel = vector3_add(cam->vel, vector3_scale((Vector3){-cam->speed * cos(cam->rot.y), 0, -cam->speed * sin(cam->rot.y)}, delta_time));
  else if (state[SDL_SCANCODE_D])
    cam->vel = vector3_add(cam->vel, vector3_scale((Vector3){cam->speed * cos(cam->rot.y), 0, cam->speed * sin(cam->rot.y)}, delta_time));

  if (state[SDL_SCANCODE_SPACE])
    cam->vel = vector3_add(cam->vel, (Vector3){0, -delta_time, 0});
  if (state[SDL_SCANCODE_LCTRL])
    cam->vel = vector3_add(cam->vel, (Vector3){0, delta_time, 0});

  if (state[SDL_SCANCODE_Q])
    cam->rot.z -= 0.01;



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
      cam->dir.y = sin(-cam->rot.x);
      cam->dir.z = cos(-cam->rot.y);
    }
    
    if (event.type == SDL_MOUSEBUTTONDOWN)
      toggle = 1;
    else if (event.type == SDL_MOUSEBUTTONUP)
      toggle = 0;
  }
}

#endif /* INPUT_H */