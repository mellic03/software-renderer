#ifndef QUEUE_H
#define QUEUE_H

#ifdef __unix__
  #include <SDL2/SDL.h>
#elif defined(_WIN32) || defined(WIN32)
  #define SDL_MAIN_HANDLED
  #include <SDL.h>
#endif

#include "model.h"

typedef struct RSR_node {
  Polygon *polygon;
  struct RSR_node *next;
} RSR_node_t;

typedef struct RSR_queue {
  int size;
  RSR_node_t *head;
  RSR_node_t *tail;
} RSR_queue_t;


// LINKED LIST
//----------------------------------------------
RSR_node_t *RSR_node_create(Polygon *polygon);
void RSR_node_free(RSR_node_t *node);
//----------------------------------------------


// QUEUE
//----------------------------------------------
RSR_node_t *RSR_node_mid(RSR_queue_t *queue);
void RSR_enque(RSR_queue_t *queue, Polygon *polygon);
void RSR_dequeue(RSR_queue_t *queue);
Polygon *RSR_front(RSR_queue_t *queue);
Polygon *RSR_rear(RSR_queue_t *queue);
RSR_queue_t *RSR_queue_init(void);
//----------------------------------------------



#endif /* QUEUE_H */