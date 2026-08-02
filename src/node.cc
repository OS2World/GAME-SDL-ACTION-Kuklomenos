/*
 * Kuklomenos
 * Copyright (C) 2008-2009 Martin Bays <mbays@sdf.lonestar.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */

#include "node.h"
#include "invaders.h"
#include "random.h"
#include "sound.h"
#include "coords.h"

Node::Node(RelPolarCoord pos, float ds, NodeColour nodeColour, float spinRate,
	Angle spin, int pitch, float radius) :
    HPInvader(1,2),
    SpirallingPolygonalInvader(3, pos, ds, 0),
    sparkPoint(rani(3)),
    pitch(pitch),
    radius(radius),
    spinRate(spinRate),
    spin(spin),
    nodeColour(nodeColour),
    status(NODEST_NONE), primed(0), primeRate(0),
    targettingInfester(NULL), extractionProgress(0)
{
    setPoints();
    setSparks();
}

void Node::setPoints()
{
    for (int i = 0; i < 3; i++)
	points[i] = RelPolarCoord(spin + i*4.0/3, radius);
}

void Node::doUpdate(int time)
{
    SpirallingInvader::doUpdate(time);

    if (status == NODEST_YOU && primed < 1)
	primed += primeRate*0.001*time;
    else if (status == NODEST_NONE && primed > 0)
	primed = std::max(0.0, primed - 0.005*time);

    if (status == NODEST_NONE || status == NODEST_YOU)
    {
	if (fabs(angleDiff(spin*3, 0)) < fabs(time*spinRate*3))
	{
	    soundEvents.newEvent(pos, nodeHumChunk,
		    48, pitch, true);
	    if (status == NODEST_YOU)
	    {
		// harmonies
		soundEvents.newEvent(pos, nodeHumChunk,
			48, int(pitch*4/5), true);
		soundEvents.newEvent(pos, nodeHumChunk,
			48, int(pitch*2/3), true);
	    }
	}

	if (primed >= 1)
	{
	    // c.f. Node::glowPhase()
	    if (fabs(angleDiff(spin*6, 0)) < fabs(time*spinRate*6))
		soundEvents.newEvent(pos, primedChunk, 48, pitch);
	}

	spin += time*spinRate;
	setPoints();
    }
    else if (status == NODEST_EVIL)
    {
	// turn to point directly away from centre:
	spin -= (angleDiff(0,spin*3)*time/3000);
	setPoints();
    }
}

int Node::extract(int time, bool hasCyan)
{
    int extracted = 0;

    extractionProgress += (hasCyan ? 2.5 : 1.0)*0.001*time;

    while (extractionProgress > 1)
    {
	extractionProgress--;
	extracted++;
	soundEvents.newEvent(pos, sparkChunk,
		48 + int(16*gaussian()),
		pitch + int(200*gaussian()));
    }

    if (extracted)
	setSparks();

    return extracted;
}

void Node::setSparks()
{
    const int numSparkVertices = 3+rani(3);
    sparkVertices.clear();
    sparkPoint = rani(3);
    RelPolarCoord vertex = RelPolarCoord(0, dist(points[0]));
    sparkVertices.push_back(vertex);
    for (int i = 1; i < numSparkVertices - 1; i++)
    {
	vertex = RelPolarCoord(
		vertex.angle + (-0.1+ranf(0.2))*i,
		vertex.dist -
		(0.5+ranf(0.5))*(vertex.dist/(numSparkVertices-i)));
	sparkVertices.push_back(vertex);
    }
    sparkVertices.push_back(RelPolarCoord(0,0));
}

CartCoord Node::getSparkVertex(int v) const
{
    return cpos() + sparkVertices[v].rotated(
	    pos.angle + angle(points[sparkPoint]));
}

Angle Node::glowPhase() const
{
    return 6*spin;
}

