#ifndef LIGHTING_H
#define LIGHTING_H

#include "../math/enginemath.h"
#include "model.h"

#define LIGHT_PIXEL_STEP 4 
#define AMBIENT_LIGHT ((Vector3){0.02, 0.02, 0.02})

struct lightsource;
typedef struct lightsource LightSource;

extern LightSource *lightsource_head;

/*
  The lighting system works by running through a linked-list of LightSources
  and performing the relevant light calculations, adding to the total light
  per vertex/fragment until the end of the list is reached.

  Each LightSource contains a pointer to both a vertex and fragment shader
  in the form of a function.
*/
typedef Vector3 (*VertexShader)(Polygon *, Vector3, LightSource);
typedef Vector3 (*FragmentShader)(Polygon *, Vector3, LightSource);
typedef enum {GE_DIRLIGHT, GE_POINTLIGHT, GE_SPOTLIGHT} LightType;

typedef struct lightsource {
  
  LightType light_type;
  Vector3 pos, pos_viewspace;
  Vector3 dir, dir_viewspace;
  Vector3 colour;
  float intensity;
  float inner_cutoff, outer_cutoff;

  VertexShader vert_shader;
  FragmentShader frag_shader;

  struct lightsource *next;

} LightSource;

LightSource *GE_lightsource_init(LightType light_type);
LightSource *GE_lightsource_create(LightType light_type);
Vector3 GE_lightsource_perform_fragment(Polygon *tri, Vector3 frag_pos);
void GE_lightsource_perform_vertex(Polygon *tri, Vector3 *out1, Vector3 *out2, Vector3 *out3);

void GE_lightsource_world_to_view(LightSource *lightsource);
void GE_lightsource_world_to_view_all(void);



#endif /* LIGHTING_H */