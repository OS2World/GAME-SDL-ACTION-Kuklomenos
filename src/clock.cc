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

#include "clock.h"

GameClock::GameClock(int rate) :
    rate(rate),
    paused(false),
    ticks(0)
{}

int GameClock::scale(int time)
{
    if (paused)
	return 0;
    else
	return time * rate / 1000;
}

void GameClock::update(int time)
{
    ticks += scale(time);
}
void GameClock::updatePreScaled(int time)
{
    ticks += time;
}