Uint32 Node::colour() const
{
    Uint32 col;
    switch (nodeColour)
    {
	case NODEC_RED:
	    col = 0x01000000;
	    break;
	case NODEC_YELLOW:
	    col = 0x01010000;
	    break;
	case NODEC_GREEN:
	    col = 0x00010000;
	    break;
	case NODEC_BLUE:
	    col = 0x00000100;
	    break;
	case NODEC_PURPLE:
	    col = 0x01000100;
	    break;
	case NODEC_CYAN:
	    col = 0x00010100;
	    break;
	default:
	    col = 0x01010100;
    }

    int intensity = 0;

    if (status == NODEST_NONE || status == NODEST_YOU)
    {
	if ( primed >= 1 ) 
	    intensity = 0xff - (int)(0x30 * (1 - glowPhase().sinf()));
	else
	{
	    intensity = (status == NODEST_NONE ? 0x80 : 0xbf);
	    intensity += int( (0xff - intensity) *
		    std::max(0.0, 1 - ( fabs(angleDiff(spin*3, 0)) * 2 )) );
	}
    }
    else
	intensity = status == NODEST_DESTROYED ? 0x20 :
	    status == NODEST_EVIL ? 0xbf :
	    0x00;
    return col*0xff + intensity;
}

Uint32 Node::innerColour() const
{
    return 0;
}

void Node::draw(SDL_Surface* surface, const View& view, View*
	boundView, bool noAA) const
{

    if (primed > 0)
    {
	// prongs
	float prongStage = std::min(1.0f, primed*3);
	float prongStart = std::max(0.0f, primed*20 - 19);

	if (prongStage < 1)
	    Circle(cpos(), 2.0*(1-prongStage),
		    0x0000c000 + (int)(0x60*(1-prongStage)),
		    true).draw(surface, view, boundView, noAA);

	if (prongStart < 1)
	    for (int i = 0; i < 3; i++)
	    {
		Line(cpos() + points[i].rotated(pos.angle) * prongStart,
			cpos() + points[i].rotated(pos.angle) * prongStage,
			0x00ffff40+(int)(0x60*prongStage)
		    ).draw(surface, view, boundView, noAA);
	    }

	CartCoord tpoints[3];
	for (int i = 0; i < 3; i++)
	    tpoints[i] = cpos() + points[i].rotated(pos.angle) * primed;

	Polygon(tpoints, 3, 0x00ffff00 +
		((primed >= 1) ? (0xff - (int)(0x30 * (1 - glowPhase().sinf()))) :
		 (int)(0x40 + 0x60*primed)),
		true).draw(surface, view, boundView, noAA);
	if (primed < 1)
	    Polygon(tpoints, 3, 0x00ffffa0,
		    false).draw(surface, view, boundView, noAA);
    }

    SpirallingPolygonalInvader::draw(surface, view, boundView, noAA);

    if (status == NODEST_EVIL)
    {
	static const float glintSep = ARENA_RAD/15.0;
	float glintDist = pos.dist + glintSep*extractionProgress;
	while (glintDist < ARENA_RAD)
	{
	    const CartCoord glintPos = ARENA_CENTRE + RelPolarCoord(pos.angle,
		    glintDist);

	    Pixel(glintPos, 0x00ffffff
		 ).draw(surface, view, boundView, noAA);

	    glintDist += glintSep;
	}
	if (extractionProgress > 0.8)
	{
	    const int numVertices = sparkVertices.size();
	    const int sparkFrom = (int)(
		    (extractionProgress-0.8)*numVertices/0.2);
	    if (sparkFrom < numVertices - 1)
		Line(getSparkVertex(sparkFrom),
			getSparkVertex(sparkFrom + 1),
			0x00ffffe0
		    ).draw(surface, view, boundView, noAA);
	    else
		Pixel(getSparkVertex(sparkFrom), 0x00ffffff
		     ).draw(surface, view, boundView, noAA);

	    if (sparkFrom > 0 && sparkFrom < numVertices)
		Line(getSparkVertex(sparkFrom - 1),
			getSparkVertex(sparkFrom),
			0x00ffff90
		    ).draw(surface, view, boundView, noAA);
	}
    }
}

bool Node::infest(InfestingInvader* inv)
{
    if (status == NODEST_DESTROYED || status == NODEST_EVIL)
	return false;
    if (status == NODEST_YOU)
    {
	uncapture();
	return false;
    }
    status = NODEST_EVIL;

    return true;
}
void Node::uninfest()
{
    if (status == NODEST_EVIL)
	status = NODEST_NONE;
}
bool Node::capture(CapturePod* pod)
{
    if (status != NODEST_NONE)
	return false;

    status = NODEST_YOU;
    primeRate = pod->primeRate;
    return true;
}
void Node::uncapture()
{
    status = NODEST_NONE;
}

int Node::hit(int weight)
{
    if (weight >= 3 && status == NODEST_YOU && primed >= 1)
    {
	uncapture();
	status = NODEST_DESTROYED;
	primed = 0;
	return 3;
    }

    return weight;
}
