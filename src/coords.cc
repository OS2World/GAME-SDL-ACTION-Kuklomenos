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

#include <cmath>

#include "coords.h"

RelPolarCoord::RelPolarCoord(const RelCartCoord& c)
{
    dist = sqrt(c.dx*c.dx + c.dy*c.dy);
    angle = atan2(-c.dx, c.dy)/(PI/2);
}

RelCartCoord::RelCartCoord(const RelPolarCoord &rp)
{
    dx = -rp.dist*sin(rp.angle * (PI/2));
    dy = rp.dist*cos(rp.angle * (PI/2));
}

RelCartCoord RelCartCoord::rotated(float angle) const
{
    if (angle == 0)
	return *this;

    RelPolarCoord p(*this);
    p.angle += angle;
    return p;
}

float dist(RelPolarCoord c)
{ return c.dist; }
Angle angle(RelPolarCoord c)
{ return c.angle; }

// angleDiff: returns signed difference between the angles, i.e. angle of
// shortest rotation to turn from 'a1' to 'a2'. Return value is in (-2,2].
float angleDiff(Angle a1, Angle a2)
{
    Angle copy = a2;
    a2 -= a1;
    float a = a2;
    
    if (a > 2)
	return a-4;
    else
	return a;
}
