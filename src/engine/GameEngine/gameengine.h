#ifndef GAMEENGINE_H
#define GAMEENGINE_H

#include "../GraphicsEngine/graphics.h"
#include "../PhysicsEngine/physics.h"
#include "../math/vector.h"

typedef struct gameobject {

  Vector3 pos, vel;
  BoxCollider *box_collider;
  SphereCollider *sphere_collider;
  Model *model;

  struct gameobject *next; // Intrusive linked-list

} GameObject;


/** Initialise a new GameObject with all values set to zero
 */
GameObject *gameobject_create(void);

void gameobject_translate(GameObject *object, float x, float y, float z);


void gameobject_draw_all(Camera cam);


#endif /* GAMEENGINE_H */