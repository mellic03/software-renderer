#include <SDL2/SDL.h>
#include "model.h"

void model_free(Model *model)
{
  free(model->polygons);

  free(model->vertices);
  free(model->vertex_normals);
  for (int i=0; i<model->mat_count*2; i++)
    SDL_FreeSurface(model->materials[i]);
  free(model->materials);
  free(model);
}