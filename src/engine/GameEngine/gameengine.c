#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "../engine.h"
#include "gameengine.h"
#include "../GraphicsEngine/graphics.h"

static GameObject *head = NULL;
int gameobject_count = 0;

/** Determine if a ray intersects a triangle
 *  Pulled straight from wikipedia
 */
bool ray_intersects_triangle(Vector3 ray_origin, Vector3 ray_vector, Polygon *tri_in, Vector3 *intersect_point)
{
  const float EPSILON = 0.0000001;
  Vector3 vertex0 = tri_in->vertices[0];
  Vector3 vertex1 = tri_in->vertices[1];  
  Vector3 vertex2 = tri_in->vertices[2];
  Vector3 edge1, edge2, h, s, q;
  float a,f,u,v;

  edge1 = vector3_sub(vertex1, vertex0);
  edge2 = vector3_sub(vertex2, vertex0);
  h = vector3_cross(ray_vector, edge2);
  a = vector3_dot(edge1, h);
  if (a > -EPSILON && a < EPSILON)
    return false;

  f = 1.0/a;
  s = vector3_sub(ray_origin, vertex0);
  u = f * vector3_dot(s, h);
  if (u < 0.0 || u > 1.0)
    return false;

  q = vector3_cross(s, edge1);
  v = f * vector3_dot(ray_vector, q);
  if (v < 0.0 || u + v > 1.0)
      return false;

  float t = f * vector3_dot(edge2, q);
  if (t > EPSILON)
  {
    *intersect_point = vector3_add(ray_origin, vector3_scale(ray_vector, t));
    return true;
  }
  else
    return false;
}

void player_collide(Player *player, Polygon *tri, Vector3 ray_direction, float d)
{
  Vector3 intersect;
  if (ray_intersects_triangle(player->game_object->pos, ray_direction, tri, &intersect))
  {
    float dist = vector3_dist(player->game_object->pos, intersect);
    if (0 < dist && dist < d)
    {
      float impulse_1d = calculate_impulse(player->game_object->phys_object->vel, tri->face_normal, player->game_object->phys_object->mass, 0, player->game_object->phys_object->elasticity);
      Vector3 impulse = vector3_scale(tri->face_normal, impulse_1d);
      player->game_object->phys_object->vel.y += player->game_object->phys_object->inv_mass * impulse.y;

      player->game_object->pos = vector3_sub(player->game_object->pos, vector3_scale(ray_direction, (d-dist)));
      // player->game_object->pos.y -= (4-dist);
    }
  }
}

void player_collision(Player *player)
{
  GameObject *obj = head;

  float nearest_dist = 1000;

  while (obj != NULL)
  {
    if (obj->model != NULL && obj->object_tag != 15)
    {
      for (int i=0; i<obj->model->poly_count; i++)
      {
        player_collide(player, &obj->model->polygons[i], player->ray_up   , 2);
        player_collide(player, &obj->model->polygons[i], player->ray_down , 4);
        
        player_collide(player, &obj->model->polygons[i], player->ray_left , 2);
        player_collide(player, &obj->model->polygons[i], player->ray_right, 2);
        player_collide(player, &obj->model->polygons[i], player->ray_front, 2);
        player_collide(player, &obj->model->polygons[i], player->ray_back , 2);
        
        // Vector3 intersect_point;
        // if (ray_intersects_triangle(*player->cam->pos, player->cam->dir, &obj->model->polygons[i], &intersect_point))
        // {
        //   float dist = vector3_dist(player->game_object->pos, intersect_point);
        //   if (dist < nearest_dist)
        //   {
        //     nearest_dist = dist;
        //     Vector3 dir = vector3_sub(*GE_cam->pos, intersect_point);
        //     vector3_normalise(&dir);
        //     lightsource.pos = vector3_add(intersect_point, dir);
        //   }
        // }

      }
    }
    obj = obj->next;
  }
}

void gameobject_tick(void)
{
  GameObject *obj = head;
  Vector3 diff;

  while (obj != NULL)
  {
    // If physics suddenly breaks this is probably why, uncomment this and delete the line below it
    diff = vector3_sub(obj->phys_object->pos, obj->phys_object->pos_last);
    // diff = vector3_sub(obj->phys_object->pos, obj->pos);
    gameobject_translate(obj, diff.x, diff.y, diff.z);

    obj = obj->next;
  }
}

void gameobject_set_pos(GameObject *obj, float x, float y, float z)
{
  gameobject_translate(obj, -obj->pos.x, -obj->pos.y, -obj->pos.z);
  gameobject_translate(obj, x, y, z);
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
    GE_model_translate(object->model, x, y, z);
}

void gameobject_scale(GameObject *obj, float x, float y, float z)
{
  GE_model_scale_xyz(obj->model, x, y, z);

  if (obj->phys_object->box_collider != NULL)
    physobject_box_collider_scale(obj->phys_object, x, y, z);
}

void gameobject_rotate_x(GameObject *object, float r)
{
  // float px = object->pos.x;
  // float py = object->pos.y;
  // float pz = object->pos.z;

  // gameobject_translate(object, -px, -py, -pz);
  GE_model_rotx(object->model, r);
  // gameobject_translate(object, px, py, pz);

}

void gameobject_rotate_y(GameObject *object, float r)
{
  // float px = object->pos.x;
  // float py = object->pos.y;
  // float pz = object->pos.z;

  // gameobject_translate(object, -px, -py, -pz);
  GE_model_roty(object->model, r);
  // gameobject_translate(object, px, py, pz);
}

void gameobject_rotate_z(GameObject *object, float r)
{
  GE_model_rotz(object->model, r);

  if (object->phys_object->plane_collider != NULL)
    vector3_rotz(&object->phys_object->plane_collider->dir, r);
}

void gameobject_free(GameObject *object)
{
  physobject_free(object->phys_object);
  GE_model_free(object->model);
  free(object);
}

void gameobject_delete(GameObject *object)
{
  // int id = object->object_id;

  // GameObject *temp = head;
  
  // if (head->object_id == id)
  // {
  //   head = head->next;
  //   gameobject_free(temp);
  // }
  // else
  // {
  //   while (temp->next != NULL && temp->next->object_id != id)
  //     temp = temp->next;

  //   GameObject *temp2 = temp->next->next;
  //   gameobject_free(temp->next);
  //   temp->next = temp2;
  // }
}

void gameobject_draw_all() {

  GameObject *temp = head;

  if (temp == NULL)
    return;

  if (temp->model != NULL)
    GE_model_enque(temp->model);
  
  while (temp->next != NULL)
  {
    temp = temp->next;

    if (temp->model != NULL)
      GE_model_enque(temp->model);
  }

}
