#ifndef INC_PLAYER_H
#define INC_PLAYER_H

#include <SDL/SDL.h>

#include "coords.h"
#include "gfx.h"

struct Player
{
    int shootCoolrate;
    int shootHeat;
    int shootMaxHeat;

    float shield;

    RelPolarCoord aim;

    int shootTimer;
    int podTimer;

    int score;
    bool doneLaunchedPod;
    bool dead;

    float aimAccuracy();

    void draw(SDL_Surface* surface, const View& view, View* boundView=NULL,
	bool noAA=false);

    void update(int time, bool superShield = false);

    float radius();

    Player();
};

#endif /* INC_PLAYER_H */
