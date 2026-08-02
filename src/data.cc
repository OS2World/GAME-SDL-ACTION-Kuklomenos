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

#include <iostream>
#include <fstream>
#include <string>
using namespace std;

#include <SDL_gfxPrimitivesDirty.h>

#include "data.h"

char fontSmall[3328];
char fontBig[10240];

#ifndef DATADIR
const int numDataPaths = 4;
const string dataPaths[numDataPaths] = { "./", "/usr/share/kuklomenos/",
    "/usr/local/share/kuklomenos/", "/usr/share/games/kuklomenos/"};
#else
const int numDataPaths = 5;
const string dataPaths[numDataPaths] = { DATADIR "/kuklomenos/",
    "./", "/usr/share/kuklomenos/", "/usr/local/share/kuklomenos/",
    "/usr/share/games/kuklomenos/"};
#endif

ifstream* openDataFile(string relPath)
{
    ifstream* f = new ifstream;
    for (int i=0; i < numDataPaths; i++)
    {
	f->open((dataPaths[i]+relPath).c_str(), ios::binary);
	if (f->is_open())
	    return f;
	else
	    continue;
    }
    return f;
}

const string findDataPath(string relPath)
{
    ifstream f;
    for (int i=0; i < numDataPaths; i++)
    {
	const string absPath = dataPaths[i] + relPath;
	f.open(absPath.c_str(), ios::binary);
	if (f.is_open())
	{
	    f.close();
	    return absPath;
	}
	else
	    continue;
    }
    return "";
}

bool initFont()
{
    ifstream* f = openDataFile("fonts/7x13.fnt");
    ifstream* f2 = openDataFile("fonts/10x20.fnt");

    if (!f->is_open())
	return false;

    f->read(fontSmall, 3328);
    f2->read(fontBig, 10240);

    delete f;
    delete f2;

    return true;
}
