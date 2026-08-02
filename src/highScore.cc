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

#include "net.h"
#include "overlay.h"
#include "conffile.h"
#include <SDL_gfxPrimitivesDirty.h>

#include <SDL/SDL.h>
#include <sstream>

const char* HSProtocolVersion = "v1";

Overlay highOverlay(-0.1);
Overlay lowOverlay(0.1);

bool inputText(SDL_Surface* surface, char* text, int maxlen)
{
    // quick-and-dirty text entry:
    int i=0;
    text[i] = '\0';
    SDL_Event event;
    int done = false;
    while (!done)
    {
	SDL_Delay(50);
	int redraw = false;
	while (SDL_PollEvent(&event))
	    switch (event.type)
	    {
		case SDL_QUIT:
		    return false;
		case SDL_KEYDOWN:
		    SDLKey sym = event.key.keysym.sym;
		    char c=0;
		    if (0 <= sym - SDLK_0 && sym - SDLK_0 < 10 &&
			    !(event.key.keysym.mod & KMOD_SHIFT))
			c = (sym - SDLK_0) + '0';
		    else if (0 <= sym - SDLK_a && sym - SDLK_a < 26)
			if (!(event.key.keysym.mod & KMOD_SHIFT))
			    c = (sym - SDLK_a) + 'a';
			else
			    c = (sym - SDLK_a) + 'A';
		    if (c)
		    {
			if (i < maxlen)
			{
			    text[i++] = c;
			    text[i] = '\0';
			    redraw = true;
			}
		    }

		    else if ( (sym == SDLK_DELETE || sym == SDLK_BACKSPACE) &&
			    i > 0 )
		    {
			text[--i] = '\0';
			redraw = true;
		    }

		    else if (sym == SDLK_ESCAPE)
			return false;

		    else if (sym == SDLK_RETURN)
			done = true;

		    break;
	    }
	if (redraw)
	{
	    lowOverlay.drawstr(surface, text);
	    highOverlay.drawstr(surface, "Pick a username:");
	    SDL_Flip(surface);
	    blankDirty();
	}
    }
    return true;
}
void waitKey()
{
    bool down = false;
    while (true)
    {
	SDL_Delay(50);
	SDL_Event event;
	while (SDL_PollEvent(&event))
	    switch (event.type)
	    {
		case SDL_QUIT:
		    return;
		case SDL_KEYDOWN:
		    down = true;
		    break;
		case SDL_KEYUP:
		    if (down)
			return;
		    break;
	    }
    }
}
void showErr(SDL_Surface* surface, const char* error)
{
    highOverlay.colour = lowOverlay.colour = 0xff0000ff;
    highOverlay.drawstr(surface, "Error:");
    lowOverlay.drawstr(surface, error);

    SDL_Flip(surface);
    blankDirty();

    waitKey();
}
bool reportHighScore(SDL_Surface* surface, int speed)
{
    const double highRating = config.highestRating[speed];
    if (highRating <= 5.0)
	return false;
    highOverlay.colour = lowOverlay.colour = 0x00ffffff;
    if (config.username[0] == '\0')
    {
	highOverlay.drawstr(surface, "Pick a username:");
	SDL_Flip(surface);
	blankDirty();

	char username[17];
	bool ret = inputText(surface, username, 16);
	if (!ret)
	{
	    return false;
	}
	strncpy(config.username, username, 17);
    }

    if (config.uuid == 0)
    {
	std::string response = "";
	ostringstream command;
	command << HSProtocolVersion << ":" << "reg:" << config.username;
	hssResponseCode ret = doHSSCommand(command.str(), response);

	if (ret == HSS_ERROR || HSS_FAIL)
	{
	    config.username[0]='\0';
	    if (ret == HSS_ERROR)
		showErr(surface, response.c_str());
	    else
		showErr(surface, "Server communication failed");
	    return false;
	}
	else
	{
	    int n = sscanf(response.c_str(), "%x", &config.uuid);
	    if ( n != 1 )
	    {
		showErr(surface, "Server communication failed");
		return false;
	    }
	}
    }

    highOverlay.drawstr(surface, "Submitting high score");
    ostringstream s;
    s << config.username << " - " << highRating << ", " << speedString(speed);
    lowOverlay.drawstr(surface, s.str());
    SDL_Flip(surface);
    blankDirty();

    std::string response = "";
    ostringstream command;
    command << HSProtocolVersion << ":" << "sub:" << hex << config.uuid << dec
	<< ":" << speed << ":" << obfuscatedRating(highRating);
    hssResponseCode ret = doHSSCommand(command.str(), response);

    if (ret == HSS_ERROR || ret == HSS_FAIL)
    {
	if (ret == HSS_ERROR)
	    showErr(surface, response.c_str());
	else
	    showErr(surface, "Server communication failed");
	return false;
    }
    else
    {
	highOverlay.drawstr(surface, "Rating Submitted");
	lowOverlay.drawstr(surface, response);
	SDL_Flip(surface);
	blankDirty();
	waitKey();
	return true;
    }
}
