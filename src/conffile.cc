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
#include <cstdlib>
#include <sstream>
using namespace std;

#include "conffile.h"
#include "keybindings.h"
#include "SDL_gfxPrimitivesDirty.h"

int powmod(int n, int m, int p)
{
    int out = 1;
    while (m > 0)
	if (m % 2 == 0)
	{
	    n = (n*n) % p;
	    m /= 2;
	}
	else
	{
	    out = (out*n) % p;
	    m -= 1;
	}
    return out;

}

/* obfuscate the rating string: if you want to cheat, you at least have to
 * source-dive...
 */
int obfuscatedRating(double rating)
{
    int pertinentRating = (int)(round(10*rating));
    return powmod(pertinentRating, 5, 45953);
}
double unobfuscatedRating(int obfuscated)
{
    double unobf = powmod(obfuscated, 18381, 45953)/10.0;
    if (unobf < 0 || unobf > 20)
	return 0.0;
    return unobf;
}

Config::Config() :
    useAA(AA_YES), showGrid(true), zoomEnabled(true), rotatingView(true),
    turnRateFactor(1.0), fps(30), showFPS(true), sound(true), volume(1.0),
    soundFreq(44100),
    aaGamma(2.2),
    shouldUpdateRating(false), uuid(0),
    rebindings(),
    bgType(BG_NONE),
    width(0), height(0), bpp(16), fullscreen(false),
    confFileVersion(CONFFILE_VERSION_FIRST)
{
    username[0] = '\0';
    for (int i=0; i<3; i++)
	rating[i] = highestRating[i] = 5.0;
}

void Config::read()
{
    ifstream f;
    char* home = getenv("HOME");

    if (home != NULL)
    {
	string homeStr = home;
	f.open((home + (string)"/.kuklomenosrc").c_str());
    }
    if (!f.is_open())
	f.open("./kuklomenosrc.txt");

    if (f.is_open())
    {
	string line;
	double val, val2, val3, val4, val5, val6;
	char buf[31];
	char buf2[31];
	unsigned int uint;
	int i, i2, i3;
	int n;

	while (!f.eof())
	{
	    std::getline(f, line);
	    const char* cstr = line.c_str();
	    n = sscanf(cstr, "data: %lf %lf %lf %lf %lf %lf", &val, &val2,
		    &val3, &val4, &val5, &val6);
	    if (n > 0)
		for (int i=0; i<3; i++)
		{
		    if (n > 2*i)
		    {
			rating[i] = unobfuscatedRating( (int)(
				    i==0 ? val : i==1 ? val3 : val5
				    ) );
			if (n > 2*i+1)
			    highestRating[i] = unobfuscatedRating( ((int)(
					    i==0 ? val2 : i==1 ? val4 : val6
					    )-1337-37*i)/2 );
		    }
		}
	    else if (sscanf(cstr, "confversion: %lf", &val) == 1)
		confFileVersion = val;
	    else if (sscanf(cstr, "showGrid: %lf", &val) == 1)
		showGrid = val;
	    else if (sscanf(cstr, "zoomEnabled: %lf", &val) == 1)
		zoomEnabled = val;
	    else if (sscanf(cstr, "rotatingView: %lf", &val) == 1)
		rotatingView = val;
	    else if (sscanf(cstr, "turnRate: %lf", &val) == 1)
		turnRateFactor = val;
	    else if (sscanf(cstr, "showFPS: %lf", &val) == 1)
		showFPS = val;
	    else if (sscanf(cstr, "fullscreen: %lf", &val) == 1)
		fullscreen = val;
	    else if (sscanf(cstr, "speed: %lf", &val) == 1)
	    {
		speed = (int) val;
		if (speed < 0) speed = 0;
		if (speed > 2) speed = 2;
	    }
	    else if (sscanf(cstr, "sound: %lf", &val) == 1)
		sound = val;
	    else if (sscanf(cstr, "volume: %lf", &val) == 1)
		volume = val;
	    else if (sscanf(cstr, "freq: %lf", &val) == 1)
		soundFreq = int(val);
	    else if (sscanf(cstr, "antialiasGamma: %lf", &val) == 1)
		aaGamma = val;
	    else if (sscanf(cstr, "fps: %lf", &val) == 1)
		fps = (int) val;
	    else if (sscanf(cstr, "useAA: %lf", &val) == 1)
		useAA = (val == 0 ? AA_NO :
			val == 2 ? AA_FORCE :
			AA_YES);
	    else if (sscanf(cstr,
			"mode: %dx%dx%d", &i, &i2, &i3) == 3)
	    {
		width = (int) i;
		height = (int) i2;
		bpp = (int) i3;
	    }
	    else if (sscanf(cstr, "username: %16s", buf) == 1)
		strncpy(username, buf, 17);
	    else if (strcmp(cstr, "username: ") == 0)
		username[0] = '\0';
	    else if (sscanf(cstr, "uuid: %x", &uint) == 1)
		uuid = uint;
	    else if (sscanf(cstr, "background: %30s", buf) == 1)
	    {
		for (BGType bg = BG_FIRST; bg <= BG_LAST; bg = BGType(bg+1))
		    if (bgTypeStrings[bg].compare(buf) == 0)
			bgType = bg;
	    }
	    else if (sscanf(cstr, "bind %30s %30s", buf, buf2) == 2)
	    {
		command c = commandOfString(buf);
		Key k(buf2);
		if (c == C_NONE)
		    fprintf(stderr, "Unparsable command: %s\n", buf);
		else if (k == KEY_NONE)
		    fprintf(stderr, "Unparsable key description: %s\n", buf2);
		else
		    rebindings[c] = k;
	    }
	    else if (*cstr != '\0' && *cstr != '#')
		fprintf(stderr, "Unparsable line in config file: %s\n", cstr);
	}
    }
}

