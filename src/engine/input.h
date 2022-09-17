#ifndef INPUT_H
#define INPUT_H

#include <SDL2/SDL.h>
#include "engine.h"

void input(SDL_Event event, Camera *cam)
{
  const Uint8 *state = SDL_GetKeyboardState(NULL);

  camera_pos = (Vector3)cam->pos;

  if (state[SDL_SCANCODE_W])
  {
    cam->pos.z += 0.02 * sin(cam->R.y + 3.1415 / 2);
    cam->pos.x += 0.02 * cos(cam->R.y + 3.1415 / 2);
  }
  else if (state[SDL_SCANCODE_S])
  {
    cam->pos.x -= 0.02 * cos(cam->R.y+3.14/2);
    cam->pos.z -= 0.02 * sin(cam->R.y+3.14/2);
  }
  if (state[SDL_SCANCODE_A])
  {
    cam->pos.x -= 0.02 * cos(cam->R.y);
    cam->pos.z -= 0.02 * sin(cam->R.y);
  }
  else if (state[SDL_SCANCODE_D])
  {
    cam->pos.x += 0.02 * cos(cam->R.y);
    cam->pos.z += 0.02 * sin(cam->R.y);       
  }

  if (state[SDL_SCANCODE_SPACE])
    cam->pos.y -= 0.05;
  else if (state[SDL_SCANCODE_LCTRL])
    cam->pos.y += 0.05;

  if (state[SDL_SCANCODE_LEFTBRACKET])
    cam->fov -= 1;
  else if (state[SDL_SCANCODE_RIGHTBRACKET])
    cam->fov += 1;


  while(SDL_PollEvent(&event))
  {
    if ((event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE) ||
      (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)) 
    {
      exit(0);
    }
    if (event.type == SDL_MOUSEMOTION)
    {
      cam->R.y -= event.motion.xrel * 0.001;
      cam->R.x += event.motion.yrel * 0.001;
      // if (cam->R.x < -0.2)
      //   cam->R.x = -0.199;
      // else if (cam->R.x > 0.2)
      //   cam->R.x = 0.199;
      // printf("%f\n", cam->R.x);
      if (cam->R.x > 6.28)
        cam->R.x -= 6.28;
      else if (cam->R.x < 0)
        cam->R.x += 6.28;
      cam->dir.x = sin(-cam->R.y);
      cam->dir.y = sin(-cam->R.x);
      cam->dir.z = cos(-cam->R.y);
    }
  }
}

#endif /* INPUT_H */