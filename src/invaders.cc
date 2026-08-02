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

#include <cstdlib>
#include <algorithm>

#include "invaders.h"
#include "state.h"
#include "geom.h"
#include "random.h"
#include "collision.h"

using namespace std;

void Invader::update(int time)
{
    CartCoord startPos = cpos();
    doUpdate(time);
    RelCartCoord velocity = (cpos()-startPos)*(1.0/time);
    setCollTrajectory(startPos, velocity);
}

void Invader::spawnInvader(Invader* invader)
{
    spawns.push_back(invader);
}

const CollisionObject& CircularInvader::collObj() const
{
    return cc;
}
void CircularInvader::setCollTrajectory(CartCoord startPos, RelCartCoord velocity)
{
    cc.startPos = startPos;
    cc.velocity = velocity;
}

Uint32 Invader::innerColour() const
{
    // same as colour(), but with alpha set to 0x60:
    return (colour() >> 8 << 8) + 0x60;
}

const CollisionObject& SpirallingPolygonalInvader::collObj() const
{
    return cp;
}
void SpirallingPolygonalInvader::setCollTrajectory(CartCoord startPos,
	RelCartCoord velocity)
{
    cp.startPos = startPos;
    cp.velocity = velocity;
    cp.angle = pos.angle;
}

void KamikazeInvader::doUpdate(int time)
{
    BasicInvader::doUpdate(time);

    if (kamikaze == 0)
    {
	if (rani(3000) <= time && rani(ARENA_RAD/2) > pos.dist)
	    kamikaze = 1;
    }
    else
	if ((timer+=time) > 200)
	{
	    timer = 0;
	    switch (kamikaze)
	    {
		case 1:
		    if (ds == 0)
			kamikaze = 2;
		    else
			ds += (ds > 0 ? -1 : 1);
		    break;
		case 2:
		    if (dd < 8)
			dd++;
	    }
	}
}

void SplittingInvader::doUpdate(int time)
{
    BasicInvader::doUpdate(time);

    if (pos.dist <= spawnDist + ARENA_RAD/3)
	radius = 5.0 + 2.0 * cosf(
		(PI/2)*(1 - (pos.dist-spawnDist)/(ARENA_RAD/3)));

    if (pos.dist <= spawnDist)
    {
	if (hp > 1)
	    for (int dds = -1; dds <= 1; dds += 2)
		spawnInvader(new EggInvader(pos, ds+dds));
	else
	    spawnInvader(new EggInvader(pos, ds));
	die();
    }
}

void SplittingInvader::draw(SDL_Surface* surface, const View& view,
	View* boundView, bool noAA) const
{
    const float eggRad = 
	5.0 * sinf((PI/2) * (max(0.0f, 2 - pos.dist/spawnDist)));
    Circle(cpos(), eggRad, 0xff000000 + (int)(0xb0 * (eggRad/5.0)),
	    true).draw( surface, view, boundView, noAA);

    Circle(cpos(), radius, (colour() >> 8 << 8) + (int)(0x60 - 0x10 * eggRad),
		true).draw(surface, view, boundView, noAA);
    Circle(cpos(), radius, colour()).draw(surface, view, boundView, noAA);

    if (super)
	drawSuper(surface, view, boundView, noAA);
}

Uint32 BasicInvader::colour() const
{
    switch(hp)
    {
	case 3: return 0x00ff00ff;
	case 2: return 0xffff00ff;
	default: return 0xff0000ff;
    }
}

Uint32 KamikazeInvader::colour() const
{
    switch(hp)
    {
	case 2: return (kamikaze ? 0xffff00ff : 0xa0a000ff);
	default: return (kamikaze ? 0xff0000ff : 0xa00000ff);
    }
}

Uint32 KamikazeInvader::innerColour() const
{
    switch(hp)
    {
	case 2: return (kamikaze ? 0xffff00a0 : 0xa0a00060);
	default: return (kamikaze ? 0xff0000a0 : 0xa0000060);
    }
}

Uint32 FoulEggLayingInvader::colour() const
{
    return 0xffa000ff;
    /*
    switch(hp)
    {
	case 5: return 0xff00ffff;
	case 4: return 0xcf00cfff;
	case 3: return 0xaf00afff;
	case 2: return 0x8f008fff;
	default: return 0x6f006fff;
    }
    */
}


