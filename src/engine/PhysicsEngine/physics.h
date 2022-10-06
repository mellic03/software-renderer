#ifndef PHYSICS_H
#define PHYSICS_H

#include "../math/vector.h"
#define G 55 // Gravitational constant
#define DEFAULT_ELASTICITY 1

extern double phys_delta_time;

typedef struct {
  Vector3 *pos, dir;
} PlaneCollider;


typedef struct {

  float xwidth, ywidth, zwidth;
  Vector3 *pos;
  Vector3 plane_pos[6];
  PlaneCollider **planes;

} BoxCollider;


typedef struct {

  Vector3 *pos;
  float radius;

} SphereCollider;


typedef struct physobject {

  float mass, inv_mass;
  Vector3 pos_last, pos, acc, vel;

  float elasticity;

  BoxCollider *box_collider;
  SphereCollider *sphere_collider;
  PlaneCollider *plane_collider;

  struct physobject *next;
  int object_id;

} PhysObject;


void physics_tick(void);


PhysObject *physobject_create(void);
void physobject_free(PhysObject *object);
float calculate_impulse(Vector3 vel, Vector3 normal, float mass1, float mass2, float elasticity);

// FORCE
//------------------------------------------------------------------
void physics_attract(Vector3 *attracted_vel, Vector3 *attracted, Vector3 *attractor);
//------------------------------------------------------------------
void physobject_box_collider_scale(PhysObject *obj, float x, float y, float z);

// COLLISION
//------------------------------------------------------------------
void physobject_give_box_collider(PhysObject *object, float x, float y, float z);
void physobject_give_sphere_collider(PhysObject *object, float radius);
void physobject_give_plane_collider(PhysObject *object, Vector3 dir);

// DETECTION
int box_colliding(BoxCollider *box1, BoxCollider *box2);
int sphere_colliding(SphereCollider *sphere1, SphereCollider *sphere2, float *dist);
int sphere_plane_colliding(SphereCollider *sphere, PlaneCollider *plane, float *dist);

// RESPONSE
void physobject_collision(PhysObject *obj1, PhysObject *obj2);
//------------------------------------------------------------------


#endif /* PHYSICS_H */