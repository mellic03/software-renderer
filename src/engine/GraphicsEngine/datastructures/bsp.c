#ifdef __unix__
  #include <SDL2/SDL.h>
#elif defined(_WIN32) || defined(WIN32)
  #define SDL_MAIN_HANDLED
  #include <SDL.h>
#endif

#include <stdio.h>
#include <stdbool.h>

#include "datastructures.h"


float fpoint_plane_dist(Vector3 point, Vector3 plane_pos, Vector3 plane_normal)
{
  return vector3_dot(plane_normal, vector3_sub(point, plane_pos));
}

int point_plane_dist(Vector3 point, Vector3 plane_pos, Vector3 plane_normal)
{
  if (vector3_dot(plane_normal, vector3_sub(point, plane_pos)) > 0)
    return 1;
  else
    return 0;
}

/** Return true if all of a's vertices have a positive signed distance to the plane which b is a segment of
 */
bool poly_in_front(Polygon *a, Polygon *b)
{
  int total = 0;
  total += point_plane_dist(a->vertices[0], b->vertices[0], b->face_normal);
  total += point_plane_dist(a->vertices[1], b->vertices[0], b->face_normal);
  total += point_plane_dist(a->vertices[2], b->vertices[0], b->face_normal);
  
  if (total == 3)
    return true;
  
  if (total == 0)
    return false;
}

BSPnode_t *BSP_node_create(Polygon *polygon)
{
  BSPnode_t *new = (BSPnode_t *)calloc(1, sizeof(BSPnode_t));
  new->polygon = polygon;
  return new;
}

int front_count = 0;
int back_count = 0;

void BSP_insert(BSPnode_t *node, Polygon *polygon)
{
  if (node == NULL)
  {
    node = BSP_node_create(polygon);
    return;
  }

  else
  {
    if (poly_in_front(polygon, node->polygon))
    {
      front_count++;
      BSP_insert(node->left, polygon);
    }

    else
    {
      back_count++;
      BSP_insert(node->right, polygon);
    }
  }
}

BSPnode_t *BSP_tree_generate(Polygon *polygons, int poly_count)
{
  int indx = rand() % poly_count;

  BSPnode_t *root = BSP_node_create(&polygons[indx]);

  for (int i=0; i<indx; i++)
    BSP_insert(root, &polygons[i]);

  for (int i=indx+1; i<poly_count; i++)
    BSP_insert(root, &polygons[i]);

  printf("front: %d\n", front_count);
  printf("back: %d\n", back_count);

  return root;
}



void BSP_tree_enque(Vector3 *pos, BSPnode_t *node, RSR_queue_t *queue)
{
  const float EPSILON = 0.0001;

  if (node->left == NULL && node->right == NULL)
  {
    RSR_enque(queue, node->polygon);
    return;
  }

  float dist = fpoint_plane_dist(*pos, node->polygon->vertices[0], node->polygon->face_normal);

  // If on +ve side of polygon
  if (dist > EPSILON)
  {
    BSP_tree_enque(pos, node->left, queue);
    RSR_enque(queue, node->polygon);
  }
  
  // If on -ve side of polygon
  else if (dist < -EPSILON)
  {
    BSP_tree_enque(pos, node->right, queue);
    RSR_enque(queue, node->polygon);
  }

}