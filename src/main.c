#include <SDL2/SDL.h>
#include <math.h>
#include "engine/engine.h"
#include "engine/input.h"

#define SCREEN_WIDTH 2000
#define SCREEN_HEIGHT 1000
#define FRAMERATE 30

int main(int argc, char** argv)
{
  if (SDL_Init(SDL_INIT_VIDEO) != 0)
  {
    printf("Error\n");
    return 1;
  }

  SDL_Window* win = SDL_CreateWindow(
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

  //Create and init the renderer
  SDL_Renderer* ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_SOFTWARE);
  if (ren == NULL)
  {
    SDL_DestroyWindow( win );
    return 1;
  }

  Uint64 NOW = SDL_GetPerformanceCounter();
  Uint64 LAST = 0;
  double deltaTime = 0;

  int mousex, mousey;

  Camera cam;
  cam.pos = (Vector3){0, 0, -5};
  cam.fov = 1000;

  // Load model
  //------------------------------------------------------------
  Model monkey = load_model("./monkey.obj");
  monkey.pos.z = 20;

  Model monkey2 = load_model("./monkey.obj");
  monkey2.pos.z = 20;
  monkey2.pos.x = 20;
  //------------------------------------------------------------

  SDL_SetRelativeMouseMode(SDL_TRUE);
  SDL_Event event; // no need for new/delete, stack is fine

  while (1)
  {
    input_handler(event, &cam, &mousex, &mousey);

    SDL_PumpEvents();

    SDL_RenderSetLogicalSize(ren, SCREEN_WIDTH, SCREEN_HEIGHT);
    SDL_SetRenderDrawColor(ren, 255, 255, 255, 255); // Set background
    SDL_RenderClear(ren); // Clear screen
    
    // Render loop
    //----------------------------------------------
    SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);

    draw_model(ren, cam, monkey);
    draw_model(ren, cam, monkey2);

    SDL_RenderPresent(ren);
    //----------------------------------------------
  }

  //Clean Up
  SDL_DestroyRenderer(ren);
  SDL_DestroyWindow(win);
  SDL_Quit();

  return 0;
}