bool HPInvader::dead() const
{
    return hp <= 0;
}

CartCoord SpirallingInvader::cpos() const
{
    return focus + pos;
}

int HPInvader::hit(int weight)
{
    int used = min(weight, hp+armour);
    hp -= max(0, used-armour);
    return used;
}

int HPInvader::die()
{
    return hit(hp+armour);
}

void SpirallingInvader::doUpdate(int time)
{
    pos.angle += ds*0.0001*(100/pos.dist)*time;
    pos.dist += dd*-0.0075*time;
}

void SpirallingInvader::fleeOnWin()
{
    dd *= -3;
}

void SpirallingInvader::dodge()
{
    ds = -ds;
}

void CircularInvader::draw(SDL_Surface* surface, const View& view, View*
	boundView, bool noAA) const
{
    Circle(cpos(), radius, innerColour(), true).draw(surface,
	    view, boundView, noAA);
    Circle(cpos(), radius, colour()).draw(surface, view, boundView, noAA);
}

void SpirallingInvader::drawSuper(SDL_Surface* surface, const View& view,
	View* boundView, bool noAA) const
{
    Line(cpos() + RelPolarCoord(pos.angle, 1.5),
	    cpos() + RelPolarCoord(pos.angle, -1.5),
	    0xffffffff).draw(surface, view, boundView, noAA);
    Line(cpos() + RelPolarCoord(pos.angle+1, 1.5),
	    cpos() + RelPolarCoord(pos.angle+1, -1.5),
	    0xffffffff).draw(surface, view, boundView, noAA);
}

void BasicInvader::draw(SDL_Surface* surface, const View& view, View*
	boundView, bool noAA) const
{
    CircularInvader::draw(surface, view, boundView, noAA);
    if (super)
	drawSuper(surface, view, boundView, noAA);
}


SpirallingPolygonalInvader::SpirallingPolygonalInvader(int inumPoints,
	RelPolarCoord ipos, float ids, float idd, CartCoord ifocus) :
    SpirallingInvader(ipos, ids, idd, ifocus),
    numPoints(inumPoints),
    points(new RelCartCoord[numPoints]),
    cp(inumPoints, points)
{
}

SpirallingPolygonalInvader::SpirallingPolygonalInvader(
	const SpirallingPolygonalInvader& other) :
    SpirallingInvader(other.pos, other.ds, other.dd, other.focus),
    numPoints(other.numPoints),
    points(new RelCartCoord[numPoints]),
    cp(other.numPoints, points)
{
    for (int i = 0; i < numPoints; i++)
	points[i] = other.points[i];
}

SpirallingPolygonalInvader& SpirallingPolygonalInvader::operator=(
	const SpirallingPolygonalInvader& other)
{
    if (this != &other)
    {
	pos = other.pos; ds = other.ds; dd = other.dd; focus = other.focus;
	numPoints = other.numPoints;
	points = new RelCartCoord[numPoints];
	cp = other.cp;
	for (int i = 0; i < numPoints; i++)
	    points[i] = other.points[i];
    }
    return *this;
}

SpirallingPolygonalInvader::~SpirallingPolygonalInvader()
{
    delete[] points;
}

void SpirallingPolygonalInvader::getAbsPoints(CartCoord* absPoints) const
{
    for (int i=0; i<numPoints; i++)
	absPoints[i] = cpos() + points[i].rotated(pos.angle);
}

void SpirallingPolygonalInvader::draw(SDL_Surface* surface, const View& view,
	View* boundView, bool noAA) const
{
    CartCoord* absPoints = new CartCoord[numPoints];

    getAbsPoints(absPoints);

    const Uint32 innerCol = innerColour();
    if (innerCol != 0)
	Polygon(absPoints, numPoints, innerCol, true).draw(
		surface, view, boundView, noAA);

    Polygon(absPoints, numPoints, colour()).draw(surface, view, boundView, noAA);

    delete[] absPoints;
}

