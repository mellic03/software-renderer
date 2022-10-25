#ifndef DATASTRUCTURES_H
#define DATASTRUCTURES_H

#ifdef __unix__
  #include <SDL2/SDL.h>
#elif defined(_WIN32) || defined(WIN32)
  #define SDL_MAIN_HANDLED
  #include <SDL.h>
#endif

#include <pthread.h>

#include "../model.h"

typedef struct RSR_node {
  Polygon *polygon;
  struct RSR_node *next;
} RSR_node_t;

typedef struct RSR_queue {
  pthread_mutex_t mutex;
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
void RSR_enque(RSR_queue_t *queue, Polygon *polygon);
void RSR_dequeue(RSR_queue_t *queue);
Polygon *RSR_front(RSR_queue_t *queue);
Polygon *RSR_rear(RSR_queue_t *queue);
RSR_queue_t *RSR_queue_init(void);
//----------------------------------------------


// BSP TREE
//----------------------------------------------
typedef struct BSPnode {
  Polygon *polygon;
  struct BSPnode *left, *right;
} BSPnode_t;

BSPnode_t *BSP_tree_generate(Polygon *polygons, int poly_count);
void BSP_tree_enque(Vector3 *pos, BSPnode_t *node, RSR_queue_t *queue);

//----------------------------------------------




#endif /* DATASTRUCTURES_H */