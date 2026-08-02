#ifndef INC_SHOT_H
#define INC_SHOT_H

#include <SDL/SDL.h>

class Shot
{
    private:
	int timeLived;
    public:
	CartCoord pos;
	RelPolarCoord vel;

	int dead;
	static int is_dead(const Shot& shot)
	    { return shot.dead; }

	int weight;

	bool super;

	Shot(CartCoord ipos, RelPolarCoord ivel, int iweight=1,
		bool isuper=false) :
	    timeLived(0),
	    pos(ipos), vel(ivel), dead(0), weight(iweight), super(isuper)
	{}

	// time in ms
	void update(int time);

	int hit(int damage);
	int die();

	void draw(SDL_Surface* surface, const View& view, View*
	    boundView=NULL, bool noAA=false);
};


#endif /* INC_SHOT_H */
