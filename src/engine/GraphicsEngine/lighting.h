#ifndef LIGHTING_H
#define LIGHTING_H

#include "../math/enginemath.h"
#include "model.h"

#define LIGHT_PIXEL_STEP 5 
#define AMBIENT_LIGHT ((Vector3){0.002, 0.002, 0.002})

struct lightsource;
typedef struct lightsource LightSource;

// LIGHTSOURCE
//----------------------------------------------------
/*
  The lighting system works by running through a linked-list of LightSources
  and performing the relevant light calculations, adding to the total light
  per vertex/fragment until the end of the list is reached.
*/
typedef Vector3 (*Shader)(Polygon *, Vector3, LightSource);
typedef enum {GE_DIRLIGHT, GE_POINTLIGHT, GE_SPOTLIGHT} LightType;

typedef struct lightsource {
  
  LightType light_type;
  Vector3 pos, pos_viewspace;
  Vector3 dir, dir_viewspace;
  Vector3 colour;
  float intensity;
  float inner_cutoff, outer_cutoff;

  Shader frag_shader;
  Shader vert_shader;

  struct lightsource *next;

} LightSource;

LightSource *GE_lightsource_init(LightType light_type);
LightSource *GE_lightsource_create(LightType light_type);
Vector3 GE_lightsource_perform_fragment(Polygon *tri, Vector3 frag_pos);

void GE_lightsource_world_to_view(LightSource *lightsource);
void GE_lightsource_world_to_view_all(void);
//----------------------------------------------------



#endif /* LIGHTING_H */