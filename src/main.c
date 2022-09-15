#include <SDL2/SDL.h>
#include <math.h>
#include <time.h>

#include "engine/engine.h"
#include "engine/input.h"
#include "engine/screen.h"

void pixel(SDL_Texture *texture, int x, int y, int r, int g, int b);

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
  monkey.fill = (Vector3){50, 100, 50};
  monkey.stroke = (Vector3){50, 100, 50};
  rotate_y(monkey, 3.14);

  // Model plane = load_model("./plane.obj");
  // plane.pos.y = -5;
  // Model cube = load_model("./cube.obj");
  // cube.pos.z = 5;
  // cube.fill = (Vector3){200, 100, 50};
  //------------------------------------------------------------

  SDL_SetRelativeMouseMode(SDL_TRUE);
  SDL_Event event; // no need for new/delete, stack is fine

  window_texture = SDL_CreateTexture(ren, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);


  while (1)
  {
    input_handler(event, &cam);
    
    // Render loop
    //----------------------------------------------
    for (int i=0; i<SCREEN_WIDTH; i++)
    {
      for (int j=0; j<SCREEN_WIDTH; j++)
      {
        pixels[4*SCREEN_WIDTH*j + 4*i + 0] = 0;
        pixels[4*SCREEN_WIDTH*j + 4*i + 1] = 0;
        pixels[4*SCREEN_WIDTH*j + 4*i + 2] = 0;
        pixels[4*SCREEN_WIDTH*j + 4*i + 3] = 255;
      }
    }

    // draw_model(ren, cam, plane);
    draw_model(ren, cam, monkey);
    // draw_model(ren, cam, cube);




    int texture_pitch = 0;
    void *texture_pixels = NULL;
    if (SDL_LockTexture(window_texture, NULL, &texture_pixels, &texture_pitch) != 0)
        SDL_Log("Unable to lock texture: %s", SDL_GetError());
    else
      memcpy(texture_pixels, pixels, texture_pitch * SCREEN_HEIGHT);
    SDL_UnlockTexture(window_texture);
    SDL_RenderClear(ren);
    SDL_RenderCopy(ren, window_texture, NULL, NULL);
    SDL_RenderPresent(ren);
    //----------------------------------------------
  }

  //Clean Up
  SDL_DestroyRenderer(ren);
  SDL_DestroyWindow(win);
  SDL_Quit();

  return 0;
}


