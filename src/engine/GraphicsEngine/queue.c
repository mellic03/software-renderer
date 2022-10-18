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
  new->polygon = (Polygon *)malloc(sizeof(Polygon));
  *new->polygon = *polygon;
  return new;
}

void RSR_node_free(RSR_node_t *node)
{
  free(node->polygon);
  free(node);
}

/** Return a pointer to the middle node in the queue
 */
RSR_node_t *RSR_node_mid(RSR_queue_t *queue)
{
  int size = queue->size;
  if (size == 0)
    return NULL;
  
  RSR_node_t *ptr = queue->head;

  for (int i=0; i<size/2; i++)
    ptr = ptr->next;

  return ptr;
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
    return queue->head->polygon;
  else
    return NULL;
}

Polygon *RSR_rear(RSR_queue_t *queue)
{
  if (queue->tail->polygon != NULL)
    return queue->tail->polygon;
  else
    return NULL;
}

RSR_queue_t *RSR_queue_init(void)
{
  RSR_queue_t *new = calloc(1, sizeof(RSR_queue_t));
  return new;
}

