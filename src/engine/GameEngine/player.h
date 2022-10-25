#ifndef PLAYER_H
#define PLAYER_H

#include "../math/enginemath.h"
#include "gameengine.h"



Player *player_create(void);
void input(SDL_Event event, Camera *cam, Player *player);


#endif /* PLAYER_H */