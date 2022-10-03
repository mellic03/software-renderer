#include <stddef.h>
#include <stdio.h>
#include <math.h>
#include "physics.h"
#include "../math/vector.h"

#define CAP(x, max) (x) > (max) ? (max) : (x)

/** Attract attracted towards attractor. If attractor is NULL attracted will be pulled towards y+.
 */
void physics_attract(Vector3 *attracted_vel, Vector3 *attracted, Vector3 *attractor)
{
  if (attractor == NULL)
  {
    float dist = attracted->y - 10;
    float force = G / dist;
    attracted_vel->y -= force;
  }

  else
  {
    float force = G / vector3_dist(*attracted, *attractor);
    Vector3 dir = vector3_sub(*attractor, *attracted);
    vector3_normalise(&dir);
    dir = vector3_scale(dir, force);

    *attracted_vel = vector3_add(*attracted_vel, dir);
  }

}

/** Return 1 on overlap of two BoxColliders
 */
int box_colliding(BoxCollider *box1, BoxCollider *box2)
{
  int x_overlap = fabs(box1->pos->x - box2->pos->x) < (box1->xwidth + box2->xwidth);  
  int y_overlap = fabs(box1->pos->y - box2->pos->y) < (box1->ywidth + box2->ywidth);  
  int z_overlap = fabs(box1->pos->z - box2->pos->z) < (box1->zwidth + box2->zwidth);  

  if (x_overlap && y_overlap && z_overlap)
    return 1;
  else
    return 0;
}

int sphere_colliding(SphereCollider *sphere1, SphereCollider *sphere2)
{
  if (sphere1 == NULL || sphere2 == NULL)
    return 0;
  float dist = fabs(vector3_dist(*sphere1->pos, *sphere2->pos));
  return dist < sphere1->radius + sphere2->radius;
}

float sphere_plane_colliding(SphereCollider *sphere, PlaneCollider *plane)
{
  if (sphere == NULL || plane == NULL)
    return 0;

  float dist = vector3_dot(plane->dir, vector3_sub(*sphere->pos, *plane->pos));

  // printf("%f\n", dist);

  if (dist < sphere->radius)
    return (dist);
  else
    return 0;
}


// int sphere_plane_colliding(SphereCollider *sphere, PlaneCollider *plane)
// {
//   if (sphere == NULL || plane == NULL)
//     return 0;
//   // Distance between point (x1, y1, z1) and plane (A, B, C), (x0, y0, z0)
//   // float d = -A*x0 - B*y0 -C*z0 
//   Vector3 plane_to_sphere = vector3_sub(*sphere->pos, *plane->pos);
//   float angle = vector3_angle(plane_to_sphere, plane->dir);
//   float dist = vector3_mag(plane_to_sphere) * cos(angle);

//   if (dist < sphere->radius*2)
//     return dist;

//   else
//     return 0;
// }