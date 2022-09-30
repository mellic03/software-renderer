#include "gameengine.h"
#include "../GraphicsEngine/graphics.h"

GameObject *head;


GameObject *gameobject_create(void)
{
  GameObject *new = (GameObject *)calloc(1, sizeof(GameObject));
  new->model = NULL;
  new->next = head;
  head = new;
  return new;
}

void gameobject_translate(GameObject *object, float x, float y, float z)
{
  object->pos.x += x;
  object->pos.y += y;
  object->pos.z += z;

  translate_model(object->model, x, y, z);
}

void gameobject_draw_all(Camera cam) {

  GameObject *temp = head;

  if (temp == NULL)
    return;

  draw_model(cam, temp->model);

  while (temp->next != NULL)
  {
    temp = temp->next;
    draw_model(cam, temp->model);
  }
}