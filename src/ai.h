#ifndef INC_AI_H
#define INC_AI_H

#include <SDL/SDL_stdinc.h>

class GameState;
class Invader;
class HPInvader;
struct RelPolarCoord;

class AI
{
    protected:
	GameState* gameState;

	// Some utility functions for dealing with the state (which has
	// befriended us)
	HPInvader* closestEnemy();
	void updateSeen();
	RelPolarCoord predictPos(Invader* inv, float time);

    public:
	static const Uint8 K_LEFT = 1<<0;
	static const Uint8 K_RIGHT = 1<<1;
	static const Uint8 K_DEAIM = 1<<2;
	static const Uint8 K_DEZOOM = 1<<3;
	static const Uint8 K_SHOOT1 = 1<<4;
	static const Uint8 K_SHOOT2 = 1<<5;
	static const Uint8 K_SHOOT3 = 1<<6;
	static const Uint8 K_POD = 1<<7;
	Uint8 keys;

	virtual void update(int time) =0;

	AI(GameState* gameState) : gameState(gameState), keys(0) {}

	virtual ~AI() {}
};

class AIData
{
    public:
	bool seen;
	AIData() : seen(false) {}
};

class BasicAI : public AI
{
    private:
	int seed;
	void newSeed();
	int getShotWeight(int hp, int armour) const;
    public:
	void update(int time);

	BasicAI(GameState* gameState);
};


#endif /* INC_AI_H */
