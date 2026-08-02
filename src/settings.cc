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

#include "config.h"

#include "settings.h"
#include "conffile.h"
#include "keybindings.h"
#include "SDL_gfxPrimitivesDirty.h"
#include <getopt.h>
#include <SDL/SDL.h>
#include <vector>

string bgTypeStrings[] = { "none", "stars", "solar" };

Settings::Settings() :
    debug(false), wizard(false), invuln(false),
    useAA(AA_YES), showGrid(true), zoomEnabled(true), rotatingView(true),
    turnRateFactor(1.0), requestedRating(0), speed(0), stopMotion(false),
    keybindings(defaultKeybindings()), commandToBind(C_NONE), 
    bgType(BG_NONE),
    fps(30), showFPS(true), width(0), height(0), bpp(16),
    videoFlags(SDL_RESIZABLE | SDL_SWSURFACE), sound(true), volume(1.0),
    soundFreq(44100),
    clockRate(1000)
{
}

Settings settings;

int rateOfSpeed(int speed)
{ return 1000 + 500*speed; }

const char* speedString(int speed)
{ return speed == 0 ? "Normal" : speed == 1 ? "Fast" : "Very Fast"; }

const char* speedStringShort(int speed)
{ return speed == 0 ? "N" : speed == 1 ? "F" : "VF"; }

#ifndef __APPLE__
void load_settings(int argc, char** argv)
{
    config.read();
    config.exportSettings(settings);

    while (1)
    {
	int c;
	static const struct option long_options[] =
	{
	    {"width", 1, 0, 'W'},
	    {"height", 1, 0, 'H'},
	    {"bpp", 1, 0, 'b'},
	    {"fps", 1, 0, 'f'},
	    {"rating", 1, 0, 'r'},
	    {"gamma", 1, 0, 'g' << 8},
	    {"noantialias", 0, 0, 'A'},
	    {"nogrid", 0, 0, 'G'},
	    {"nozoom", 0, 0, 'Z'},
	    {"norotate", 0, 0, 'R'},
	    {"antialias", 0, 0, 'a'},
	    {"grid", 0, 0, 'g'},
	    {"zoom", 0, 0, 'z'},
	    {"rotate", 0, 0, '@'},
	    {"turnrate", 1, 0, 't'},
	    {"fullscreen", 0, 0, 'F'},
	    {"hwsurface", 0, 0, 's'},
	    {"hwpalette", 0, 0, 'P'},
	    {"noresizable", 0, 0, 'S'},
	    {"nosound", 0, 0, 'q'},
	    {"debug", 0, 0, 'd'},
	    {"xyzzy", 0, 0, '+'},
	    {"invulnerable", 0, 0, 'i'},
	    {"stopmotion", 0, 0, 'M'},
	    {"speed", 1, 0, 'p'},
	    {"aispeed", 1, 0, '-'},
	    {"version", 0, 0, 'V'},
	    {"help", 0, 0, 'h'},
	    {0,0,0,0}
	};
	c = getopt_long(argc, argv, "W:H:b:f:r:t:I:p:AGZRagzFsPSqdiMVh",
		long_options, NULL);
	if (c == -1)
	    break;
	switch (c)
	{
	    case 'W':
		settings.width = atoi(optarg);
		break;
	    case 'H':
		settings.height = atoi(optarg);
		break;
	    case 'b':
		settings.bpp = atoi(optarg);
		break;
	    case 'f':
		settings.fps = atoi(optarg);
		break;
	    case 'r':
		if (1.0 <= atof(optarg))
		    settings.requestedRating = atof(optarg);
		else
		{
		    printf("bad rating\n");
		    exit(1);
		}
		break;
	    case 'g'<<8:
		antialiasGamma = atoi(optarg);
		break;
	    case 'F':
		settings.videoFlags |= SDL_FULLSCREEN;
		break;
	    case 'S':
		settings.videoFlags &= ~SDL_RESIZABLE;
		break;
	    case 'P':
		settings.videoFlags |= SDL_HWPALETTE;
		break;
	    case 's':
		settings.videoFlags |= SDL_HWSURFACE;
		break;
	    case 'Z':
		settings.zoomEnabled = false;
		break;
	    case 'R':
		settings.rotatingView = false;
		break;
	    case 'A':
		settings.useAA =
		    (settings.useAA == AA_FORCE) ? AA_YES :
		    AA_NO;
		break;
	    case 'G':
		settings.showGrid = false;
		break;
	    case 'z':
		settings.zoomEnabled = true;
		break;
	    case '@':
		settings.rotatingView = true;
		break;
	    case 't':
		if (0 < atof(optarg) <= 1.0)
		    settings.turnRateFactor = atof(optarg);
		break;
	    case 'a':
		settings.useAA =
		    (settings.useAA == AA_NO) ? AA_YES :
		    AA_FORCE;
		break;
	    case 'g':
		settings.showGrid = true;
		break;
	    case 'q':
		settings.sound = false;
		break;
	    case 'd':
		settings.debug = true;
		break;
	    case '+':
		settings.wizard = true;
		settings.debug = true;
		break;
	    case 'i':
		settings.invuln = true;
		settings.debug = true;
		break;
	    case 'M':
		settings.stopMotion = true;
		settings.debug = true;
		break;
	    case 'p':
		settings.speed = (int)(atof(optarg));
		if (settings.speed < 0) settings.speed = 0;
		if (settings.speed > 2) settings.speed = 2;
		break;
	    case 'V':
		printf("%s\n", PACKAGE_STRING);
		exit(0);
	    case 'h':
		printf("Options:\n\t"
			"-W --width WIDTH\n\t-H --height HEIGHT\n\t-b --bpp BITS\n\t-f --fps FPS\n\t"
			"-F --fullscreen\n\t-S --noresizable\n\t-P --hwpalette\n\t-s --hwsurface\n\t"
			"-Z,-z --[no]zoom\n\t-R --[no]rotate\n\t-G,-g --[no]grid\n\t-A,-a --[no]antialias\n\t"
			"-t --turnrate 0.1-1.0\n\t"
			"-r --rating RATING\t\t1: harmless... 4: average... 9: elite\n\t"
			"--gamma GAMMA\n\t"
#ifdef SOUND
			"-q --nosound\n\t"
#endif
			"-p --speed 0-2\n\n\t"
			"-d --debug\n\t-i --invulnerable\t\timplies --debug\n\t-M --stopmotion\t\t\timplies --debug\n\n\t"
			"-V --version\n\t-h --help\n");
		exit(0);
		break;

	    case '?':
		exit(1);
		break;

	    default:
		printf("getopt returned unexpected character %c\n", c);
	}
    }
}
#endif

// lexicographic comparison, widthfirst
int Rect::cmp(const Rect& r1, const Rect& r2)
{
    if (r1.w > r2.w)
	return 1;
    if (r1.w < r2.w)
	return -1;
    if (r1.h > r2.h)
	return 1;
    if (r1.h < r2.h)
	return -1;
    return 0;
}

std::vector<Rect> getSDLModes()
{
    std::vector<Rect> modes;
    int numModes = SDL_GetNumDisplayModes(0);
    if (numModes < 1)
    {
        // fallback: return standard modes
        modes.push_back(Rect(640, 480));
        modes.push_back(Rect(800, 600));
        modes.push_back(Rect(1024, 768));
        modes.push_back(Rect(1280, 1024));
        return modes;
    }
    for (int i = 0; i < numModes; i++)
    {
        SDL_DisplayMode mode;
        if (SDL_GetDisplayMode(0, i, &mode) == 0)
            modes.push_back(Rect(mode.w, mode.h));
    }
    return modes;
}
