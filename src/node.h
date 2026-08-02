#ifndef INC_NODE_H
#define INC_NODE_H

#include <vector>

#include "invaders.h"
#include "coords.h"

enum NodeStatus
{
    NODEST_NONE,
    NODEST_EVIL,
    NODEST_YOU,
    NODEST_DESTROYED
};
enum NodeColour
{
    NODEC_BLUE,
    NODEC_PURPLE,
    NODEC_CYAN,
    NODEC_RED,
    NODEC_YELLOW,
    NODEC_GREEN
};

class Node : public HPInvader, public SpirallingPolygonalInvader
{
    private:
	std::vector<RelPolarCoord> sparkVertices;
	int sparkPoint;
	void setSparks();
	void setPoints();
	CartCoord getSparkVertex(int v) const;
	Angle glowPhase() const;
    protected:
	void doUpdate(int time);
	Uint32 colour() const;
	Uint32 innerColour() const;
    public:
	int pitch;
	float radius;

	float spinRate;
	Angle spin;

	NodeColour nodeColour;
	NodeStatus status;

	float primed;
	float primeRate;

	CapturePod* capturer;
	InfestingInvader* targettingInfester;
	bool infest(InfestingInvader* inv);
	void uninfest();
	bool capture(CapturePod* pod);
	void uncapture();

	int hit(int weight);
	int die() { return 0; }
	bool dead() const { return false; }

	float extractionProgress;

	int extract(int time, bool hasCyan=false);

	void draw(SDL_Surface* surface, const View& view, View*
		boundView=NULL, bool noAA=false) const;

	Node(RelPolarCoord pos, float ds, NodeColour nodeColour,
		float spinRate=0, Angle spin=0, int pitch=1000, float
		radius=6);
};

#endif /* INC_NODE_H */
