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

#include "ai.h"
#include "random.h"
#include "state.h"
#include "invaders.h"
#include "coords.h"
#include "node.h"
#include "gfx.h"
#include "settings.h"

/* AI
 *
 * Architecture:
 *
 * 'update' gets called every time the state gets updated; it should set
 * 'keys', which will then affect state update just as if the player were
 * pressing them.
 *
 * The AI can peek directly at the state. Because keeping its own copy
 * of everything would be a nightmare, it can also record whatever
 * record-keeping information it likes in the 'aidata' attribute of state
 * objects, which is of type 'struct AIData'.
 *
 * XXX: necessarily, the AI code depends heavily on the specifics of the game
 * rules, with many formulae copied from or based on those in state.cc. Be
 * sure to keep it in sync.
 */

// AI::closestEnemy: return closest shootable enemy
HPInvader* AI::closestEnemy()
{
    HPInvader* closest = NULL;
    float closestDist = -1;

    for (std::vector<Invader*>::iterator it = gameState->invaders.begin();
	    it != gameState->invaders.end();
	    it++)
    {
	HPInvader* inv = dynamic_cast<HPInvader*>(*it);
	if (inv == NULL)
	    continue;

	float d = dist(inv->cpos() - ARENA_CENTRE);
	if ( inv->aiData.seen && inv->evil() && 
		inv->hitsShots() && inv->armour < 3 &&
		(closest == NULL || d < closestDist) )
	{
	    closestDist = d;
	    closest = inv;
	}
    }

    return closest;
}

void AI::updateSeen()
{
    const RelPolarCoord d(gameState->you.aim.angle, gameState->zoomdist);

    const View zoomView(ARENA_CENTRE + d,
	    (float)screenGeom.rad/((float)ARENA_RAD-gameState->zoomdist),
	    settings.rotatingView ? -d.angle : 0);

    for (std::vector<Invader*>::iterator it = gameState->invaders.begin();
	    it != gameState->invaders.end();
	    it++)
	if (zoomView.inView((*it)->cpos()))
	    (*it)->aiData.seen=true;
}

// AI::predictPos: return predicted position of 'inv' in 'time' ms
RelPolarCoord AI::predictPos(Invader* inv, float time)
{
    SpirallingInvader* spiralInv = dynamic_cast<SpirallingInvader*>(inv);
    if (spiralInv)
    {
	RelPolarCoord pos = spiralInv->pos;

	InfestingInvader* infInv = dynamic_cast<InfestingInvader*>(spiralInv);
	if (!infInv)
	{
	    // use same calculation as in SpirallingInvader::doUpdate(int time):
	    const int steps = std::min(10, 1 + (int)(time / 30));
	    for (int i = 0; i < steps; i++)
	    {
		pos.angle += spiralInv->ds*0.0001*(100/spiralInv->pos.dist)*(time/steps);
		pos.dist += spiralInv->dd*-0.0075*(time/steps);
	    }
	}
	else
	{
	    // InfestingInvaders are a special case - they align with
	    // their target node
	    Node* node = infInv->targetNode;
	    pos.angle = node->pos.angle +
		node->ds*0.0001*(100/node->pos.dist)*time;
	    pos.dist += spiralInv->dd*-0.0075*time;
	}
	return pos;
    }
    else
	// linear approximation
	return (inv->cpos()
		    + (inv->collObj().velocity * time)
		    - ARENA_CENTRE
		 );
}

/* BasicAI:
 * Simple, mostly stateless, reactionary AI.
 *
 * 'update' routine:
 * Consider closest enemy. Estimate, in a simplistic inaccurate way, where we
 * want to be rotated to so as to give us time to aim sufficiently and fire.
 * If close to it, wait/fire as appropriate. Else, rotate.
 *
 * Occasionally, fire pod instead. Also occasionally, check for primed nodes
 * and try to shoot them.
 */

