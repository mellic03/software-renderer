#ifndef LIGHTING_H
#define LIGHTING_H

#include "../math/vector.h"
#include "model.h"

#define LIGHT_PIXEL_STEP 4 
#define AMBIENT_LIGHT ((Vector3){0.002, 0.002, 0.002})


// LIGHTSOURCE
//----------------------------------------------------
/*
  The lighting system works by running through a linked-list of LightSources
  and performing the relevant light calculations, adding to the total light
  per vertex/fragment until the end of the list is reached.
*/

typedef enum {DIRECTIONAL, POINT, SPOT} LightType;

typedef struct lightsource {
  LightType light_type;
  Vector3 pos, dir, colour;
  float intensity;
  float inner_cutoff, outer_cutoff;
  struct lightsource *next;
} LightSource;

void GE_lightsource_init(LightSource *lightsoure, LightType light_type);
LightSource GE_lightsource_world_to_view(LightSource *lightsource_world);
//----------------------------------------------------


// SHADER
//----------------------------------------------------
/*
  VertexShaders contain a pointer to a function "perform()".
  perform() returns a vector3 which holds the computed light
  values for each vertex of the Polygon passed to it.

  FragmentShaders work similarly but also require the view-space
  position of the fragment.
*/

typedef struct {
  Vector3 (*perform)(Polygon *tri, LightSource lightsource);
} VertexShader;

typedef struct {
  Vector3 (*perform)(Polygon *tri, Vector3 frag_pos, LightSource lightsource);
} FragmentShader;
//----------------------------------------------------


#endif /* LIGHTING_H */