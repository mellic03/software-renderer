#ifndef QUEUE_H
#define QUEUE_H

#ifdef __unix__
  #include <SDL2/SDL.h>
#elif defined(_WIN32) || defined(WIN32)
  #define SDL_MAIN_HANDLED
  #include <SDL.h>
#endif

#include "model.h"

typedef struct node {
  Polygon *data;
  struct node *next;
} NODE;

typedef struct queue {
  int size;
  NODE *head;
  NODE *tail;
} QUEUE;


// LINKED LIST
//----------------------------------------------
NODE *RSR_node_create(Polygon *polygon);
void RSR_node_free(NODE *node);
//----------------------------------------------


// QUEUE
//----------------------------------------------
void RSR_enque(QUEUE *queue, Polygon *polygon);
void RSR_dequeue(QUEUE *queue);
Polygon *RSR_front(QUEUE *queue);
Polygon *RSR_rear(QUEUE *queue);
QUEUE *RSR_queue_create(void);
//----------------------------------------------




#endif /* QUEUE_H */