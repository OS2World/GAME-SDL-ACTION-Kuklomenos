#ifndef INC_INVADERS_H
#define INC_INVADERS_H

#include <vector>
#include <SDL/SDL.h>

#include "gfx.h"
#include "coords.h"
#include "collision.h"
#include "geom.h"
#include "ai.h"

class Node;

class Invader
{
    protected:
	virtual void doUpdate(int time) =0;
	virtual Uint32 colour() const =0;
	virtual Uint32 innerColour() const;

    public:
	// setCollTrajectory: Called after doUpdate; should set up a
	// CollisionObject to be returned by subsequent calls to collObj,
	// representing a linear approximation to the movement made by the
	// object.
	virtual void setCollTrajectory(CartCoord startPos, RelCartCoord
		velocity) =0;
	virtual const CollisionObject& collObj() const =0;

	virtual bool dead() const =0;

	// cpos: return current absolute position in the arena
	virtual CartCoord cpos() const =0;

	virtual int hit(int weight) =0;
	virtual int die() =0;
	virtual void onDeath() const {};

	virtual bool evil() const { return true; }
	virtual bool hitsYou() const { return true; }
	virtual bool hitsShots() const { return true; }

	virtual void fleeOnWin() {};

	virtual int killScore() const { return 1; }

	// hitsInvaders: if returns r>0, hits invaders as a circle of radius r
	virtual float hitsInvaders() const { return 0; }

	void update(int time);

	virtual void dodge() {};

	std::vector<Invader*> spawns;
	void spawnInvader(Invader* invader);

	AIData aiData;

	virtual void draw(SDL_Surface* surface, const View& view, View*
		boundView=NULL, bool noAA=false) const =0;

	Invader() {}
	virtual ~Invader() {};
};

class HPInvader : public virtual Invader
{
    public:
	int hp;
	int armour;
	virtual int hit(int weight);
	bool dead() const;
	int die();

    HPInvader(int ihp=3, int iarmour=0) : hp(ihp), armour(iarmour) {}
    virtual ~HPInvader() {};
};

class SpirallingInvader : public virtual Invader
{
    protected:
	void doUpdate(int time);

	virtual void drawSuper(SDL_Surface* surface, const View& view, View*
		boundView=NULL, bool noAA=false) const;
    public:
	RelPolarCoord pos;
	CartCoord focus;
	float ds;
	float dd;

	virtual void dodge();

	virtual void fleeOnWin();

	SpirallingInvader(RelPolarCoord ipos, float ids=0, float idd=1,
		CartCoord ifocus=ARENA_CENTRE) :
	    pos(ipos), focus(ifocus), ds(ids), dd(idd) {}

	CartCoord cpos() const;
};

class CircularInvader : public virtual Invader
{
    public:
	float radius;
	CollisionCircle cc;
	const CollisionObject& collObj() const;
	void setCollTrajectory(CartCoord startPos, RelCartCoord velocity);

	void draw(SDL_Surface* surface, const View& view, View*
	    boundView=NULL, bool noAA=false) const;

	CircularInvader(float iradius=5) :
	    radius(iradius), cc(CartCoord(0,0), RelCartCoord(0,0), iradius) {}
};

class SpirallingPolygonalInvader : public SpirallingInvader
{
    protected:
	void getAbsPoints(CartCoord* absPoints) const;

    public:
	int numPoints;
	RelCartCoord* points;

	CollisionPolygon cp;
	const CollisionObject& collObj() const;
	void setCollTrajectory(CartCoord startPos, RelCartCoord velocity);

	virtual void draw(SDL_Surface* surface, const View& view, View*
		boundView=NULL, bool noAA=false) const;

	SpirallingPolygonalInvader(int inumPoints, RelPolarCoord ipos, float
		ids=0, float idd=0, CartCoord ifocus=ARENA_CENTRE);

	SpirallingPolygonalInvader(
		const SpirallingPolygonalInvader& other);

	SpirallingPolygonalInvader& operator=(
		const SpirallingPolygonalInvader& other);

	virtual ~SpirallingPolygonalInvader();
};

class BasicInvader : public HPInvader, public SpirallingInvader, public
		     CircularInvader
{
    protected:
	Uint32 colour() const;
    public:
	bool super;

	virtual void draw(SDL_Surface* surface, const View& view, View*
		boundView=NULL, bool noAA=false) const;

	BasicInvader(int ihp, RelPolarCoord ipos, float ids=0, float idd=1,
		float iradius=5, bool isuper=false) : HPInvader(ihp),
	SpirallingInvader(ipos, ids, idd), CircularInvader(iradius),
	super(isuper)
	{}
};

class EggInvader : public BasicInvader
{
    public:
	EggInvader(RelPolarCoord ipos, float ids=0, bool super=false);
};
class KamikazeInvader : public BasicInvader
{
    protected:
	void doUpdate(int time);
	Uint32 colour() const;
	Uint32 innerColour() const;
    private:
	int kamikaze;
	int timer;

    public:
	KamikazeInvader(RelPolarCoord ipos, float ids=0, bool super=false);
};
class SplittingInvader : public BasicInvader
{
    private:
	float spawnDist;
    protected:
	void doUpdate(int time);
    public:
	SplittingInvader(RelPolarCoord ipos, float ids=0, bool super=false);
	void draw(SDL_Surface* surface, const View& view, View*
	    boundView=NULL, bool noAA=false) const;
};
class InfestingInvader : public HPInvader, public CircularInvader, public SpirallingInvader
{
    private:
	float healRate;
	float partialHP;
	float shownHP;
	bool infesting;
	bool super;
	int maxHP;
	Angle glowPhase;
    protected:
	Uint32 colour() const;
	void doUpdate(int time);
    public:
	Node* targetNode;

	void dodge() {};

	void fleeOnWin();

	InfestingInvader(Node* itargetNode, bool super=false);
	void draw(SDL_Surface* surface, const View& view, View*
	    boundView=NULL, bool noAA=false) const;
	void onDeath() const;
};

class CapturePod : public HPInvader, public CircularInvader, public SpirallingInvader
{
    private:
	Node* targetNode;
	bool super;
    protected:
	Uint32 colour() const;
	void doUpdate(int time);
    public:
	void dodge() {};

	bool evil() const;
	bool hitsYou() const;
	float hitsInvaders() const;

	void fleeOnWin();

	int killScore() const;

	float primeRate;
	CapturePod(Node* itargetNode, RelPolarCoord ipos, bool super=false);
};

class FoulEggLayingInvader : public HPInvader,
    public SpirallingPolygonalInvader
{
    protected:
	void doUpdate(int time);
	void setPoints(int time);
	float eggRadius;

	static float eggRate;
	static float layRadius;

	Uint32 colour() const;

    public:
	int hit(int weight);

	void draw(SDL_Surface* surface, const View& view, View*
	    boundView=NULL, bool noAA=false) const;

	FoulEggLayingInvader(RelPolarCoord ipos, float ids=0, int ihp=5);
};

#endif /* INC_INVADERS_H */
