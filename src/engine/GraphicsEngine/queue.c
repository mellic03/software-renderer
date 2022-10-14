#ifdef __unix__
  #include <SDL2/SDL.h>
#elif defined(_WIN32) || defined(WIN32)
  #define SDL_MAIN_HANDLED
  #include <SDL.h>
#endif

#include <stdio.h>

#include "queue.h"
#include "model.h"


NODE *RSR_node_create(Polygon *polygon)
{
  NODE *new = (NODE *)malloc(sizeof(NODE));
  new->data = polygon;
  return new;
}

void RSR_node_free(NODE *node)
{
  SDL_FreeSurface(node->data->texture);
  free(node->data);
  free(node);
}

void RSR_enque(QUEUE *queue, Polygon *polygon)
{
  if (queue->size == 0)
  {
    queue->head = RSR_node_create(polygon);
    queue->tail = queue->head;
    queue->head->next = queue->tail;
  }
  else
  {
    NODE *new = RSR_node_create(polygon);
    queue->tail->next = new;
    queue->tail = new;
  }
  queue->size += 1;
}

void RSR_dequeue(QUEUE *queue)
{
  if (queue->size == 0)
    return;

  else
  {
    NODE *temp = queue->head->next;
    RSR_node_free(queue->head);
    queue->head = temp;
  }
  queue->size -= 1;
}

Polygon *RSR_front(QUEUE *queue)
{
  return queue->head->data;
}

Polygon *RSR_rear(QUEUE *queue)
{
  return queue->tail->data;
}

QUEUE *RSR_queue_create(void)
{
  QUEUE *new = calloc(1, sizeof(QUEUE));
  return new;
}
