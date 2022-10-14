#include <stdio.h>
#include <stdlib.h>

#include "../engine.h"
#include "gameengine.h"
#include "../GraphicsEngine/graphics.h"

GameObject *head = NULL;
int gameobject_count = 0;

int point_over_triangle(Polygon *tri, Vector3 point)
{
  Vector3 n1 = vector3_cross(vector3_sub(tri->vertices[1], tri->vertices[0]), vector3_sub(tri->vertices[2], tri->vertices[0]));
  Vector3 n2 = vector3_cross(vector3_sub(tri->vertices[1], tri->vertices[0]), vector3_sub(point, tri->vertices[0]));
  Vector3 n3 = vector3_cross(vector3_sub(tri->vertices[2], tri->vertices[1]), vector3_sub(point, tri->vertices[1]));
  Vector3 n4 = vector3_cross(vector3_sub(tri->vertices[0], tri->vertices[2]), vector3_sub(point, tri->vertices[2]));
  return vector3_dot(n1, n2) >0 && vector3_dot(n1, n3) > 0 && vector3_dot(n1, n4) > 0;
}


void Barycentric(Vector3 a, Vector3 b, Vector3 c, Vector3 p, float *u, float *v, float *w)
{
  Vector3 v0 = vector3_sub(b, a), v1 = vector3_sub(c, a), v2 = vector3_sub(p, a);
  float d00 = vector3_dot(v0, v0);
  float d01 = vector3_dot(v0, v1);
  float d11 = vector3_dot(v1, v1);
  float d20 = vector3_dot(v2, v0);
  float d21 = vector3_dot(v2, v1);
  float denom = d00 * d11 - d01 * d01;
  *v = (d11 * d20 - d01 * d21) / denom;
  *w = (d00 * d21 - d01 * d20) / denom;
  *u = 1.0f - *v - *w;
}

void player_collision(Player *player)
{
  GameObject *obj = head;

  while (obj != NULL)
  {
    if (obj->model != NULL)
    {
      for (int i=0; i<obj->model->poly_count; i++)
      {
        if (vector3_dot(obj->model->polygons[i].face_normal, player->ray_down) < -0.2)
        {
          if (point_over_triangle(&obj->model->polygons[i], player->game_object->pos))
          {
            float dist = vector3_dot(obj->model->polygons[i].face_normal, vector3_sub(player->game_object->pos, obj->model->polygons[i].vertices[0]));
            if (0 < dist && dist < 4)
            {
              float impulse_1d = calculate_impulse(player->game_object->phys_object->vel, obj->model->polygons[i].face_normal, player->game_object->phys_object->mass, 0, player->game_object->phys_object->elasticity);
              Vector3 impulse = vector3_scale(obj->model->polygons[i].face_normal, impulse_1d);
              player->game_object->phys_object->vel.y += player->game_object->phys_object->inv_mass * impulse.y;
              player->game_object->pos.y -= (4-dist);
            }
          }
        }
      }
    }
    obj = obj->next;
  }
}

void gameobject_tick(void)
{
  GameObject *obj = head;
  Vector3 trans;

  while (obj != NULL)
  {
    trans = vector3_sub(obj->phys_object->pos, obj->phys_object->pos_last);
    gameobject_translate(obj, trans.x, trans.y, trans.z);

    obj = obj->next;
  }
}

GameObject *gameobject_create(void)
{
  if (head == NULL)
  {
    head = (GameObject *)calloc(1, sizeof(GameObject));
    head->object_id = gameobject_count;
  }

  else
  {
    GameObject *new = (GameObject *)calloc(1, sizeof(GameObject));
    new->object_id = gameobject_count;
    new->next = head;
    head = new;
  }

  head->phys_object = physobject_create();
  head->phys_object->pos = head->pos;
  gameobject_count += 1;

  return head;
}

void gameobject_assign_model(GameObject *object, Model *model)
{
  object->model = model;
  object->model->pos = object->pos;
}

void gameobject_translate(GameObject *object, float x, float y, float z)
{
  object->pos.x += x;
  object->pos.y += y;
  object->pos.z += z;

  object->phys_object->pos_last.x = object->pos.x;
  object->phys_object->pos_last.y = object->pos.y;
  object->phys_object->pos_last.z = object->pos.z;

  object->phys_object->pos.x = object->pos.x;
  object->phys_object->pos.y = object->pos.y;
  object->phys_object->pos.z = object->pos.z;

  if (object->model != NULL)
    translate_model(object->model, x, y, z);
}

void gameobject_scale(GameObject *obj, float x, float y, float z)
{
  scale_xyz(obj->model, x, y, z);

  if (obj->phys_object->box_collider != NULL)
    physobject_box_collider_scale(obj->phys_object, x, y, z);
}

void gameobject_rotate_x(GameObject *object, float r)
{
  // float px = object->pos.x;
  // float py = object->pos.y;
  // float pz = object->pos.z;

  // gameobject_translate(object, -px, -py, -pz);
  rotate_x(object->model, r);
  // gameobject_translate(object, px, py, pz);

}

void gameobject_rotate_y(GameObject *object, float r)
{
  // float px = object->pos.x;
  // float py = object->pos.y;
  // float pz = object->pos.z;

  // gameobject_translate(object, -px, -py, -pz);
  rotate_y(object->model, r);
  // gameobject_translate(object, px, py, pz);
}

void gameobject_rotate_z(GameObject *object, float r)
{
  rotate_z(object->model, r);

  if (object->phys_object->plane_collider != NULL)
    rotate_point(&object->phys_object->plane_collider->dir, 0, 0, r);
}

void gameobject_free(GameObject *object)
{
  physobject_free(object->phys_object);
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
    while (temp->next != NULL && temp->next->object_id != id)
      temp = temp->next;

    GameObject *temp2 = temp->next->next;
    gameobject_free(temp->next);
    temp->next = temp2;
  }
}

void gameobject_draw_all(Camera *cam) {

  GameObject *temp = head;

  if (temp == NULL)
    return;

  if (temp->model != NULL)
    model_draw(cam, temp->model);
  
  while (temp->next != NULL)
  {
    temp = temp->next;

    if (temp->model != NULL)
      model_draw(cam, temp->model);
  }

}
