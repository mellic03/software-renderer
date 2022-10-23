#include <stdio.h>
#include <stdlib.h>
#include "physics.h"
#include "../engine.h"

void physobject_give_box_collider(PhysObject *object, float x, float y, float z)
{
  object->box_collider = (BoxCollider *)calloc(1, sizeof(BoxCollider));
  object->box_collider->planes = (PlaneCollider **)calloc(6, sizeof(PlaneCollider *));

  for (int i=0; i<6; i++)
  {
    object->box_collider->planes[i] = (PlaneCollider *)calloc(1, sizeof(PlaneCollider));
    object->box_collider->plane_pos[i] = object->pos;
  }

  object->box_collider->plane_pos[0].x += x;
  object->box_collider->planes[0]->dir = (Vector3){1, 0, 0};
  object->box_collider->planes[0]->pos = &object->box_collider->plane_pos[0];
  object->box_collider->plane_pos[1].x -= x;
  object->box_collider->planes[1]->dir = (Vector3){-1, 0, 0};
  object->box_collider->planes[1]->pos = &object->box_collider->plane_pos[1];

  object->box_collider->plane_pos[2].y += y;
  object->box_collider->planes[2]->dir = (Vector3){0, 1, 0};
  object->box_collider->planes[2]->pos = &object->box_collider->plane_pos[2];
  object->box_collider->plane_pos[3].y -= y;
  object->box_collider->planes[3]->dir = (Vector3){0, -1, 0};
  object->box_collider->planes[3]->pos = &object->box_collider->plane_pos[3];

  object->box_collider->plane_pos[4].z += z;
  object->box_collider->planes[4]->dir = (Vector3){0, 0, 1};
  object->box_collider->planes[4]->pos = &object->box_collider->plane_pos[4];
  object->box_collider->plane_pos[5].z -= z;
  object->box_collider->planes[5]->dir = (Vector3){0, 0, -1};
  object->box_collider->planes[5]->pos = &object->box_collider->plane_pos[5];
}

void physobject_give_sphere_collider(PhysObject *object, float radius)
{
  object->sphere_collider = (SphereCollider *)calloc(1, sizeof(SphereCollider));
  object->sphere_collider->pos = &object->pos;
  object->sphere_collider->radius = radius;
}

void physobject_give_plane_collider(PhysObject *object, Vector3 dir)
{
  object->plane_collider = (PlaneCollider *)calloc(1, sizeof(PlaneCollider));
  object->plane_collider->pos = &object->pos;
  object->plane_collider->dir = dir;
}

float calculate_impulse(Vector3 vel, Vector3 normal, float mass1, float mass2, float elasticity)
{
  float relative_to_normal = vector3_dot(vel, normal);
  if (relative_to_normal > 0)
    return 0.0f;
  
  float j = -(1 + elasticity) * relative_to_normal;
  if (mass2 == 0)
    mass2 = 10000000;
  j /= (1/mass1) + 0.00001;

  return j;
}

/** Sphere on sphere collision detection
 * @param obj1 PhysObject with sphere collider  
 * @param obj2 PhysObject with sphere collider  
 */
int PE_sphere_sphere_detect(PhysObject *obj1, PhysObject *obj2)
{

}

/** Sphere on sphere collision response
 * @param obj1 PhysObject with sphere collider  
 * @param obj2 PhysObject with sphere collider  
 */
void PE_sphere_sphere_response(PhysObject *obj1, PhysObject *obj2)
{

}

/** Sphere on plane collision detection
 * @param obj1 PhysObject with sphere collider  
 * @param obj2 PhysObject with plane collider  
 */
int PE_sphere_plane_detect(PhysObject *obj1, PlaneCollider *plane, float *dist)
{
  if (obj1->sphere_collider == NULL || plane == NULL)
    return 0;
  *dist = vector3_dot(plane->dir, vector3_sub(*obj1->sphere_collider->pos, *plane->pos));
  return (*dist < obj1->sphere_collider->radius);
}

/** Sphere on plane collision response
 * @param obj1 PhysObject with sphere collider  
 * @param obj2 PhysObject with plane collider  
 */
void PE_sphere_plane_response(PhysObject *obj1, PhysObject *obj2, float *dist)
{
  float elasticity = obj1->elasticity;
  float impulse_1d = calculate_impulse(obj1->vel, obj2->plane_collider->dir, obj1->mass, obj2->mass, elasticity);
  if (impulse_1d == 0.0f)
    return;
  Vector3 impulse = vector3_scale(obj2->plane_collider->dir, impulse_1d);
  obj1->vel.x += obj1->inv_mass * impulse.x;
  obj1->vel.y += obj1->inv_mass * impulse.y;
  obj1->vel.z += obj1->inv_mass * impulse.z;
}

int PE_sphere_box_detect(PhysObject *obj1, PhysObject *obj2, float *dist, Vector3 *nearest_normal)
{
  if (obj1->sphere_collider == NULL || obj2->box_collider == NULL)
    return 0;
  int count = 0;
  float lowest_dist = RENDER_DISTANCE;

  for (int i=0; i<6; i++)
  {
    count += PE_sphere_plane_detect(obj1, obj2->box_collider->planes[i], dist);
    if (*dist < lowest_dist)
    {
      lowest_dist = *dist;
      *nearest_normal = obj2->box_collider->planes[i]->dir;
    }
  }
  *dist = lowest_dist;
  return count == 6;
}

void PE_sphere_box_response(PhysObject *obj1, PhysObject *obj2, float *dist, Vector3 *nearest_normal)
{
  float elasticity = obj1->elasticity;
  float impulse_1d = calculate_impulse(obj1->vel, *nearest_normal, obj1->mass, obj2->mass, elasticity);
  if (impulse_1d == 0.0f)
    return;
  Vector3 impulse = vector3_scale(*nearest_normal, impulse_1d);
  obj1->vel.x += obj1->inv_mass * impulse.x;
  obj1->vel.y += obj1->inv_mass * impulse.y;
  obj1->vel.z += obj1->inv_mass * impulse.z;
}

void physobject_collision(PhysObject *obj1, PhysObject *obj2)
{
  float dist; 
  Vector3 nearest_normal;

  if (PE_sphere_sphere_detect(obj1, obj2))
    PE_sphere_sphere_response(obj1, obj2);

  if (PE_sphere_plane_detect(obj1, obj2->plane_collider, &dist))
    PE_sphere_plane_response(obj1, obj2, &dist);
    
  if (PE_sphere_box_detect(obj1, obj2, &dist, &nearest_normal))
    PE_sphere_box_response(obj1, obj2, &dist, &nearest_normal);
}
