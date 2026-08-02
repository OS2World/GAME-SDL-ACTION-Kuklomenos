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

#include <vector>
#include <set>
#include <algorithm>
#include <cstdio>
#include <SDL/SDL.h>
#include <SDL_gfxPrimitivesDirty.h>

#include "state.h"
#include "shot.h"
#include "player.h"
#include "geom.h"
#include "gfx.h"
#include "random.h"
#include "settings.h"
#include "conffile.h"
#include "node.h"
#include "ai.h"
#include "sound.h"

const int pentatonicScale[14] = { 0, 2, 5, 7, 9, 12, 14, 17, 19, 21, 24, 26, 29 };
const int majorScale[14] = { 0, 2, 4, 5, 7, 9, 11, 12, 14, 16, 17, 19, 21, 23 };
const int minorScale[14] = { 0, 2, 3, 5, 7, 8, 10, 12, 14, 15, 19, 20, 22 };
const int* scales[3] = { pentatonicScale, majorScale, minorScale };

// beatRatios: beat frequency of inner is beatRatio * frequency of outer
const float beatRatios[5] = { 1.0/1.0, 3.0/2.0, 4.0/3.0, 5.0/4.0, 5.0/3.0 };

GameState::GameState(int speed) :
    targettedNode(NULL), mutilationWave(-1), preMutilationPhase(0),
    extractPreMutCutoff(350), freeViewMode(false),
    extracted(0), extractDecayRate(0.0002), you(), zoomdist(0), invaderRate(0),
    speed(speed), extractMax(500), end(END_NOT), ai(NULL)
{
    setRating();
    invaderCooldown = invaderRate;

    std::vector<NodeColour> colours;
    for (int i = 0; i < 6; i++)
	colours.push_back( NodeColour(i) );

    float baseAngleInner = 0;
    float baseAngleOuter = 0;
    int innerDir = 1;
    if (this->rating > 5.0)
    {
	// Let's mix things up a bit...
	colours = randomSort(colours);
	baseAngleInner = ranf(4.0);
	baseAngleOuter = ranf(4.0);
	if (rani(2) == 0)
	    innerDir = -1;
    }

    const int* scale = scales[rani(3)];
    const int key = rani(12) - 8;
    const float beatRatio = beatRatios[rani(5)];

    set<int> notesUsed;

    int i=0;
    for (std::vector<NodeColour>::iterator it = colours.begin();
	    it != colours.end();
	    i++, it++)
    {
	const NodeColour colour = *it;

	int note = rani(14);
	while (notesUsed.find(note) != notesUsed.end())
	    note = rani(14);
	notesUsed.insert(note);

	const int pitch = int(1000 *
		pow(2, - (key+scale[note]) / 12.0) );
	if (i < 3)
	    nodes.push_back(
		    Node(RelPolarCoord(baseAngleInner+2+i*4.0/3,
			    ARENA_RAD * 5/9),
			innerDir*1.0, colour,
			beatRatio*1.0/3000, rani(16)/4.0, pitch));
	else
	    nodes.push_back(
		    Node(RelPolarCoord(baseAngleOuter+i*4.0/3,
			    ARENA_RAD * 2/3),
			-innerDir*1.0, colour,
			1.0/3000, rani(16)/4.0, pitch));
    }
}

bool isNullInvp(Invader* p)
{
    return (p == NULL);
}

bool GameState::youHaveNode(NodeColour colour)
{
    for (std::vector<Node>::iterator it = nodes.begin();
	    it != nodes.end();
	    it++)
	if (it->status == NODEST_YOU && it->nodeColour == colour)
	    return true;
    return false;
}
bool GameState::youHaveShotNode(int type)
{
    switch (type)
    {
	case 0: return youHaveNode(NODEC_GREEN); break;
	case 1: return youHaveNode(NODEC_YELLOW); break;
	case 2: return youHaveNode(NODEC_RED); break;
	case 3: return youHaveNode(NODEC_BLUE); break;
	default: return false;
    }
}
bool GameState::evilHasNode(NodeColour colour)
{
    for (std::vector<Node>::iterator it = nodes.begin();
	    it != nodes.end();
	    it++)
	if (it->status == NODEST_EVIL && it->nodeColour == colour)
	    return true;
    return false;
}

int GameState::shotHeat(int type)
{
    if (type == 3)
	return you.shootMaxHeat-1;

    int baseHeat;
    const bool super = youHaveShotNode(type);
    switch (type)
    {
	case 0: baseHeat = 4000; break;
	case 1: baseHeat = 7000; break;
	case 2: baseHeat = 10000; break;
	default: baseHeat = 0;
    }
    return super ? 7*baseHeat/10 : baseHeat;
}
int GameState::shotDelay(int type)
{
    int delay;
    const bool super = youHaveShotNode(type);
    switch (type)
    {
	case 0: delay = super ? 100 : 200; break;
	case 1: delay = super ? 250 : 300; break;
	case 2: delay = super ? 300 : 400; break;
	case 3: delay = 10000; break;
	default: delay = 0;
    }
    return delay;
}

