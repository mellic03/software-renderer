#include <SDL2/SDL.h>
#include <math.h>
#include <time.h>

#include "engine/engine.h"
#include "engine/input.h"
#include "engine/screen.h"

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

  Camera cam = create_camera();

  // Load model
  //------------------------------------------------------------
  Model monkey = load_model("./monkey.obj");
  monkey.pos.z = 0;
  rotate_y(monkey, 3.14);

  Model plane = load_model("./plane.obj");
  plane.pos.y = -5;
  //------------------------------------------------------------

  SDL_SetRelativeMouseMode(SDL_TRUE);
  SDL_Event event; // no need for new/delete, stack is fine


  while (1)
  {
    input_handler(event, &cam);
    SDL_PumpEvents();
    SDL_RenderSetLogicalSize(ren, SCREEN_WIDTH, SCREEN_HEIGHT);
    SDL_SetRenderDrawColor(ren, 109, 133, 169, 50); // Set background
    SDL_RenderClear(ren); // Clear screen
    
    // Render loop
    //----------------------------------------------
   

    SDL_SetRenderDrawColor(ren, 0, 200, 0, 255);
    draw_model(ren, cam, plane);

    SDL_SetRenderDrawColor(ren, 200, 200, 200, 255);
    draw_model(ren, cam, monkey);


    SDL_RenderPresent(ren);
    //----------------------------------------------
  }

  //Clean Up
  SDL_DestroyRenderer(ren);
  SDL_DestroyWindow(win);
  SDL_Quit();

  return 0;
}

