#ifndef GAMEENGINE_H
#define GAMEENGINE_H

#include "../GraphicsEngine/graphics.h"
#include "../PhysicsEngine/physics.h"
#include "../math/vector.h"

typedef struct gameobject {

  int object_id;
  Vector3 pos, vel;
  int mass;

  BoxCollider *box_collider;
  SphereCollider *sphere_collider;
  PlaneCollider *plane_collider;

  Model *model;

  struct gameobject *next; // Intrusive linked-list

} GameObject;


GameObject *gameobject_create(void);
void gameobject_assign_model(GameObject *object, Model *model);
void gameobject_give_box_collider(GameObject *object);
void gameobject_give_sphere_collider(GameObject *object, float radius);
void gameobject_give_plane_collider(GameObject *object, Vector3 dir);


void gameobject_tick(void);

void gameobject_translate(GameObject *object, float x, float y, float z);
void gameobject_delete(GameObject *object);

void gameobject_draw_all(Camera *cam);


#endif /* GAMEENGINE_H */