void GameState::updateMutilation(int time)
{
    if (mutilationWave > 0)
    {
	mutilationWave = std::max(0.0f,
		mutilationWave - time*0.001f*ARENA_RAD/3);
	if (mutilationWave < 0)
	    mutilationWave = 0;

	for (std::vector<Node>::iterator it = nodes.begin();
		it != nodes.end();
		it++)
	    if (it->pos.dist >= mutilationWave-it->radius)
		if (it->status != NODEST_NONE &&
			it->status != NODEST_DESTROYED)
		{
		    it->status = NODEST_NONE;

		    soundEvents.newEvent(it->cpos() - ARENA_CENTRE,
			    mutChunk, 128, it->pitch,
			    true);
		}

	for (std::vector<Invader*>::iterator it = invaders.begin();
		it != invaders.end();
		it++)
	    if (((*it)->cpos() - ARENA_CENTRE).lengthsq() >=
		    mutilationWave*mutilationWave)
	    {
		int damage = (*it)->die();
		if ((*it)->dead())
		{
		    soundEvents.newEvent((*it)->cpos() - ARENA_CENTRE,
			    invDieChunk,
			    128, 500+100*damage + int(200*gaussian()),
			    true);
		    soundEvents.newEvent((*it)->cpos() - ARENA_CENTRE,
			    mutChunk,
			    128, 500+100*damage + int(200*gaussian()),
			    true);
		}
	    }

	for (std::vector<Shot>::iterator it = shots.begin();
		it != shots.end();
		it++)
	    if ((it->pos - ARENA_CENTRE).lengthsq() >=
		    mutilationWave*mutilationWave)
	    {
		it->die();
		deadShots = true;
	    }

	while (!you.dead && mutilationWave <= you.radius())
	{
	    soundEvents.newEvent(0, shieldChunk,
		    128, 1000 + 100*int(you.shield)); 
	    you.shield -= 1;
	    if (you.shield < 0)
	    {
		you.dead = true;
		soundEvents.newEvent(0, mutChunk,
			128, 2000); 
	    }
	    else
		soundEvents.newEvent(0, mutChunk,
			128, 1000 + 100*int(you.shield)); 
	    end = END_EXTRACTED;
	}
    }
    else if (extracted > extractPreMutCutoff && mutilationWave == -1)
    {
	preMutilationPhase += time*0.002;
	if (preMutilationPhase < time*0.002)
	{
	    float stage = float(extracted - extractPreMutCutoff) /
		(extractMax - extractPreMutCutoff );
	    soundEvents.newEvent(0, mutChunk,
		    int(24 + 48 * stage),
		    int(1400 - 400 * stage));
	}
    }
}

void GameState::update(int time, bool noInput)
{
    if (time <= 0)
	return;

    deadShots = false;

    if (ai)
	ai->update(time);

    updateObjects(time);

    if (freeViewMode)
    {
	if (!noInput)
	    handleFreeViewInput(time);
    }
    else
    {
	if ( (!noInput || ai) && !you.dead)
	    handleGameInput(time);
	updateZoom(time);
    }

    evilAI(time);

    soundEvents.update(RelPolarCoord(you.aim.angle, zoomdist));

    cleanup();
}