void BasicAI::update(int time)
{
    keys=0;

    updateSeen();

    const RelPolarCoord aim = gameState->you.aim;
    const float aimAccuracy = gameState->you.aimAccuracy();
    const float aimRate = gameState->youHaveNode(NODEC_PURPLE) ? 0.0006 : 0.0004;

    HPInvader* inv = closestEnemy();

    if ( !inv || dist(inv->cpos() - ARENA_CENTRE) > ARENA_RAD/3 )
    {
	// no immediate urgent threat - consider strategic aims

	if (seed % 15 == 0 && gameState->you.podTimer <= 0)
	{
	    // consider shooting a pod
	    if ( gameState->targettedNode != NULL &&
		    gameState->targettedNode->status != NODEST_YOU &&
		    (gameState->you.shootHeat <
		     gameState->you.shootMaxHeat - gameState->shotHeat(3)) )
	    {
		keys |= K_POD;
		newSeed();
	    }
	    else if (seed % 2)
		keys |= K_LEFT;
	    else
		keys |= K_RIGHT;
	    return;
	}

	if (seed % 4 == 0)
	{
	    // consider shooting a primed node
	    Node* node = NULL;
	    Angle minangle;
	    for (std::vector<Node>::iterator it = gameState->nodes.begin();
		    it != gameState->nodes.end();
		    it++)
	    {
		Angle dangle = it->pos.angle - gameState->you.aim.angle;
		if (dangle > 2)
		    // normalise to [0,2]
		    dangle = -dangle;
		if (it->status == NODEST_YOU && it->primed >= 1
			&& ( node == NULL || dangle < minangle ))
		{
		    node = &*it;
		    minangle = dangle;
		}
	    }
	    if (node)
		inv=node;
	}
    }
    
    if (inv)
    {
	// some tweakable parameters to affect behaviour:
	// aimPerDist: 'aim.dist' to wait for is aimPerDist * (dist to target)
	// aimRange: don't turn while this close (in sds) to target angle
	// deAimRange: deaim to turn fast until this close to target angle
	// estimateIterations: times through estimation loop
	static const float aimPerDist = 0.3f;
	static const float aimRange = 0.4f;
	static const float deAimRange = 2.0f;
	static const int estimateIterations = 3;

	const int shotWeight = getShotWeight(inv->hp, inv->armour);
	const bool super = gameState->youHaveNode(NodeColour(NODEC_GREEN + (shotWeight-1)));
	const float shotSpeed = 0.1+0.05*(3-shotWeight) + super*0.05;

	const RelPolarCoord invPos = inv->cpos() - ARENA_CENTRE;

	// iteratively predict where we should want to turn to to give us
	// sufficient time to aim and for the shot to get to the target.
	// 'ppos' is current estimate of where the nasty will be when the shot
	// hits it (so 'ppos.angle' is the angle to turn to before shooting);
	// we start with 'ppos' being current position of target, and
	// iteratively refine the estimate. Note that it will not tend to the
	// correct answer, as we are using some linear estimates, e.g. in
	// turnTime.
	float targAimDist;
	RelPolarCoord ppos = invPos;
	for (int i=0; i<estimateIterations; i++)
	{
	    targAimDist = std::min(AIM_MAX*2/3, aimPerDist*ppos.dist);
	    const float turnTime = fabs(angleDiff(aim.angle, ppos.angle)) /
		(0.015*settings.turnRateFactor*aimAccuracy);
	    const float aimAtTarget = std::max(AIM_MIN, (float)(aim.dist-turnTime*.04));

	    // aiming time required: almost exact, the +1 is just to avoid
	    // division by 0.
	    const float aimTime =
		-logf((AIM_MAX - targAimDist) / (AIM_MAX - aimAtTarget + 1)) / aimRate;

	    const float shootTime = ppos.dist / shotSpeed;

	    ppos = predictPos(inv, shootTime + aimTime);
	}
	const float relAimDist = std::max(0.0f, targAimDist - aim.dist);
	const float relAngle = angleDiff(aim.angle, ppos.angle);
	const float modRelAngle = fabs(angleDiff(aim.angle, ppos.angle));

	// sameDir: our target angle is off from current angle in the same dir
	// as our target is turning about us
	const float invRelAngle = angleDiff(invPos.angle, ppos.angle);
	const bool sameDir =
	    ((relAngle > 0) && (invRelAngle > 0)) || 
	    ((relAngle < 0) && (invRelAngle < 0));


	if (modRelAngle <= aimRange*aimAccuracy &&
		relAimDist == 0 &&
		(gameState->you.shootHeat <
		 gameState->you.shootMaxHeat - gameState->shotHeat(shotWeight-1)
		))
	{
	    keys |= K_SHOOT1 << (shotWeight-1);
	    newSeed();
	}
	else if (sameDir || modRelAngle > aimRange*aimAccuracy)
	{
	    if (relAngle >= 0)
		keys |= K_LEFT;
	    else
		keys |= K_RIGHT;

	    if (modRelAngle > deAimRange*aimAccuracy)
		keys |= K_DEAIM;
	}
    }
    else
    {
	// haven't seen any valid enemies - spin around until we find one
	keys |= K_DEAIM;
	if (seed % 2)
	    keys |= K_LEFT;
	else
	    keys |= K_RIGHT;
	if (! rani(300))
	    newSeed();
    }
}

// getShotWeight: returns, based on seed, a random number in [1,3] which
// is greater than armour and no more than hp+armour.
int BasicAI::getShotWeight(int hp, int armour) const
{
    if (armour > 2)
	// sanity check
	return 1; 

    return armour + 1 + seed%std::min((3 - armour), hp);
}

// the seed determines current behaviour; we change it every time the current
// behaviour has been completed (e.g. we fire off a shot)
void BasicAI::newSeed()
{
    seed = rani(32767);
}

BasicAI::BasicAI(GameState* gameState) : AI(gameState)
{
    newSeed();
}
