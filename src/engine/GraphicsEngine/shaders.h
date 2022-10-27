#ifndef SHADERS_H
#define SHADERS_H

#include "model.h"
#include "../math/enginemath.h"
#include "lighting.h"


Vector3 shade_phong_spotlight(Polygon *tri, Vector3 frag_pos, LightSource lightsource);
Vector3 shade_phong_pointlight(Polygon *tri, Vector3 frag_pos, LightSource lightsource);


#endif /* SHADERS_H */