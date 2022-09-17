#include <SDL2/SDL.h>
#include <math.h>
#include <time.h>

#include "engine/engine.h"
#include "engine/input.h"
#include "engine/screen.h"

void pixel(SDL_Texture *texture, int x, int y, int r, int g, int b);

int main(int argc, char** argv)
{
  // SDL setup
  //-------------------------------------------------------
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

  SDL_SetRelativeMouseMode(SDL_TRUE);
  SDL_Event event; // no need for new/delete, stack is fine
  window_texture = SDL_CreateTexture(ren, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
  //-------------------------------------------------------



  Camera cam = create_camera();

  // Load models
  //------------------------------------------------------------
  Model cube = load_model("./cube.obj");
  cube.fill = (Vector3){200, 100, 50};
  translate_model(&cube, 0, 0, 10);

  //------------------------------------------------------------

  // Render loop
  //----------------------------------------------
  while (1)
  {
    clear_screen();
    input(event, &cam);
    


    draw_model(cam, cube);



    printf("%f %f %f\n", cam.dir.x, cam.dir.y, cam.dir.z);




    render_screen(ren);
  }
  //----------------------------------------------

  //Clean Up
  SDL_DestroyRenderer(ren);
  SDL_DestroyWindow(win);
  SDL_Quit();

  return 0;
}


