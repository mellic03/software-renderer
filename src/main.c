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
  cam.pos.x = 0;
  cam.pos.z = -50;

  // Load model
  //------------------------------------------------------------
  Model monkey = load_model("./monkey.obj");
  monkey.fill = (Vector3){50, 100, 50};
  monkey.stroke = (Vector3){50, 100, 50};

  Model plane = load_model("./plane.obj");
  plane.fill = (Vector3){155, 155, 155};
  translate_model(&plane, 0, -5, 0);

  Model cube1 = load_model("./cube.obj");
  cube1.fill = (Vector3){200, 100, 50};

  Model cube2 = load_model("./cube.obj");
  cube2.fill = (Vector3){55, 200, 50};
  //------------------------------------------------------------
  translate_model(&cube1, cube1.pos.x, cube1.pos.y, cube1.pos.z);
  translate_model(&cube2, cube2.pos.x, cube2.pos.y, cube2.pos.z);

  SDL_SetRelativeMouseMode(SDL_TRUE);
  SDL_Event event; // no need for new/delete, stack is fine

  window_texture = SDL_CreateTexture(ren, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);

  while (1)
  {
    input_handler(event, &cam);
    
    // Render loop
    //----------------------------------------------
    for (int i=0; i<SCREEN_WIDTH; i++)
    {
      for (int j=0; j<SCREEN_HEIGHT; j++)
      {
        pixels[3*SCREEN_WIDTH*j + 3*i + 0] = 109;
        pixels[3*SCREEN_WIDTH*j + 3*i + 1] = 133;
        pixels[3*SCREEN_WIDTH*j + 3*i + 2] = 169;
      }
    }

    draw_model(ren, cam, &plane);
    draw_model(ren, cam, &cube1);


    // printf("left: %.2f %.2f %.2f, right : %.2f %.2f %.2f, top: %.2f %.2f %.2f, bot: %.2f %.2f %.2f\n",
    // cam.left_normal.x, cam.left_normal.y, cam.left_normal.z,
    // cam.right_normal.x, cam.right_normal.y, cam.right_normal.z,
    // cam.top_normal.x, cam.top_normal.y, cam.top_normal.z,
    // cam.bot_normal.x, cam.bot_normal.y, cam.bot_normal.z);

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


