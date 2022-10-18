#include <stddef.h>
#include <stdio.h>
#include <math.h>
#include "physics.h"
#include "../math/vector.h"

#include "../GameEngine/gameengine.h"

static PhysObject *head = NULL;
static int physobject_count = 0;

void physics_tick(void)
{
  PhysObject *obj1 = head;
  PhysObject *obj2; 

  while (obj1 != NULL)
  {
    obj1->pos_last = obj1->pos;
    obj1->pos = vector3_add(obj1->pos, vector3_scale(obj1->vel, delta_time));

    if (obj1->mass != 0)
    {
      obj1->acc = vector3_scale(obj1->acc, 0.99);
      obj1->vel = vector3_scale(obj1->vel, 0.999);
      obj1->vel = vector3_add(obj1->vel, (Vector3){0, G*obj1->mass*delta_time, 0});

      obj2 = head;

      while (obj2 != NULL)
      {
        if (obj1->object_id != obj2->object_id)
          physobject_collision(obj1, obj2);

        obj2 = obj2->next;
      }
    }
    obj1 = obj1->next;
  }
}

PhysObject *physobject_create(void)
{
  if (head == NULL)
  {
    head = (PhysObject *)calloc(1, sizeof(PhysObject));
    head->object_id = physobject_count;
  }

  else
  {
    PhysObject *new = (PhysObject *)calloc(1, sizeof(PhysObject));
    new->object_id = physobject_count;
    new->next = head;
    head = new;
  }

  head->elasticity = DEFAULT_ELASTICITY;
  physobject_count += 1;

  return head;
}

void physobject_free(PhysObject *object)
{
  free(object->box_collider);
  free(object->sphere_collider);
  free(object->plane_collider);
  free(object);
}

void physobject_delete(PhysObject *object)
{
  PhysObject *temp = head;
  if (temp->next == NULL)
  {
    physobject_free(temp);
    return;
  }
  
  while (temp->next->object_id != object->object_id)
    temp = temp->next;

  PhysObject *temp2 = temp->next->next;

  physobject_free(temp->next);
  temp->next = temp2;
}

void physobject_box_collider_scale(PhysObject *obj, float x, float y, float z)
{
  obj->box_collider->plane_pos[0].x *= x;
  obj->box_collider->plane_pos[1].x *= x;

  obj->box_collider->plane_pos[2].y *= y;
  obj->box_collider->plane_pos[3].y *= y;

  obj->box_collider->plane_pos[4].z *= z;
  obj->box_collider->plane_pos[5].z *= z;
}
