#include "player.h"


Player *player_create(void)
{
  Player *new = (Player *)calloc(1, sizeof(Player));
  new->cam = create_camera();


  new->ray_up  = (Vector3){0, -1, 0};
  new->ray_down  = (Vector3){0, 1, 0};

  new->ray_left  = (Vector3){-1, 0, 0};
  new->ray_right = (Vector3){1, 0, 0};

  new->ray_front = (Vector3){0, 0, 1};
  new->ray_back  = (Vector3){0, 0, -1};

  return new;
}
