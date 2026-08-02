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

#include "overlay.h"
#include "geom.h"
#include "data.h"
#include "SDL_gfxPrimitivesDirty.h"

#include <string>
#include <vector>
#include <SDL/SDL.h>
using namespace std;

void Overlay::drawstr(SDL_Surface* surface, string str, Uint8 alpha)
{
    clear();
    push_back(str);
    draw(surface);
    clear();
}

void Overlay::draw(SDL_Surface* surface, Uint8 alpha)
{
    if (!drawWithFont(surface, fontBig, 10, 20, alpha))
	drawWithFont(surface, fontSmall, 7, 13, alpha, true);
}

bool Overlay::drawWithFont(SDL_Surface* surface, const void *fontdata, int cw,
	int ch, Uint8 alpha, bool force)
{
    const unsigned int maxlen = (2*3*screenGeom.rad/5)/cw;

    vector<string> splitStrings;

    for (vector<string>::iterator it = begin();
	    it != end(); it++)
    {
	if (!force && it->length() > maxlen)
	    return false;

	if (!splitStrings.empty() &&
		it->length() + splitStrings.back().length() < maxlen)
	    splitStrings.back() += " " + *it;
	else
	    splitStrings.push_back(*it);
    }

    gfxPrimitivesSetFont(fontdata, cw, ch);

    const int ystart = screenGeom.centre.y + int(screenGeom.rad*offy) -
	( splitStrings.size() * ch - ch/2 +
	  (splitStrings.size() - 1) * ch/2 )/2;
    const int yinc = ch + ch/2;

    int i = 0;
    for (vector<string>::iterator it = splitStrings.begin();
	    it != splitStrings.end();
	    it++, i++)
    {
	const int strx = screenGeom.centre.x - it->length()*cw/2;
	const int stry = ystart + i*yinc;
	stringColor(surface, strx, stry, it->c_str(),
		(alpha == 0) ? colour
		    : (colour >> 8 << 8) + alpha);
    }
    return true;
}
