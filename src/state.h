#ifndef INC_STATE_H
#define INC_STATE_H

#include <SDL/SDL.h>
#include "coords.h"
#include "gfx.h"
#include "shot.h"
#include "invaders.h"
#include "player.h"
#include "node.h"

#include <vector>

#define MAX(x,y) ((x) > (y) ? (x) : (y))
#define MIN(x,y) ((x) < (y) ? (x) : (y))
#define BOUNDED(x,min,max) (MAX(MIN((x),(max)),(min)))

enum EndState
{
    END_NOT,
    END_DEAD,
    END_EXTRACTED,
    END_WIN
};

class AI;

class GameState
{
    friend class AI;
    friend class BasicAI;
    private:
	std::vector<Shot> shots;
	std::vector<Invader*> invaders;
	std::vector<Node> nodes;

	Node* targettedNode;
	int invaderCooldown;

	bool youHaveNode(NodeColour colour);
	bool youHaveShotNode(int type);
	bool evilHasNode(NodeColour colour);

	int shotHeat(int type);
	int shotDelay(int type);

	float mutilationWave;
	Angle preMutilationPhase;
	int extractPreMutCutoff;
	void updateMutilation(int time);

	int rateOfRating(int rating);
	void updateObjects(int time);
	void handleGameInput(int time);
	void handleFreeViewInput(int time);
	void updateZoom(int time);
	void evilAI(int time);
	bool deadShots;
	void cleanup();

	bool freeViewMode;
	View freeView;

	void drawGrid(SDL_Surface* surface, const View& view);
	void drawTargettingLines(SDL_Surface* surface, const View& view);
	void drawNodeTargetting(SDL_Surface* surface, const View& view);
	void drawIndicators(SDL_Surface* surface, const View& view);
	void drawObjects(SDL_Surface* surface, const View& view,
		View* boundView=NULL);

    public:
	double extracted;
	float extractDecayRate;
	Player you;
	float zoomdist;
	int invaderRate;
	int speed;
	double rating;
	int extractMax;
	EndState end;

	AI* ai;

	void setRating();
	void update(int time, bool noInput=false);
	void draw(SDL_Surface* surface);

	const char* getHint();

	GameState(int speed);
};

const char* ratingString(int rating);

#endif /* INC_STATE_H */