void GameState::updateObjects(int time)
{
    you.update(time, youHaveNode(NODEC_CYAN));

    if (!end && you.dead)
	end = END_DEAD;

    extracted -= extractDecayRate*(end == END_WIN ? 10 : 1)*time;
    extracted = std::max(0.0, extracted);

    int destroyedCount = 0;
    for (std::vector<Node>::iterator it = nodes.begin();
	    it != nodes.end();
	    it++)
    {
	it->update(time);

	if (it->status == NODEST_EVIL)
	{
	    extracted += it->extract(time, evilHasNode(NODEC_CYAN));
	    if (extracted > extractMax && mutilationWave == -1)
	    // initiate wave of mutilation
		mutilationWave = ARENA_RAD;
	}
	else if (it->status == NODEST_DESTROYED)
	    destroyedCount++;
    }
    if (destroyedCount >= 4 && end == END_NOT)
    {
	// you win - set invaders fleeing away
	end = END_WIN;
	for (std::vector<Invader*>::iterator it = invaders.begin();
		it != invaders.end();
		it++)
	    (*it)->fleeOnWin();
    }

    updateMutilation(time);

    std::vector<Invader*> topush;

    for (std::vector<Invader*>::iterator it = invaders.begin();
	    it != invaders.end();
	    it++)
    {
	Invader* inv = *it;
	inv->update(time);

	while (!inv->spawns.empty())
	{
	    Invader* spawned = inv->spawns.back();
	    inv->spawns.pop_back();
	    topush.push_back(spawned);
	}

	if (inv->hitsYou() &&
		inv->collObj().circleIntersects(ARENA_CENTRE, you.radius()))
	{
	    inv->die();
	    if (!you.dead)
	    {
		soundEvents.newEvent(inv->cpos()-ARENA_CENTRE, shieldChunk,
			96, 1000 + 100*int(you.shield)); 
		you.shield -= 1;
		if (you.shield < 0 && !settings.invuln)
		    you.dead = true;
	    }
	}

	// check for leaving arena
	if (((inv)->cpos() - ARENA_CENTRE).lengthsq() >=
		ARENA_RAD*ARENA_RAD)
	    inv->die();
    }

    // check for collisions between invaders
    // (note that this must come after updating all invaders, since we need
    // the collision objects to be set)
    for (std::vector<Invader*>::iterator it = invaders.begin();
	    it != invaders.end();
	    it++)
    {
	Invader* inv = *it;
	if (inv->hitsInvaders() > 0)
	{
	    Invader* hitInvader = NULL;
	    const float r = inv->hitsInvaders();
	    for (std::vector<Invader*>::iterator it2 = invaders.begin();
		    it2 != invaders.end();
		    it2++)
	    {
		if (*it2 == inv)
		    continue;
		if ((*it2)->collObj().circleIntersects(inv->cpos(), r))
		{
		    hitInvader = *it2;
		    break;
		}
	    }

	    if (hitInvader != NULL)
	    {
		int damage = hitInvader->die();
		if (hitInvader->dead())
		{
		    soundEvents.newEvent(hitInvader->cpos() - ARENA_CENTRE,
			    invDieChunk,
			    96, 800+200*damage + int(100*gaussian()));
		    you.score += hitInvader->killScore();
		}
		inv->hit(damage);
	    }
	}
    }

    for (std::vector<Shot>::iterator it = shots.begin();
	    it != shots.end();
	    it++)
    {
	float hitTime = -1;
	Invader* hitInvader = NULL;

	for (std::vector<Invader*>::iterator invit = invaders.begin();
		invit != invaders.end();
		invit++)
	{
	    if (!(*invit)->hitsShots())
		continue;

	    RelCartCoord v = it->vel;
	    float t = (*invit)->collObj().pointHits(it->pos, v, time);
	    if (t >= 0 && (hitTime == -1 || t < hitTime))
	    {
		hitTime = t;
		hitInvader = *invit;
	    }
	}
	for (std::vector<Node>::iterator nodeit = nodes.begin();
		nodeit != nodes.end();
		nodeit++)
	{
	    if (nodeit->primed < 1)
		continue;

	    RelCartCoord v = it->vel;
	    float t = nodeit->collObj().pointHits(it->pos, v, time);
	    if (t >= 0 && (hitTime == -1 || t < hitTime))
	    {
		hitTime = t;
		hitInvader = &*nodeit;
	    }
	}

	if (hitInvader != NULL)
	{
	    int damage = hitInvader->hit(it->weight);
	    if (hitInvader->dead())
	    {
		soundEvents.newEvent(it->pos - ARENA_CENTRE, invDieChunk,
			96, 800+200*damage + int(100*gaussian()));
		you.score += hitInvader->killScore();
	    }
	    else
		soundEvents.newEvent(it->pos - ARENA_CENTRE, invHitChunk,
			96, 800+200*damage + int(100*gaussian()));
	    it->hit(damage);
	    if (Shot::is_dead(*it))
		deadShots = true;
	}

	it->update(time);
	RelCartCoord d = it->pos - ARENA_CENTRE;
	if (d.lengthsq() >= ARENA_RAD*ARENA_RAD)
	{
	    it->dead = 1;
	    deadShots = true;
	}
    }

    // add newly spawned invaders
    for (std::vector<Invader*>::iterator it = topush.begin();
	    it != topush.end();
	    it++)
    {
	invaders.push_back(*it);
    }
}

