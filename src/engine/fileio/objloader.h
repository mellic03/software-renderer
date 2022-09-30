#ifndef OBJLOADER_H
#define OBJLOADER_H

#ifdef __unix__
  #include <SDL2/SDL.h>
#elif defined(_WIN32) || defined(WIN32)
  #define SDL_MAIN_HANDLED
  #include <SDL.h>
#endif

#include "../graphics/engine.h"


Model load_model(char *filepath);

#endif /* OBJLOADER_H */
