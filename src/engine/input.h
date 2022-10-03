#ifndef INPUT_H
#define INPUT_H

#ifdef __unix__
  #include <SDL2/SDL.h>
#elif defined(_WIN32) || defined(WIN32)
  #include <SDL.h>
#endif
#include "GraphicsEngine/graphics.h"

extern int toggle;

void input(SDL_Event event, Camera *cam);


#endif /* INPUT_H */