void GameState::handleGameInput(int time)
{
    bool keyRotLeft;
    bool keyRotRight;
    bool keyDeAim;
    bool keyDeZoom;
    bool keyShoot1;
    bool keyShoot2;
    bool keyShoot3;
    bool keyShootPod;

    if (!ai)
    {
	keyRotLeft = settings.keybindings[C_LEFT].isPressed();
	keyRotRight = settings.keybindings[C_RIGHT].isPressed();
	keyDeAim = settings.keybindings[C_DEAIM].isPressed();
	keyDeZoom = settings.keybindings[C_DEZOOM].isPressed();
	keyShoot1 = settings.keybindings[C_SHOOT_GREEN].isPressed();
	keyShoot2 = settings.keybindings[C_SHOOT_YELLOW].isPressed();
	keyShoot3 = settings.keybindings[C_SHOOT_RED].isPressed();
	keyShootPod = settings.keybindings[C_SHOOT_POD].isPressed();
    }
    else
    {
	keyRotLeft = ai->keys & ai->K_LEFT;
	keyRotRight = ai->keys & ai->K_RIGHT;
	keyDeAim = ai->keys & ai->K_DEAIM;
	keyDeZoom = ai->keys & ai->K_DEZOOM;
	keyShoot1 = ai->keys & ai->K_SHOOT1;
	keyShoot2 = ai->keys & ai->K_SHOOT2;
	keyShoot3 = ai->keys & ai->K_SHOOT3;
	keyShootPod = ai->keys & ai->K_POD;
    }

    if (keyRotLeft || keyRotRight)
    {
	if (keyRotLeft)
	    you.aim.angle += time*.015*settings.turnRateFactor*you.aimAccuracy();
	else
	    you.aim.angle += time*-.015*settings.turnRateFactor*you.aimAccuracy();
	you.aim.dist = std::max(AIM_MIN, (float)(you.aim.dist-time*.04));
    }
    if (keyDeAim)
	you.aim.dist = std::max(AIM_MIN, (float)(you.aim.dist-time*.1));
    else if (keyDeZoom)
    {
	you.aim.dist = std::max(AIM_MIN, (float)(you.aim.dist-time*.04));
	zoomdist = std::max(ZOOM_MIN, (float)(zoomdist-time*.2));
    }
    if (!(keyDeAim || keyDeZoom || keyRotLeft || keyRotRight))
    {
	// exponential decay from AIM_MIN to AIM_MAX:
	// dist = max - (max-min)*2^{-time*rate}
	// so d(dist)/d(time) is proportional to (max-dist).
	// In fact, we define:
	// max - dist = min(max, (max-dist_0) * exp(-time*aimRate))
	const float aimRate = youHaveNode(NODEC_PURPLE) ? 0.0006 : 0.0004;
	you.aim.dist = std::min(AIM_MAX,
		(float)(you.aim.dist + (AIM_MAX-you.aim.dist)*time*aimRate));
    }

    float mintheta = 4;
    for (std::vector<Node>::iterator it = nodes.begin();
	    it != nodes.end();
	    it++)
    {
	Angle relAngle = you.aim.angle - it->pos.angle;
	const float dtheta = std::min(float(relAngle), 4.0f-relAngle);
	if (it->status != NODEST_DESTROYED &&
		dtheta < mintheta)
	{
	    mintheta = dtheta;
	    targettedNode = &*it;
	}
    }
    if (mintheta > 0.5)
	targettedNode = NULL;

    you.shootTimer -= time;
    you.podTimer -= time;

    if ((keyShoot1 || keyShoot2 || keyShoot3)
	    && you.shootTimer <= 0)
    {
	int weight=0;
	bool super;
	if (keyShoot3 &&
		you.shootHeat < you.shootMaxHeat - shotHeat(2))
	{
	    weight = 3;
	    super = youHaveNode(NODEC_RED);
	}
	else if (keyShoot2 &&
		you.shootHeat < you.shootMaxHeat - shotHeat(1))
	{
	    weight = 2;
	    super = youHaveNode(NODEC_YELLOW);
	}
	else if (keyShoot1 &&
		you.shootHeat < you.shootMaxHeat - shotHeat(0))
	{
	    weight = 1;
	    super = youHaveNode(NODEC_GREEN);
	}
	if (weight > 0)
	{
	    float noise = gaussian()*you.aimAccuracy();

	    Shot shot( ARENA_CENTRE,
		    RelPolarCoord(you.aim.angle+noise,
			0.1+0.05*(3-weight) + super*0.05),
		    weight, super);
	    // we're generous, and assume that the command to shoot was given
	    // just after the last update, and the shot was fired as soon as
	    // the heat became low enough:
	    shot.update(std::min(time,
			std::min(
			    (you.shootMaxHeat - you.shootHeat -
			     shotHeat(weight-1))/you.shootCoolrate,
			    -you.shootTimer)));

	    shots.push_back(shot);

	    const int pitch = 900+weight*100 + int(40*gaussian());
	    soundEvents.newEvent(shot.pos-ARENA_CENTRE, shotChunk,
		    32, pitch);
	    if (super)
		soundEvents.newEvent(shot.pos-ARENA_CENTRE, shotChunk,
			32, pitch*3/4);

	    you.shootHeat += shotHeat(weight-1);
	    you.shootTimer = shotDelay(weight-1);

	    //you.aim.angle += weight*noise/4;

	    you.aim.dist = std::max(AIM_MIN,
		    (float)(you.aim.dist-weight*6));
	}
    }
    if (keyShootPod &&
	    targettedNode != NULL &&
	    you.podTimer <= 0 &&
	    you.shootHeat < you.shootMaxHeat - shotHeat(3))
    {
	const bool super = youHaveNode(NODEC_BLUE);
	invaders.push_back(new CapturePod(targettedNode,
		    RelPolarCoord(you.aim.angle, 5),
		    super));
	you.podTimer = shotDelay(3);
	you.shootHeat += shotHeat(3);
	you.doneLaunchedPod = true;

	const int pitch = 1500 + int(40*gaussian());
	soundEvents.newEvent(0, shotChunk,
		32, pitch);
	if (super)
	    soundEvents.newEvent(0, shotChunk,
		    32, pitch*3/4);
    }

    you.shootTimer = std::max(0, you.shootTimer);
    you.podTimer = std::max(0, you.podTimer);
}

