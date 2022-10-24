#ifdef __unix__
  #include <SDL2/SDL.h>
#elif defined(_WIN32) || defined(WIN32)
  #define SDL_MAIN_HANDLED
  #include <SDL.h>
#endif
#include "model.h"


void GE_material_init(Material *material)
{
  material = (Material *)calloc(1, sizeof(Material));
}

void model_free(Model *model)
{
  free(model->polygons);

  free(model->vertices);
  free(model->vertex_normals);
  for (int i=0; i<model->mat_count*2; i++)
    SDL_FreeSurface(model->textures[i]);
  free(model->textures);
  free(model);
}