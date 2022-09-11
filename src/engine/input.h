#ifndef INPUT_H
#define INPUT_H

#include <SDL2/SDL.h>
#include "engine.h"

void input_handler(SDL_Event event, Camera *cam, int *mousex, int *mousey)
{

  SDL_GetMouseState(mousex, mousey);

  const Uint8 *state = SDL_GetKeyboardState(NULL);
  if (state[SDL_SCANCODE_W])
  {
    cam->pos.x -= 0.05 * cos(cam->R.y+3.14/2);
    cam->pos.z -= 0.05 * sin(cam->R.y+3.14/2);
  }
  else if (state[SDL_SCANCODE_S])
  {
    cam->pos.x += 0.05 * cos(cam->R.y+3.14/2);
    cam->pos.z += 0.05 * sin(cam->R.y+3.14/2);
  }
  if (state[SDL_SCANCODE_A])
  {
    cam->pos.x += 0.05 * cos(cam->R.y);
    cam->pos.z += 0.05 * sin(cam->R.y);
  }
  else if (state[SDL_SCANCODE_D])
  {
    cam->pos.x -= 0.05 * cos(cam->R.y);
    cam->pos.z -= 0.05 * sin(cam->R.y);       
  }
  if (state[SDL_SCANCODE_SPACE])
  {
    cam->pos.y += 0.01;
  }
  else if (state[SDL_SCANCODE_LCTRL])
  {
    cam->pos.y -= 0.01;
  }
  if (state[SDL_SCANCODE_LEFTBRACKET])
  {
    cam->fov -= 100;
  }
  else if (state[SDL_SCANCODE_RIGHTBRACKET])
  {
    cam->fov += 100;
  }

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
    }
  }
}

#endif /* INPUT_H */