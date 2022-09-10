#include <SDL2/SDL.h>
#include <math.h>
#include "engine/engine.h"

#define SCREEN_WIDTH 1000
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

  SDL_Event evt;    // no need for new/delete, stack is fine

  Uint64 NOW = SDL_GetPerformanceCounter();
  Uint64 LAST = 0;
  double deltaTime = 0;

  int mousex, mousey;

  Camera cam;
  cam.pos = (Vector3){-10, -10, -10};


  // Load model
  //------------------------------------------------------------
  int triangle_count = 0;
  Vector3 *vertices = load_vertices(&triangle_count, "./cube.obj");
  printf("tcount = %d\n", triangle_count);

  int **triangles = (int **)malloc(triangle_count * sizeof(int **));
  for (int i=0; i<triangle_count; i++)
    triangles[i] = (int *)malloc(3 * sizeof(int));
  
  load_vertex_order(triangle_count, triangles, "./cube.obj");


  printf("YEP %f %f %f\n", vertices[triangles[1][1]].x, vertices[triangles[1][1]].y, vertices[triangles[1][1]].z);

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
            cam.R.y -= 1;
            break;
          case SDLK_e:
            cam.R.y += 1;
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

    
    for (int i=0; i<triangle_count; i+=1)
    {
      line_3d(ren, cam, vertices[triangles[i][0]], vertices[triangles[i][1]]);
      line_3d(ren, cam, vertices[triangles[i][1]], vertices[triangles[i][2]]);
      line_3d(ren, cam, vertices[triangles[i][2]], vertices[triangles[i][0]]);
    }

    SDL_RenderPresent(ren);
    //----------------------------------------------
  }

  //Clean Up
  SDL_DestroyRenderer(ren);
  SDL_DestroyWindow(win);
  SDL_Quit();

  return 0;
}