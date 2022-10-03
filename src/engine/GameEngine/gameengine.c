#include <stdio.h>
#include <stdlib.h>

#include "../engine.h"
#include "gameengine.h"
#include "../GraphicsEngine/graphics.h"

GameObject *head = NULL;
int gameobject_count = 0;

void momentumsomething(GameObject *obj1, GameObject *obj2)
{
  Vector3 momentum = vector3_add(vector3_scale(obj1->vel, obj1->mass), vector3_scale(obj2->vel, obj2->mass));
  obj1->vel = vector3_scale(momentum, 1/obj1->mass);
  obj2->vel = vector3_scale(momentum, 1/obj2->mass);
}

void gameobject_tick(void)
{
  GameObject *obj1 = head;
  GameObject *obj2;

  while (obj1 != NULL)
  {
    obj1->vel = vector3_scale(obj1->vel, 0.99);

    if (obj1->mass > 0)
      obj1->vel = vector3_add(obj1->vel, (Vector3){0, 0.01, 0});

    Vector3 new_pos = vector3_add(obj1->pos, vector3_scale(obj1->vel, 20*delta_time));
    new_pos = vector3_sub(new_pos, obj1->pos);
    gameobject_translate(obj1, new_pos.x, new_pos.y, new_pos.z);

    obj2 = head;
    while (obj2 != NULL)
    {
      if (obj1->object_id != obj2->object_id)
      {
        float dist = 0;
        Vector3 dir;
        dist = sphere_plane_colliding(obj1->sphere_collider, obj2->plane_collider);
        if (dist != 0)
        {
          float overlap_dist = obj1->sphere_collider->radius - dist;
          // printf("rad: %f, dist: %f, overlap: %f\n", obj1->sphere_collider->radius, dist, overlap_dist);
          dir = vector3_scale(obj2->plane_collider->dir, overlap_dist/2);
          obj1->vel = vector3_add(obj1->vel, dir);
        }

        if (sphere_colliding(obj1->sphere_collider, obj2->sphere_collider))
        {
          dist = vector3_dist(obj1->pos, obj2->pos);
          float overlap_dist = (obj1->sphere_collider->radius + obj2->sphere_collider->radius) - dist;

          dir = vector3_sub(obj1->pos, obj2->pos);
          vector3_normalise(&dir);

          dir = vector3_scale(dir, overlap_dist/2);
          obj1->vel = vector3_add(obj1->vel, dir);
          // gameobject_translate(obj1, dir.x, dir.y, dir.z);

          dir = vector3_scale(dir, -1);
          obj2->vel = vector3_add(obj2->vel, dir);
          // gameobject_translate(obj2, dir.x, dir.y, dir.z);

        }
      }
      obj2 = obj2->next;
    }
    obj1 = obj1->next;
  }
}

GameObject *gameobject_create(void)
{
  if (head == NULL)
  {
    head = (GameObject *)calloc(1, sizeof(GameObject));
    head->model = NULL;
    head->object_id = gameobject_count;
    head->pos = (Vector3){0, 0, 0};
    gameobject_count += 1;
  }

  else
  {
    GameObject *new = (GameObject *)calloc(1, sizeof(GameObject));
    new->model = NULL;
    new->next = head;
    new->object_id = gameobject_count;
    new->pos = (Vector3){0, 0, 0};
    gameobject_count += 1;
    head = new;
  }

  return head;
}

void gameobject_give_box_collider(GameObject *object)
{
  object->box_collider = (BoxCollider *)calloc(1, sizeof(BoxCollider));
}

void gameobject_give_sphere_collider(GameObject *object, float radius)
{
  object->sphere_collider = (SphereCollider *)calloc(1, sizeof(SphereCollider));
  object->sphere_collider->pos = &object->pos;
  object->sphere_collider->radius = radius;
}

void gameobject_give_plane_collider(GameObject *object, Vector3 dir)
{
  object->plane_collider = (PlaneCollider *)calloc(1, sizeof(PlaneCollider));
  object->plane_collider->pos = &object->pos;
  object->plane_collider->dir = dir;
}

void gameobject_assign_model(GameObject *object, Model *model)
{
  object->model = model;
  object->model->pos = &object->pos;
}

void gameobject_translate(GameObject *object, float x, float y, float z)
{
  object->pos.x += x;
  object->pos.y += y;
  object->pos.z += z;
  translate_model(object->model, x, y, z);
}

void gameobject_free(GameObject *object)
{
  free(object->box_collider);
  free(object->sphere_collider);
  free(object->plane_collider);
  model_free(object->model);
  free(object);
}

void gameobject_delete(GameObject *object)
{
  int id = object->object_id;

  GameObject *temp = head;
  
  if (head->object_id == id)
  {
    head = head->next;
    gameobject_free(temp);
  }
  else
  {
    // stop either at 2nd last object or at object before object with id
    while (temp->next != NULL && temp->next->object_id != id)
      temp = temp->next;

    if (temp->next == NULL)
      printf("object_id: %d not found, fix your code!\n", id);

    GameObject *temp2 = temp->next->next;
    gameobject_free(temp->next);
    temp->next = temp2;
  }
}

void gameobject_draw_all(Camera *cam) {

  GameObject *temp = head;

  if (temp == NULL)
    return;

  model_draw(cam, temp->model);

  while (temp->next != NULL)
  {
    temp = temp->next;
    model_draw(cam, temp->model);
  }
}