void GameState::handleFreeViewInput(int time)
{
    // "free view mode": not useful or anything, but kind of fun

    static bool dirKeys[4];
    dirKeys[0] = settings.keybindings[C_DEZOOM].isPressed() ||
	settings.keybindings[C_M_UP].isPressed();
    dirKeys[1] = settings.keybindings[C_LEFT].isPressed() ||
	settings.keybindings[C_M_LEFT].isPressed();
    dirKeys[2] = settings.keybindings[C_DEAIM].isPressed() ||
	settings.keybindings[C_M_DOWN].isPressed();
    dirKeys[3] = settings.keybindings[C_RIGHT].isPressed() ||
	settings.keybindings[C_M_RIGHT].isPressed();
    bool keyZoomIn = settings.keybindings[C_SHOOT_GREEN].isPressed();
    bool keyZoomOut = settings.keybindings[C_SHOOT_YELLOW].isPressed();
    bool keyRotLeft = settings.keybindings[C_SHOOT_RED].isPressed();
    bool keyRotRight = settings.keybindings[C_SHOOT_POD].isPressed();

    const float baseZoom = (float)screenGeom.rad / (float)ARENA_RAD;
    const float maxZoom = baseZoom * 32;
    const float minZoom = baseZoom;
    bool moved = false;
    for (int i = 0; i < 4; i++)
	if (dirKeys[i])
	{
	    moved = true;
	    freeView.centre += RelPolarCoord(i-freeView.angle,
		    0.3*time*baseZoom/freeView.zoom);
	}

    // half a second of zooming doubles/halves the zoom
    if (keyZoomOut)
	freeView.zoom -= freeView.zoom * time * 0.0014;
    if (keyZoomIn)
	freeView.zoom += freeView.zoom * time * 0.0014;

    if (keyRotLeft)
	freeView.angle += 0.00075*time;
    if (keyRotRight)
	freeView.angle -= 0.00075*time;

    RelPolarCoord freeViewCentre = freeView.centre - ARENA_CENTRE;
    if (moved)
	freeView.zoom = std::max(freeView.zoom,
		(float)screenGeom.rad / ((float)ARENA_RAD - freeViewCentre.dist));
    freeView.zoom = std::min(maxZoom, std::max(minZoom, freeView.zoom));
    freeViewCentre.dist = std::min(freeViewCentre.dist,
	    (float)ARENA_RAD - ((float)screenGeom.rad / freeView.zoom));
    freeView.centre = ARENA_CENTRE + freeViewCentre;
}

void GameState::updateZoom(int time)
{
    float targetZoomdist = you.aim.dist;
    if (targetZoomdist < 0)
	targetZoomdist = 0;
    if (targetZoomdist > ZOOMDIST_MAX)
	targetZoomdist = ZOOMDIST_MAX;
    if (end)
	targetZoomdist = 0;
    if (time >= 200)
	zoomdist = targetZoomdist;
    else
	zoomdist += (targetZoomdist-zoomdist)*time/200;

    if (end == END_DEAD || end == END_EXTRACTED && zoomdist < 1)
    {
	freeViewMode = true;
	freeView = View(ARENA_CENTRE,
		(float)screenGeom.rad / (float)ARENA_RAD, -you.aim.angle);
    }
}

void GameState::evilAI(int time)
{
    invaderCooldown -= time;
    if (end != END_WIN &&
	    invaderCooldown <= 0 &&
	    rani(invaderRate/3) <= time)
    {
	// it's about time to spawn a new invader
	Invader* p_inv = NULL;
	int cost = 0;
	while (cost == 0)
	{
	    //int type = rani(5) == 0 ? 3 : rani(3);
	    int type = rani(4);
	    if (mutilationWave > -1)
		type = 3;
	    RelPolarCoord pos;
	    int ds;
	    switch (type)
	    {
		case 0: case 1: case 2:
		    {
			bool super = evilHasNode(
				type == 0 ? NODEC_RED :
				type == 1 ? NODEC_YELLOW :
				NODEC_GREEN);
			cost = 1;
			pos = RelPolarCoord(ranf()*4, ARENA_RAD-20);
			ds = super ? rani(9)-4 : rani(5)-2;
			switch (type)
			{
			    case 0: p_inv = new EggInvader(pos, ds, super);
				    break;
			    case 1: p_inv = new KamikazeInvader(pos, ds,
					    super); break;
			    case 2: p_inv = new SplittingInvader(pos, ds,
					    super); break;
			}
			break;
		    }
		case 3:
		    {
			// blue meanies cost 3 times as much to Evil as
			// normal invaders. In order to keep the average rate
			// of normal invaders constant, this cost is charged
			// even if, due to lack of targets, no meanie is
			// actually spawned.
			cost = 3;

			std::vector<Node*> possibleTargets;
			for (std::vector<Node>::iterator it = nodes.begin();
				it != nodes.end();
				it++)
			{
			    if ((it->status == NODEST_NONE ||
					it->status == NODEST_YOU) &&
				    it->targettingInfester == NULL)
				possibleTargets.push_back(&*it);
			}
			if (possibleTargets.size() > 0)
			{
			    const int i = rani(possibleTargets.size());
			    p_inv = new InfestingInvader(possibleTargets[i],
				    evilHasNode(NODEC_BLUE));
			}
		    }
		    break;
		case 4:
		    // FoulEggLayingInvader - unused
		    cost = 1;
		    pos = RelPolarCoord(ranf()*4, ARENA_RAD-(20+rani(5)*10));
		    ds = rani(5)-2;
		    p_inv = new FoulEggLayingInvader(pos, ds);
		    break;
	    }
	}
	if (p_inv)
	    invaders.push_back(p_inv);
	invaderCooldown += invaderRate * cost;
    }

    if (evilHasNode(NODEC_PURPLE) &&
	    rani(7500) < time)
    {
	for (std::vector<Invader*>::iterator it = invaders.begin();
		it != invaders.end();
		it++)
	    (*it)->dodge();
    }
}

void GameState::cleanup()
{
    if (deadShots)
    {
	shots.erase(remove_if(shots.begin(),
		    shots.end(), Shot::is_dead),
		shots.end());
    }

    for (std::vector<Invader*>::iterator it = invaders.begin();
	    it != invaders.end();
	    it++)
    {
	if ((*it)->dead())
	{
	    (*it)->onDeath();
	    delete *it;
	    (*it) = NULL;
	}
    }
    invaders.erase(remove_if(invaders.begin(),
		invaders.end(), isNullInvp),
	    invaders.end());
}

