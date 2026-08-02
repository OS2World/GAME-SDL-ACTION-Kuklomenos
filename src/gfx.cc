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
#include <SDL/SDL.h>
#include <SDL_gfxPrimitivesDirty.h>

#include "coords.h"
#include "gfx.h"
#include "geom.h"
#include "settings.h"

View::View(CartCoord icentre, float izoom, float iangle) :
    centre(icentre), zoom(izoom), angle(iangle) {}

ScreenCoord View::coord(const CartCoord &c) const
{
    RelCartCoord d( (c-centre).rotated(angle) );
    return ScreenCoord(
	    screenGeom.centre.x+(int)(d.dx*zoom),
	    screenGeom.centre.y-(int)(d.dy*zoom));
}

bool View::inView(const CartCoord &c, float in) const
{
    RelCartCoord d = c-centre;
    return ( (zoom*zoom)*(d.dx*d.dx+d.dy*d.dy) <= (screenGeom.rad-in)*(screenGeom.rad-in) );
}

int Line::draw(SDL_Surface* surface, const View& view, View* boundView, bool noAA)
{
    ScreenCoord s = view.coord(start);
    ScreenCoord e = view.coord(end);
    const bool useAA = ( (settings.useAA==AA_YES && !noAA) ||
	    settings.useAA==AA_FORCE );

    Uint32 drawColour = colour;

    if (!boundView || (boundView->inView(start) && boundView->inView(end)))
	return ( useAA ? aalineColor : lineColor )
	    (surface, s.x, s.y, e.x, e.y, drawColour);
    else
	return 0;
}

int Circle::draw(SDL_Surface* surface, const View& view, View* boundView, bool noAA)
{
    bool useAA = ( (settings.useAA==AA_YES && !noAA) ||
	    settings.useAA==AA_FORCE );
    ScreenCoord c = view.coord(centre);
    float screenRad = r*view.zoom;

    if (screenRad > 2*screenGeom.rad)
	// our AA circle algorithm doesn't handle huge circles well
	useAA = false;

    if (!boundView || boundView->inView(centre, r*boundView->zoom))
	return (filled ? filledCircleColor :
		useAA ? aacircleColor : circleColor)
	    (surface, c.x, c.y, int(screenRad), colour);
    else
	return 0;
}

int Polygon::draw(SDL_Surface* surface, const View& view, View* boundView, bool noAA)
{
    const bool useAA = ( (settings.useAA==AA_YES && !noAA) ||
	    settings.useAA==AA_FORCE );

    for (int i=0; i<n; i++)
    {
	if (boundView && !boundView->inView(points[i]))
	    return 0;
    }

    Sint16 *sx = new Sint16[n];
    Sint16 *sy = new Sint16[n];
    int ret;

    for (int i=0; i<n; i++)
    {
	ScreenCoord s = view.coord(points[i]);
	sx[i] = s.x;
	sy[i] = s.y;
    }

    ret = ( filled ? filledPolygonColor :
	    useAA ? aapolygonColor : polygonColor )(
		surface, sx, sy, n, colour);

    delete[] sx;
    delete[] sy;
    return ret;
}

int Pixel::draw(SDL_Surface* surface, const View& view, View* boundView, bool noAA)
{
    if (boundView && !boundView->inView(point))
	return 0;
    else
    {
	ScreenCoord c = view.coord(point);
	return pixelColor(surface, c.x, c.y, colour);
    }
}
