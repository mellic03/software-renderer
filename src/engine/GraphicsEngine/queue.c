#ifdef __unix__
  #include <SDL2/SDL.h>
#elif defined(_WIN32) || defined(WIN32)
  #define SDL_MAIN_HANDLED
  #include <SDL.h>
#endif

#include <stdio.h>

#include "queue.h"
#include "model.h"


RSR_node_t *RSR_node_create(Polygon *polygon)
{
  RSR_node_t *new = (RSR_node_t *)malloc(sizeof(RSR_node_t));
  new->data = polygon;
  return new;
}

void RSR_node_free(RSR_node_t *node)
{
  free(node);
}

void RSR_enque(RSR_queue_t *queue, Polygon *polygon)
{
  if (queue->size == 0)
  {
    queue->head = RSR_node_create(polygon);
    queue->tail = queue->head;
    queue->head->next = queue->tail;
  }
  else
  {
    RSR_node_t *new = RSR_node_create(polygon);
    queue->tail->next = new;
    queue->tail = new;
  }
  queue->size += 1;
}

void RSR_dequeue(RSR_queue_t *queue)
{
  if (queue->size == 0)
    return;

  else
  {
    RSR_node_t *temp = queue->head->next;
    RSR_node_free(queue->head);
    queue->head = temp;

    queue->size -= 1;
  }
}

Polygon *RSR_front(RSR_queue_t *queue)
{
  if (queue->head != NULL)
    return queue->head->data;
  else return NULL;
}

Polygon *RSR_rear(RSR_queue_t *queue)
{
  return queue->tail->data;
}

RSR_queue_t *RSR_queue_create(void)
{
  RSR_queue_t *new = calloc(1, sizeof(RSR_queue_t));
  return new;
}