void GameState::draw(SDL_Surface* surface)
{
    View view;
    View boundView;

    if (!freeViewMode)
    {
	const RelPolarCoord d(you.aim.angle, zoomdist);

	const View zoomView(ARENA_CENTRE + d,
		(float)screenGeom.rad/((float)ARENA_RAD-zoomdist),
		settings.rotatingView ? -d.angle : 0);

	const View outerView(ARENA_CENTRE,
		(float)screenGeom.rad/(float)ARENA_RAD,
		settings.rotatingView ? -d.angle : 0);

	view = settings.zoomEnabled ? zoomView : outerView;
	boundView = zoomView;

	if (settings.zoomEnabled)
	{
	    ((settings.useAA == AA_FORCE) ? aacircleColor : circleColor)
		(surface, screenGeom.centre.x, screenGeom.centre.y,
		 screenGeom.rad, 0x505050ff);
	}
	else
	{
	    Circle(zoomView.centre, ARENA_RAD-zoomdist,
		    0x505050ff).draw(surface, view, NULL, true);
	}
    }
    else
    {
	boundView = view = freeView;
	boundView.zoom /= 3;

	((settings.useAA == AA_FORCE) ? aacircleColor : circleColor)
	    (surface, screenGeom.centre.x, screenGeom.centre.y,
	     screenGeom.rad, 0x505050ff);
    }

    drawIndicators(surface, view);
    drawGrid(surface, view);
    drawTargettingLines(surface, view);
    drawObjects(surface, view, &boundView);
    drawNodeTargetting(surface, view);
}

void GameState::drawGrid(SDL_Surface* surface, const View& view)
{
    Circle(ARENA_CENTRE, ARENA_RAD,
	    0x808080ff).draw(surface, view, NULL, true);

    if (settings.showGrid)
    {
	for (int i=1; i<6; i++)
	    Circle(ARENA_CENTRE, i*ARENA_RAD/6,
		    0x30303000 + (i%2==0)*0x08080800 + 0xff
		  ).draw(surface, view, NULL, true);
	for (int i=0; i<12; i++)
	    Line(ARENA_CENTRE, ARENA_CENTRE +
		    RelPolarCoord(i*4.0/12, ARENA_RAD),
		    0x30303000 + (i%2==0)*0x08080800 + 0xff
		).draw(surface, view, NULL, true);
    }
}

void GameState::drawTargettingLines(SDL_Surface* surface, const View& view)
{
    if (!you.dead)
    {
	const Uint32 aimColour = 
	    (ai) ? (youHaveNode(NODEC_PURPLE) ? 0x00010100 : 0x00010000) :
	    youHaveNode(NODEC_PURPLE) ? 0x01000100 : 0x01000000;

	for (int dir = -1; dir < 3; dir+=2)
	    Line(ARENA_CENTRE, ARENA_CENTRE +
		    RelPolarCoord(you.aim.angle + dir*you.aimAccuracy(),
			ARENA_RAD),
		    aimColour * 0x80 + 0xff).draw(surface, view, NULL, true);

	if (fabsf(you.aimAccuracy()) <= .45)
	    for (int dir = -1; dir < 3; dir+=2)
		Line(ARENA_CENTRE, ARENA_CENTRE +
			RelPolarCoord(you.aim.angle + dir*2*you.aimAccuracy(),
			    ARENA_RAD),
			aimColour * 0x50 + 0xff).draw(surface, view, NULL, true);
    }
}

void GameState::drawNodeTargetting(SDL_Surface* surface, const View& view)
{
    if (!you.dead && targettedNode != NULL)
    {
	const Uint32 c = ( (you.shootHeat < you.shootMaxHeat - shotHeat(3)) ?
		0xff000000 : 0xd0600000 ) + ( (you.podTimer <= 0) ?
		    0xff : 0x60);
	const CartCoord p = targettedNode->cpos();
	const float r = targettedNode->radius;
	Line(p + RelCartCoord(-9*r/5, 0), p + RelCartCoord(-7*r/5, 0),
		c).draw(surface, view, NULL);
	Line(p + RelCartCoord(9*r/5, 0), p + RelCartCoord(7*r/5, 0),
		c).draw(surface, view, NULL);
	Line(p + RelCartCoord(0, -9*r/5), p + RelCartCoord(0, -7*r/5),
		c).draw(surface, view, NULL);
	Line(p + RelCartCoord(0, 9*r/5), p + RelCartCoord(0, 7*r/5),
		c).draw(surface, view, NULL);
    }
}

void GameState::drawObjects(SDL_Surface* surface, const View& view,
	View* boundView)
{
    you.draw(surface, view, NULL);

    for (std::vector<Shot>::iterator it = shots.begin();
	    it != shots.end();
	    it++)
	it->draw(surface, view, boundView);

    for (std::vector<Invader*>::iterator it = invaders.begin();
	    it != invaders.end();
	    it++)
	(*it)->draw(surface, view, boundView);

    for (std::vector<Node>::iterator it = nodes.begin();
	    it != nodes.end();
	    it++)
	it->draw(surface, view, boundView);

    if (mutilationWave > 0)
	Circle(ARENA_CENTRE, mutilationWave, 0x00ffffff).draw(surface, view, NULL);
    else if (extracted > extractPreMutCutoff && extracted < extractMax)
	Circle(ARENA_CENTRE, ARENA_RAD,
		0x00ffff00 + (0x4f + 0x80*(
			((int)extracted - extractPreMutCutoff) /
			(extractMax - extractPreMutCutoff)) +
		    (int)(0x30 * preMutilationPhase.sinf()))
		).draw(surface, view, NULL);
}

