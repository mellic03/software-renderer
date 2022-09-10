#include <SDL2/SDL.h>
#include <math.h>
#include "engine/engine.h"

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
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

  SDL_Event evt;    // no need for new/delete, stack is fine

  Uint64 NOW = SDL_GetPerformanceCounter();
  Uint64 LAST = 0;
  double deltaTime = 0;

  int mousex, mousey;

  Camera cam;
  cam.pos = (Vector3){0, 0, -5};


  // Load model
  //------------------------------------------------------------
  Model model1 = load_model("./monkey.obj"); 
  //------------------------------------------------------------

  while (1)
  {
    SDL_GetMouseState(&mousex, &mousey);

    // event loop and draw loop are separate things, don't mix them
    while(SDL_PollEvent(&evt))
    {
      if ((evt.type == SDL_WINDOWEVENT && evt.window.event == SDL_WINDOWEVENT_CLOSE) ||
        (evt.type == SDL_KEYDOWN && evt.key.keysym.sym == SDLK_ESCAPE)) 
      {
        exit(0);
      }
      
      if (evt.type == SDL_KEYDOWN)
        switch (evt.key.keysym.sym)
        {
          case SDLK_RIGHT:
            cam.pos.x += sin(cam.R.y+3.14/2);
            cam.pos.z += cos(cam.R.y+3.14/2);
            break;
          case SDLK_LEFT:
            cam.pos.x -= sin(cam.R.y+3.14/2);
            cam.pos.z -= cos(cam.R.y+3.14/2);
            break;
          case SDLK_UP:
            cam.pos.x += sin(cam.R.y);
            cam.pos.z += cos(cam.R.y);
            break;
          case SDLK_DOWN:
            cam.pos.x -= sin(cam.R.y);
            cam.pos.z -= cos(cam.R.y);
            break;
          case SDLK_PAGEUP:
            cam.pos.y -= 1;
            break;
          case SDLK_PAGEDOWN:
            cam.pos.y += 1;
            break;
          case SDLK_q:
            cam.R.y -= 0.04;
            break;
          case SDLK_e:
            cam.R.y += 0.04;
            break;
        }
    }


    SDL_PumpEvents();

    //Render something
    SDL_RenderSetLogicalSize(ren, SCREEN_WIDTH, SCREEN_HEIGHT);
    
    SDL_SetRenderDrawColor(ren, 0, 0, 0, 255); // Set background
    SDL_RenderClear(ren); // Clear screen
    
    // Render loop
    //----------------------------------------------
    SDL_SetRenderDrawColor(ren, 0, 255, 0, 255);

    draw_model(ren, cam, model1);
    
    // Vector3 p1 = {1, 1, -1};
    // Vector3 p2 = {1, 0, -1};
    // Vector3 p3 = {1, 1, 0};
    // Vector3 p4 = {1, 0, 0};
    // Vector3 p5 = {0, 1, -1};
    // Vector3 p6 = {0, 0, -1};
    // Vector3 p7 = {0, 1, 0};
    // Vector3 p8 = {0, 0, 0};

    // triangle_3d(ren, cam, p5, p3, p1);
    // triangle_3d(ren, cam, p3, p8, p4);
    // triangle_3d(ren, cam, p7, p6, p8);
    // triangle_3d(ren, cam, p2, p8, p6);
    // triangle_3d(ren, cam, p1, p4, p2);
    // triangle_3d(ren, cam, p5, p2, p6);
    // triangle_3d(ren, cam, p3, p7, p8);
    // triangle_3d(ren, cam, p7, p5, p6);
    // triangle_3d(ren, cam, p2, p4, p8);
    // triangle_3d(ren, cam, p1, p3, p4);
    // triangle_3d(ren, cam, p5, p1, p2);


    SDL_RenderPresent(ren);
    //----------------------------------------------
  }

  //Clean Up
  SDL_DestroyRenderer(ren);
  SDL_DestroyWindow(win);
  SDL_Quit();

  return 0;
}