void FoulEggLayingInvader::draw(SDL_Surface* surface, const View& view,
	View* boundView, bool noAA) const
{
    SpirallingPolygonalInvader::draw(surface, view, boundView, noAA);

    CartCoord* absPoints = new CartCoord[numPoints];
    getAbsPoints(absPoints);

    if (eggRadius > 0)
	Circle(absPoints[4] + (
		    RelCartCoord(0, -eggRadius).rotated(pos.angle)),
		eggRadius, 0xff0000ff).draw(surface, view, boundView, noAA);

    delete[] absPoints;
}


void FoulEggLayingInvader::setPoints(int time)
{
    // animation code, currently not non-trivially used - TODO: remove it
    for (int i=0; i < numPoints; i++)
    {
	//const int x = 2+3*hp;
	//const int y = 3+1*hp;
	const int x = 6;
	const int y = 4;
	RelCartCoord aimed = 
	    (i == 0) ? RelCartCoord(x, -y) :
	    ((i == 1) ? RelCartCoord(x, y) :
	    ((i == 2) ? RelCartCoord(-x, y) :
	    ((i == 3) ? RelCartCoord(-x, -y) :
	    (RelCartCoord(0, -2*y)))));
	if (time == -1)
	{
	    // Just set:
	    points[i] = aimed;
	}
	else
	{
	    RelCartCoord d = aimed - points[i];
	    if (!(d.dx == 0 && d.dy == 0))
		points[i] += d * min(1.0f, ((float)time/50)/d.lengthsq());
	}
    }
}

float FoulEggLayingInvader::eggRate = 0.0002;
float FoulEggLayingInvader::layRadius = 5;
void FoulEggLayingInvader::doUpdate(int time)
{
    SpirallingPolygonalInvader::doUpdate(time);

    eggRadius += time*eggRate;

    if (eggRadius >= layRadius)
    {
	RelPolarCoord p(pos.angle, pos.dist + points[4].dy - eggRadius);
	Invader* egg = new EggInvader(p, ds);
	spawnInvader(egg);

	eggRadius = 0;
    }

    //setPoints(time);
}

int FoulEggLayingInvader::hit(int weight)
{
    eggRadius = 0;
    if (weight >= 3)
    {
	hp = 0;
	return 3;
    }
    return weight;
}

EggInvader::EggInvader(RelPolarCoord ipos, float ids, bool super) :
    BasicInvader(1, ipos, ids, 3+super*1.5, 5, super)
{}
KamikazeInvader::KamikazeInvader(RelPolarCoord ipos, float ids, bool super) :
    BasicInvader(2, ipos, ids, 2+super*1.5, 6, super), kamikaze(0), timer(0)
{}
SplittingInvader::SplittingInvader(RelPolarCoord ipos, float ids, bool super) :
    BasicInvader(3, ipos, ids, 1+super*1.5, 7, super)
{
    spawnDist = ARENA_RAD/10 + rani(4*ARENA_RAD/10);
}
InfestingInvader::InfestingInvader(Node* itargetNode, bool super) :
    HPInvader(3,1), CircularInvader(6),
    SpirallingInvader(
	    RelPolarCoord(itargetNode->pos.angle + ranf(0.5)-0.25, ARENA_RAD),
	    0, 0.5 + super*0.2),
    healRate(0.1 + super*0.025),
    partialHP(0), shownHP(3), infesting(false),
    super(super),
    maxHP(3),
    glowPhase(0),
    targetNode(itargetNode)
{
    targetNode->targettingInfester = this;
}
CapturePod::CapturePod(Node* itargetNode, RelPolarCoord ipos, bool super) :
    HPInvader(super ? 3 : 1), CircularInvader(2),
    SpirallingInvader(
	    ipos,
	    0, -1.25 + super*-0.5),
    targetNode(itargetNode),
    super(super),
    primeRate(super ? 1.0/25 : 1.0/30)
{}

