#ifndef GAMEENGINE_H
#define GAMEENGINE_H

#include "../PhysicsEngine/physics.h"
#include "../GraphicsEngine/graphics.h"
#include "../math/vector.h"

extern int toggle;

typedef struct gameobject {

  int object_id;
  int object_tag; 
  Vector3 pos;

  PhysObject *phys_object;  
  Model *model;

  struct gameobject *next;

} GameObject;

typedef struct {

  Camera *cam;
  GameObject *game_object;

  GameObject *raycast_test;

  Vector3 ray_up, ray_down, ray_left, ray_right, ray_front, ray_back;

} Player;

GameObject *gameobject_create(void);
void gameobject_assign_model(GameObject *object, Model *model);

void gameobject_collide(GameObject *obj1, GameObject *obj2);

void gameobject_tick();

void gameobject_translate(GameObject *object, float x, float y, float z);
void gameobject_set_pos(GameObject *obj, float x, float y, float z);

void gameobject_rotate_x(GameObject *object, float r);
void gameobject_rotate_y(GameObject *object, float r);
void gameobject_rotate_z(GameObject *object, float r);

void gameobject_scale(GameObject *object, float x, float y, float z);
void gameobject_delete(GameObject *object);

void gameobject_draw_all();

void player_collision(Player *player);


#endif /* GAMEENGINE_H */