// approxAtan2Frac: approximates atan2(y,x)*(6/PI)-1
//  (being the linear function of atan2(y,x) which is 0 at PI/6 and 1 at PI/3)
float approxAtan2Frac(int y, int x)
{
    static const int N = 10;
    static const float left=0.5, right=2.0;
    static float z0[N];
    static float t0[N], t1[N];
    static const float halfDist = (right-left)/(2*(N-1));
    static bool preCalced=false;

    if (!preCalced)
    {
	// calculate first two terms of the Taylor expansion around some
	// values of z spaced uniformly along the interval
	for (int i = 0; i < N; i++)
	{
	    z0[i] = left + (right-left)*i/(N-1);

	    t0[i] = atan(z0[i])*(6.0/PI)-1;
	    t1[i] = (1.0/(1+z0[i]*z0[i]))*(6.0/PI);
	}
	preCalced = true;
    }

    float z = float(y)/x;

    // use the precalculated linear approximation around the closest z value
    for (int i = 0; i < N; i++)
	if ( i == N-1 || z < z0[i] + halfDist )
	    return t0[i] + t1[i]*(z-z0[i]);

    return 0; // won't happen

    // cubic approximation to atan(z) around z=1:
    // return PI/4 + (z-1)/2 - (z-1)*(z-1)/4 + (z-1)*(z-1)*(z-1)/12;
}

void GameState::drawIndicators(SDL_Surface* surface, const View& view)
{
    // heat, shield and extraction indicators:
    Uint32 colour;
    for (int x = screenGeom.rad/2;
	    x <= 866*screenGeom.rad/1000 + 15;
	    x++)
    {
	const int xsq = x*x;
	int rsq;
	for (int y = int(sqrt(screenGeom.indicatorRsqLim1 - xsq));
		(rsq = xsq + y*y) <= screenGeom.indicatorRsqLim4;
		y++)
	{
	    if (rsq < screenGeom.indicatorRsqLim1)
		continue;
	    const float frac = approxAtan2Frac(y,x);
	    if (frac >= 0 && frac <= 1)
	    {
		if (rsq <= screenGeom.indicatorRsqLim2)
		{
		    // decay towards the edges, for prettiness
		    const int decay = std::min(255,
			    std::min(rsq - screenGeom.indicatorRsqLim1,
				screenGeom.indicatorRsqLim2 - rsq)/2);
		    int intensity;

		    // heat
		    intensity = 
			you.shootHeat > you.shootMaxHeat*frac ? 55+int(200*frac) :
			(frac >= 0.98 ? int(5000*(frac-0.98)) : 0) + (
				35 );
		    colour = 0x01000000 * intensity + decay;

		    pixelColor(surface, screenGeom.centre.x - x,
			    screenGeom.centre.y - y, colour);

		    // extraction
		    if (extracted <= extractPreMutCutoff)
			intensity =
			    extracted > extractPreMutCutoff*frac ? 55+int(200*frac) :
			    (frac >= 0.98 ? int(5000*(frac-0.98)) : 0) + (
				    evilHasNode(NODEC_CYAN) ? 55 : 35 );
		    else
			intensity =
			    ((extracted - extractPreMutCutoff) >
			     (extractMax - extractPreMutCutoff)*frac) ?
			    85+int(170*frac) :
			    (frac >= 0.98 ? int(5000*(frac-0.98)) : 0) + (
				    evilHasNode(NODEC_CYAN) ? 85 : 60 );
		    colour = 0x00010100 * intensity + decay;

		    pixelColor(surface, screenGeom.centre.x + x,
			    screenGeom.centre.y - y, colour);
		}
		else if (rsq < screenGeom.indicatorRsqLim3)
		{
		    // shade between heat and shield indicators
		    pixelColor(surface, screenGeom.centre.x - x,
			    screenGeom.centre.y - y, 0xa0);
		}
		else
		{
		    const int decay = std::min(255,
			    std::min(rsq - screenGeom.indicatorRsqLim3,
				screenGeom.indicatorRsqLim4 - rsq)/2);
		    int intensity;

		    // shield
		    const int i = int(frac*4);
		    const Uint32 baseColour =
			(i == 0) ? 0x01000000 :
			(i == 1) ? 0x01010000 :
			(i == 2) ? 0x00010000 :
			0x00010100;

		    intensity =
			(you.shield > frac*4) ? 55+(int(4*200*frac))%200 :
			youHaveNode(NODEC_CYAN) ? 55 :
			35;

		    colour =
			baseColour * intensity + decay;

		    pixelColor(surface, screenGeom.centre.x - x,
			    screenGeom.centre.y - y, colour);
		}
	    }
	}
    }

    // shot indicators
    static const double shotIndicatorCos = cos(PI/6-PI/120);
    static const double shotIndicatorSin = sin(PI/6-PI/120);
    for (int i = 0; i < 4; i++)
    {
	const float d = screenGeom.rad + 3 + 5*i;
	const float x = shotIndicatorCos * d;
	const float y = shotIndicatorSin * d;
	const View shotView(CartCoord(x,-y), 1, 0);
	const bool super = youHaveShotNode(i);


	if (you.shootHeat >= you.shootMaxHeat - shotHeat(i))
	    continue;

	if (i == 3)
	{
	    // capture pod
	    if (you.podTimer <= 0)
		CapturePod(NULL, RelPolarCoord(0,0), super).draw(surface, shotView, NULL);
	}
	else
	{
	    const int weight = i+1;
		Shot( CartCoord(0,0),
			RelPolarCoord(-0.3,
			    0.1+0.05*(3-weight) + super*0.04),
			weight, super).draw(surface, shotView, NULL);
	}
    }

    // node possession indicators
    static const double nodeIndicatorCos[6] = {
	cos(PI/6), cos(PI/6 + PI/36), cos(PI/6 + 2*PI/36),
	cos(PI/6 + 3*PI/36), cos(PI/6 + 4*PI/36), cos(PI/6 + 5*PI/36) };
    static const double nodeIndicatorSin[6] = {
	sin(PI/6), sin(PI/6 + PI/36), sin(PI/6 + 2*PI/36),
	sin(PI/6 + 3*PI/36), sin(PI/6 + 4*PI/36), sin(PI/6 + 5*PI/36) };
    const float dist = screenGeom.rad + 3 + 20 + 10;

    int youNodeCount = 0;
    int evilNodeCount = 0;
    for (std::vector<Node>::iterator it = nodes.begin();
	    it != nodes.end();
	    it++)
    {
	if (it->status == NODEST_YOU)
	{
	    const View nodeView(CartCoord(
			nodeIndicatorCos[youNodeCount] * dist,
			- nodeIndicatorSin[youNodeCount] * dist),
		    1, 0);
	    Node(RelPolarCoord(0,0), 0, it->nodeColour).draw(surface, nodeView, NULL);
	    youNodeCount++;
	}
	else if (it->status == NODEST_EVIL)
	{
	    const View nodeView(CartCoord(
			- nodeIndicatorCos[evilNodeCount] * dist,
			- nodeIndicatorSin[evilNodeCount] * dist),
		    1, 0);
	    Node(RelPolarCoord(0,0), 0, it->nodeColour).draw(surface, nodeView, NULL);
	    evilNodeCount++;
	}
    }
}

