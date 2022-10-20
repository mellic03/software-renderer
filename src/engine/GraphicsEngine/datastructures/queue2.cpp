#include <iostream>

#include <stdlib.h>

#include "model.h"


typedef struct RSR_node {
  Polygon *polygon;
  struct RSR_node *next;
} RSR_node_t;

RSR_node_t *RSR_node_create(Polygon *polygon)
{
  RSR_node_t *ptr = (RSR_node_t *)malloc(sizeof(RSR_node_t));
  ptr->polygon = (Polygon *)malloc(sizeof(Polygon));
  *ptr->polygon = *polygon;
  return ptr;
}

class Queue {

  private:

    RSR_node_t *head = (RSR_node_t *)calloc(1, sizeof(RSR_node_t));
    RSR_node_t *tail = (RSR_node_t *)calloc(1, sizeof(RSR_node_t));


  public:

    int size = 0;

    void enque(Polygon *polygon)
    {
      RSR_node_t *temp = this->head;
      this->head = RSR_node_create(polygon);
      this->head->next = temp;
    }

    void deque(void)
    {
      free(this->head);
    }

};


