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

#include <algorithm>

#include "player.h"
#include "gfx.h"
#include "geom.h"

Player::Player() :
    shootCoolrate(10), shootHeat(0), shootMaxHeat(32000), shield(0),
    aim(0,50), shootTimer(0), podTimer(0),
    score(0), doneLaunchedPod(false), dead(false)
{}

/* returns angle used for the standard deviation of the gaussian noise applied
 * to shots:
 */
float Player::aimAccuracy()
{
    return 0.2*(AIM_MIN*AIM_MIN)/(aim.dist * aim.dist);
}

void Player::draw(SDL_Surface* surface, const View& view, View* boundView,
	bool noAA)
{
    for (int i = 0; i < 4; i++)
	if (shield > i)
	{
	    const float r = 4+3*i;
	    const Uint32 baseColour =
		(i == 0) ? 0x01000000 :
		(i == 1) ? 0x01010000 :
		(i == 2) ? 0x00010000 :
		0x00010100;
	    const Uint32 colour =
		baseColour*(55 + std::min(200, (int)(200*shield)-200*i))
		+ 0x000000ff;
	    Circle(ARENA_CENTRE, r, colour).draw(surface, view, boundView,
		    noAA);
	}
}

void Player::update(int time, bool superShield)
{
    if (!dead && shootHeat == 0)
    {
	const double shieldRateMult = superShield ? 2.5 : 1.0;
	if (shield < 1)
	    shield += time*0.0001*shieldRateMult;
	else if (shield < 2)
	    shield += time*0.00005*shieldRateMult;
	else if (shield < 3)
	    shield += time*0.000025*shieldRateMult;
	else if (shield < 4)
	    shield = std::min(4.0, shield + time*0.0000125*shieldRateMult);
    }

    shootHeat = std::max(0, shootHeat - shootCoolrate*time);
}

float Player::radius()
{
    return 4 + 3*((int)floor(shield)+1);
}