void InfestingInvader::doUpdate(int time)
{
    SpirallingInvader::doUpdate(time);

    pos.angle = targetNode->pos.angle;

    if (!infesting && pos.dist <= targetNode->pos.dist)
    {
	if (targetNode->infest(this))
	{
	    pos.dist = targetNode->pos.dist;
	    dd = 0;
	    infesting = true;
	}
	else
	    die();
    }
    if (infesting && targetNode->status != NODEST_EVIL)
    {
	// Infested node has been recaptured
	infesting = false;
	die();
    }
    if (dd > 0 && targetNode->status == NODEST_DESTROYED)
    {
	dd *= -3;
    }
    if (hp < maxHP)
    {
	partialHP += healRate*0.001*time;
	if (partialHP >= 1)
	{
	    hp++;
	    partialHP = 0;
	}
    }
    if (infesting && hp == maxHP)
    {
	partialHP += healRate*0.25*0.001*time;
	if (partialHP >= 1)
	{
	    // Become invulnerable to all but CapturePods:
	    hp++;
	    partialHP = 0;
	    armour = 3;
	    cc.radius += 2;
	}
    }
    if (hp == maxHP+1)
	glowPhase += time*0.003;

    const float totalHP = hp + partialHP;
    if (shownHP < totalHP)
	shownHP = std::min(totalHP, shownHP +
		std::max(0.004f, 0.01f*(totalHP - shownHP)) * time);
    else
	shownHP = std::max(totalHP, shownHP +
		std::min(-0.004f, 0.01f*(totalHP - shownHP)) * time);
}

void InfestingInvader::fleeOnWin()
{
    dd = -3;
    if (infesting)
	targetNode->uninfest();
    targetNode->targettingInfester = NULL;
    infesting = false;
}

void CapturePod::doUpdate(int time)
{
    SpirallingInvader::doUpdate(time);

    pos.angle = targetNode->pos.angle;

    if (pos.dist >= targetNode->pos.dist)
    {
	targetNode->capture(this);
	die();
    }
}

bool CapturePod::evil() const { return false; }
bool CapturePod::hitsYou() const { return false; }
float CapturePod::hitsInvaders() const
{
	return radius;
}
int CapturePod::killScore() const { return 0; }

void CapturePod::fleeOnWin()
{
}

Uint32 InfestingInvader::colour() const
{
    return 0x0000c0ff;
}
Uint32 CapturePod::colour() const
{
    return 0x0000c0ff + ((0xff00-0xc000)*(hp-1)/2);
}
void InfestingInvader::draw(SDL_Surface* surface, const View& view, View*
	boundView, bool noAA) const
{
    Circle(cpos(), radius, (colour() >> 8 << 8) + 0xc0,
	    true).draw(surface, view, boundView, noAA);

    // healing
    if (shownHP < 3)
    {
	Circle(cpos(), std::min(1.0f, 3 - shownHP)*4*radius/5,
		0xffff0050,
		true).draw(surface, view, boundView, noAA);
	Circle(cpos(), std::min(1.0f, 3 - shownHP)*4*radius/5,
		0x808000ff,
		false).draw(surface, view, boundView, noAA);
    }
    if (shownHP < 2)
    {
	Circle(cpos(), std::min(1.0f, 2 - shownHP)*3*radius/5,
		0xff000070,
		true).draw(surface, view, boundView, noAA);
	Circle(cpos(), std::min(1.0f, 2 - shownHP)*3*radius/5,
		0x900000ff,
		false).draw(surface, view, boundView, noAA);
    }

    // boundary
    Circle(cpos(), radius, colour(),
	    false).draw(surface, view, boundView, noAA);

    // partial shield
    if (infesting && shownHP > maxHP && shownHP < maxHP + 1)
	Circle(cpos(), radius * (shownHP - maxHP),
		0x01010100*(int)(0x40 + 0x90 * (shownHP - maxHP)) + 0xff,
		false).draw(surface, view, boundView, noAA);

    // full shield
    if (shownHP >= maxHP+1)
	Circle(cpos(), radius+2, 0xffffff00 + (0xff -
		    (int)(0x40 * (1 + glowPhase.sinf()))),
	    false).draw(surface, view, boundView, noAA);

    if (super)
	drawSuper(surface, view, boundView, noAA);
}
void InfestingInvader::onDeath() const
{
    if (infesting)
	targetNode->uninfest();
    targetNode->targettingInfester = NULL;
}

FoulEggLayingInvader::FoulEggLayingInvader(RelPolarCoord ipos, float ids,
	int ihp) :
    HPInvader(ihp), SpirallingPolygonalInvader(5, ipos, ids, 0),
    eggRadius(0)
{
    setPoints(-1);
}