int GameState::rateOfRating(int rating)
{
    static int rates[] = {
	4000, 3200, 2500, 2250, 2000, 1750, 1500, 1300, 1150};
    if (rating < 1)
	return -1;
    if (rating > 9)
	// Elite+, Elite++, ...
	return (int)(1000.0*pow(0.9, (double)(rating-9)));
    return rates[rating-1];
}

void GameState::setRating()
{
    double newRating = settings.requestedRating > 0 ?
	settings.requestedRating : config.rating[speed];
    if (newRating < 1.0)
	newRating = 1.0;

    rating = newRating;

    double effRating = std::max(1.0, rating - 0.5*speed);

    int i = (int) effRating;
    int r1 = rateOfRating(i);
    int r2 = rateOfRating(i+1);
    
    invaderRate = (r1 + (int)((effRating-i)*(r2-r1)));
}

const char* ratingString(int rating)
{
    // Recognise these?
    switch (rating)
    {
	case 1: return "Harmless";
	case 2: return "Mostly Harmless";
	case 3: return "Poor";
	case 4: return "Average";
	case 5: return "Above Average";
	case 6: return "Competent";
	case 7: return "Dangerous";
	case 8: return "Deadly";
	case 9: return "Elite";
	case 10: return "Elite+";
	case 11: return "Elite++";
	case 12: return "Elite+++";
	default: return "Unrated";
    };
}

const char* GameState::getHint()
{
    if (rating > 5.0 || settings.invuln)
	return "";

    if (end != END_WIN)
    {
	// Let's see if we can work out what they need to know...
	if (you.score == 0)
	    return "Hint: use keys '1', '2', '3' to shoot.";

	if (you.score >= 10)
	{
	    if (!you.doneLaunchedPod)
		return "Hint: use key '4' to capture Nodes.";

	    bool destroyed = false;
	    for (std::vector<Node>::iterator it = nodes.begin();
		    it != nodes.end();
		    it++)
		if (it->status == NODEST_DESTROYED)
		{
		    destroyed = true;
		    break;
		}

	    if (!destroyed && (END_EXTRACTED || you.score >= 30))
		return "Hint: hit filled nodes with '3'; destroy 4 to win!";
	}
    }

    // Either they won, or they lost for no obvious reason. Maybe we should
    // tell them about some things they might not have thought of...
    switch (int(rating*10)%6)
    {
	case 0:
	    return "Hint: Nodes help whoever has them.";
	case 1:
	    return "Hint: Your shield charges if you stop shooting.";
	case 2:
	    return "Hint: Use Down to turn fast, Up to glance back.";
	case 3:
	    return "Hint: Pods ('4') can kill even shielded blue Blobs.";
	case 4:
	    return "Hint: Green shots won't hurt blue Blobs.";
	case 5:
	    return "Hint: Don't let Evil fill the cyan bar.";

	default:
	    return "Hint: THIS IS A BUG.";
    }
}
