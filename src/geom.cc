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

#include "geom.h"
#include "gfx.h"
#include "coords.h"

ScreenGeom::ScreenGeom(int iwidth, int iheight) :
    width(iwidth), height(iheight),
    centre(iwidth/2, iheight/2),
    rad(std::min(iwidth,iheight)/2 - 20)
{
    // info = centre + [(rad+[room for infoMaxLines lines]) out at pi/6],
    // where infoMaxLines is preferably 3, less if necessary.
    // We assume a 7x13 font with two pixels gap between lines.
    infoMaxLines = 3;
    while ((int)((rad+17*infoMaxLines)*0.866) > centre.y)
	infoMaxLines--;
    info.x = centre.x + (rad+17*infoMaxLines)/2;
    info.y = centre.y - (int)((rad+17*infoMaxLines)*0.866);
    infoMaxLength = (width - info.x) / 7;

    indicatorRsqLim1 = (rad+3)*(rad+3);
    indicatorRsqLim2 = (rad+8)*(rad+8);
    indicatorRsqLim3 = (rad+10)*(rad+10);
    indicatorRsqLim4 = (rad+15)*(rad+15);
}

ScreenGeom screenGeom;

int ARENA_RAD = 220;

float ZOOMDIST_MAX = (float)((ARENA_RAD/2));

float AIM_MIN = 20.0;
float ZOOM_MIN = 0.0;
float AIM_MAX = ZOOMDIST_MAX;

CartCoord ARENA_CENTRE(0,0);
