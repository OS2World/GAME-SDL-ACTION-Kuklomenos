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

#include "collision.h"
#include "coords.h"

float pointHitsCircle(float x, float y, float vx, float vy, float rad, float et)
{
    if (et >= 0)
    {
	if ((x < -rad && x+et*vx < -rad) || (x > rad && x+et*vx > rad) ||
		(y < -rad && y+et*vy < -rad) || (y > rad && y+et*vy > rad))
	    return -1;
    }
    else
	if ((x < -rad && vx < 0) || (x > rad && vx > 0) ||
		(y < -rad && vy < 0) || (y > rad && vy > 0))
	    return -1;

    if (x*x + y*y <= rad*rad)
	return 0;

    float a = vx*vx+vy*vy;
    float b = vx*x+vy*y;
    float c = x*x+y*y-rad*rad;

    /* answer is the lesser real solution to at^2+2bt+c=0, if it has any; we
     * just use the quadratic formula! */
    float discriminantOverFour = b*b-a*c;
    if (discriminantOverFour < 0)
	return -1;
    float t = (-b - sqrt(discriminantOverFour))/a;
    if (t < 0 || (et >= 0 && t > et))
	return -1;
    return t;
}

float pointHitsPolygon(RelCartCoord* points, int n, RelCartCoord p,
	RelCartCoord v, float et )
{
    double maxIn=0;
    double minOut=-1;
    for (int i=0; i < n; i++)
    {
	RelCartCoord next;
	if (i+1 < n)
	    next = points[i+1];
	else
	    next = points[0];
	RelCartCoord dir = next - points[i];
	RelCartCoord r = p - points[i];
	// ddy*(rdx+t.vx) - ddx(rdy+t.vy) = 0
	// t(ddy.vx - ddx.vy) = ddx.rdy-ddy.rdx
	double vel = dir.dy*v.dx - dir.dx*v.dy;
	double d = dir.dy*r.dx - dir.dx*r.dy;
	if (vel == 0)
	{
	    if (d > 0)
		return -1;
	    else
		continue;
	}

	double t = d/-vel;
	if (d > 0)
	{
	    // enter halfplane at time t
	    if (t < 0 || (et >= 0 && t > et))
		return -1;
	    if (t > maxIn)
		maxIn = t;
	}
	else
	{
	    // leave halfplane at time t
	    if (t < 0 || (et >= 0 && t > et))
		continue;
	    if (minOut == -1 || t < minOut)
		minOut = t;
	}
    }
    if (minOut == -1 || maxIn < minOut)
	return float(maxIn);
    else
	return -1;
}

bool CollisionObject::pointIn(float x, float y) const
{
    return (pointHits(x, y, 0, 0, 0) == 0);
}
float CollisionObject::pointHits(float x, float y, float vx, float vy, float et) const
{
    return pointHits(CartCoord(x,y), RelCartCoord(vx,vy), et);
}
bool CollisionObject::pointIn(CartCoord p) const
{
    return pointIn(p.x, p.y);
}

bool CollisionObject::objectCollides(const CollisionCircle& other) const
{
    return circleIntersects(other.startPos, other.radius);
}
bool CollisionObject::objectCollides(const CollisionPolygon& other) const
{
    // UNIMPLEMENTED
    return false;
}

float CollisionCircle::pointHits(CartCoord p, RelCartCoord v, float et) const
{
    return pointHitsCircle(p.x - startPos.x, p.y - startPos.y,
	    v.dx - velocity.dx, v.dy - velocity.dy, radius, et);
}

bool CollisionCircle::circleIntersects(CartCoord c, float rad) const
{
    return (pointHitsCircle(startPos.x - c.x, startPos.y - c.y, 0, 0,
		radius + rad, 0) == 0);
}

float CollisionPolygon::pointHits(CartCoord p, RelCartCoord v, float et) const
{
    RelCartCoord rp = (p - startPos).rotated(-angle);
    RelCartCoord rv = (v - velocity).rotated(-angle);
    return pointHitsPolygon(points, numPoints, rp, rv, et);
}

bool CollisionPolygon::circleIntersects(CartCoord c, float rad) const
{
    // TODO properly
    return pointIn(c);
}
