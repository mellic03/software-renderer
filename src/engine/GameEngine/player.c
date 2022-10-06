#include "player.h"


Player *player_create(void)
{
  Player *new = (Player *)calloc(1, sizeof(Player));
  new->cam = create_camera();
  new->ray_down = (Vector3){0, 1, 0};
  return new;
}
