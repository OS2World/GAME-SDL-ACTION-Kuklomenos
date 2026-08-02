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

#include "gfx.h"
#include "shot.h"

void Shot::update(int time)
{
    pos += vel*time;
    timeLived += time;
}

void Shot::draw(SDL_Surface* surface, const View& view, View* boundView,
	bool noAA)
{
    Uint32 basecolour;

    switch (weight)
    {
	case 3: basecolour = 0xff000000;
		break;
	case 2: basecolour = 0xffff0000;
		break;
	default: basecolour = 0x00ff0000;
    }

    Line(pos, pos + vel * -std::min(timeLived, 50),
	    basecolour + (super ? 0xb0 : 0x70)
	).draw(surface, view, boundView, noAA);

    Pixel(pos, basecolour + (super ? 0xff : 0xe0)
	 ).draw(surface, view, boundView, noAA);
}

int Shot::hit(int damage)
{
    int lost = std::min(weight, damage);
    weight -= lost;
    if (weight == 0)
	dead = 1;
    return lost;
}

int Shot::die()
{
    return hit(weight);
}