void Config::importSettings(const Settings& settings)
{
    useAA = settings.useAA;
    showGrid = settings.showGrid;
    zoomEnabled = settings.zoomEnabled;
    rotatingView = settings.rotatingView;
    turnRateFactor = settings.turnRateFactor;
    fps = settings.fps;
    showFPS = settings.showFPS;
    sound = settings.sound;
    volume = settings.volume;
    soundFreq = settings.soundFreq;
    speed = settings.speed;
    bgType = settings.bgType;

    width = settings.width;
    height = settings.height;
    bpp = settings.bpp;
    fullscreen = (settings.videoFlags & SDL_FULLSCREEN);

    aaGamma = antialiasGamma;

    rebindings.clear();
    for (command c=C_FIRST; c <= C_LASTDEBUG; c = command(c+1))
    {
	if (settings.keybindings.get(c) != defaultKeybindings().get(c))
	    rebindings[c] = settings.keybindings.get(c);
    }
}
void Config::exportSettings(Settings& settings) const
{
    settings.useAA = useAA;
    settings.showGrid = showGrid;
    settings.zoomEnabled = zoomEnabled;
    settings.rotatingView = rotatingView;
    settings.turnRateFactor = turnRateFactor;
    settings.fps = fps;
    settings.showFPS = showFPS;
    settings.sound = sound;
    settings.volume = volume;
    settings.soundFreq = soundFreq;
    settings.speed = speed;
    settings.bgType = bgType;

    settings.width = width;
    settings.height = height;
    settings.bpp = bpp;
    if (fullscreen)
	settings.videoFlags |= SDL_FULLSCREEN;
    else
	settings.videoFlags &= ~SDL_FULLSCREEN;

    antialiasGamma = aaGamma;

    for ( map<command,Key>::const_iterator it = rebindings.begin();
	    it != rebindings.end(); it++)
	settings.keybindings[it->first] = it->second;
}

void Config::write() const
{
    ofstream f;
    char* home = getenv("HOME");

    if (home != NULL)
    {
	string homeStr = home;
	f.open((home + (string)"/.kuklomenosrc").c_str());
    }
    if (!f.is_open())
	f.open("./kuklomenosrc.txt");

    if (!f.is_open())
	fprintf(stderr, "Couldn't write config file.\n");
    else
    {
	ostringstream s;
	string bindingsString;
	int aanum = (useAA == AA_NO ? 0 :
		useAA == AA_YES ? 1 : 2);
	if (!rebindings.empty())
	{
	    bindingsString = '\n';
	    for ( map<command,Key>::const_iterator it = rebindings.begin();
		    it != rebindings.end(); it++)
		bindingsString += ( string("bind ") + shortCommandNames[it->first] +
		    string(" ") + (it->second).getString() + string("\n"));
	}
	s <<
	    "useAA: " << aanum << endl <<
	    "showGrid: " << showGrid << endl <<
	    "zoomEnabled: " << zoomEnabled << endl <<
	    "rotatingView: " << rotatingView << endl <<
	    "turnRate: " << turnRateFactor << endl <<
	    "fps: " << fps << endl <<
	    "showFPS: " << showFPS << endl <<
	    "speed: " << speed << endl <<
	    "sound: " << sound << endl <<
	    "volume: " << volume << endl <<
	    "freq: " << soundFreq << endl <<
	    "antialiasGamma: " << aaGamma << endl << endl <<
	    "mode: " << width << "x" << height << "x" << bpp << endl <<
	    "fullscreen: " << fullscreen << endl << endl <<
	    "background: " << bgTypeStrings[bgType] << endl << endl <<
	    "username: " << username << endl <<
	    "uuid: " << hex << uuid << dec << endl <<
	    bindingsString <<
	    endl << "# Warning: Do not edit following lines" << endl <<
	    "confversion: " << confFileVersion << endl <<
	    "data: " << obfuscatedRating(rating[0]) << " " << (obfuscatedRating(highestRating[0])*2+1337)
	    << " " << obfuscatedRating(rating[1]) << " " << (obfuscatedRating(highestRating[1])*2+1337+37)
	    << " " << obfuscatedRating(rating[2]) << " " << (obfuscatedRating(highestRating[2])*2+1337+37*2)
	    << endl;
	f << s.str();
    }
}

Config config;
