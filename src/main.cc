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

#include <config.h>

#ifdef HAVE_LIBCURL
#define HIGH_SCORE_REPORTING 1
#endif

#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <stack>
#include <SDL2/SDL.h>
#include <SDL_gfxPrimitivesDirty.h>
#include <string>
#include <sstream>

#include "state.h"
#include "geom.h"
#include "clock.h"
#include "overlay.h"
#include "data.h"
#include "settings.h"
#include "conffile.h"
#include "random.h"
#include "ai.h"
#include "menu.h"
#include "keybindings.h"
#include "sound.h"
#include "background.h"

#ifdef HIGH_SCORE_REPORTING
# include "highScore.h"
#endif

#ifndef __APPLE__
# include "config.h"
#endif


SDL_Surface* screen = NULL;
SDL_Window* sdl_window = NULL;
std::stack<Menu*> menuStack;

enum EventsReturn
{
    ER_NONE,
    ER_MISC,
    ER_QUIT,
    ER_SURRENDER,
    ER_RESTART,
    ER_SCREENSHOT,
    ER_NEWBACKGROUND,
    ER_NOTIMETAKEN,
    ER_MENU
};

bool setVideoMode()
{
    if (!sdl_window) return false;
    SDL_SetWindowSize(sdl_window, settings.width, settings.height);
    screen = SDL_GetWindowSurface(sdl_window);
    if (!screen)
    {
        settings.width = screen ? screen->w : settings.width;
        settings.height = screen ? screen->h : settings.height;
        return false;
    }
    settings.width = screen->w;
    settings.height = screen->h;
    settings.bpp = screen->format->BitsPerPixel;
    screenGeom = ScreenGeom(settings.width, settings.height);
    setBackground(screen);
    setDirty(screen, background);
    return true;
}

EventsReturn handleCommand(command c, GameState* gameState, GameClock& gameClock)
{
    switch (c)
    {
	case C_QUIT:
	    if (menuStack.empty())
		return ER_QUIT;
	    else
		while (!menuStack.empty())
		    menuStack.pop();
	    break;
	case C_PAUSE:
	    gameClock.paused = !gameClock.paused;
	    break;
	case C_MENU:
	    if (menuStack.empty())
		menuStack.push(&topMenu);
	    else
		menuStack.pop();
	    break;
	case C_STARTGAME:
	    if (!menuStack.empty())
		menuStack.pop();
	    else if (gameState->end || gameState->ai)
		return ER_RESTART;
	    break;
#ifdef SOUND
	case C_SOUND:
	    settings.sound = !settings.sound;
	    break;
#endif
	case C_ZOOM:
	    settings.zoomEnabled = !settings.zoomEnabled;
	    break;
	case C_ROTATE:
	    settings.rotatingView = !settings.rotatingView;
	    break;
	case C_INCAA:
	    settings.useAA =
		(settings.useAA == AA_NO) ? AA_YES :
		AA_FORCE;
	    break;
	case C_DECAA:
	    settings.useAA =
		(settings.useAA == AA_FORCE) ? AA_YES :
		AA_NO;
	    break;
	case C_GRID:
	    settings.showGrid = !settings.showGrid;
	    break;
	case C_DECFPS:
	    settings.fps = std::max(1, settings.fps-1);
	    break;
	case C_INCFPS:
	    settings.fps = std::min(100, settings.fps+1);
	    break;
	case C_DECRATE:
	    if (settings.debug || gameState->end || gameState->ai)
	    {
		gameClock.rate = 4*gameClock.rate/5;
		const int diff =
		    gameClock.rate - rateOfSpeed(gameState->speed);
		if (abs(diff) < 100)
		    gameClock.rate = rateOfSpeed(gameState->speed);
	    }
	    break;
	case C_RESETRATE:
	    gameClock.rate = rateOfSpeed(gameState->speed);
	    break;
	case C_INCRATE:
	    if (settings.debug || gameState->end || gameState->ai)
	    {
		gameClock.rate = std::max(5*gameClock.rate/4,
			gameClock.rate+1);
		const int diff =
		    gameClock.rate - rateOfSpeed(gameState->speed);
		if (abs(diff) < 100)
		    gameClock.rate = rateOfSpeed(gameState->speed);
	    }
	    break;
	case C_INVULN:
	    if (settings.debug)
		settings.invuln = !settings.invuln;
	    break;
	case C_WIN:
	    if (settings.debug)
		gameState->end = END_WIN;
	    break;

#ifdef HIGH_SCORE_REPORTING
	case C_REPORTHS:
	    if (gameState->end || gameClock.paused)
	    {
		reportHighScore(screen, gameState->speed);
	    }
	    return ER_NOTIMETAKEN;
#endif
	case C_SCREENSHOT:
	    return ER_SCREENSHOT;

	default: 

	    if (!menuStack.empty())
	    {
		int dir =
		    (c == C_M_LEFT || c == C_LEFT) ? 0 :
		    (c == C_M_UP || c == C_DEZOOM) ? 1 :
		    (c == C_M_RIGHT || c == C_RIGHT) ? 2 :
		    (c == C_M_DOWN || c == C_DEAIM) ? 3 :
		    -1;

		if (dir < 0)
		    break;

		Menu* submenu = menuStack.top()->menus[dir];
		MenuLeaf* leaf = menuStack.top()->leaves[dir];

		if (submenu)
		{
		    menuStack.push(submenu);
		}
		else if (leaf)
		{
		    LeafReturn lr = leaf->act();
		    switch (lr)
		    {
			case LR_QUIT:
			    return ER_QUIT;
			case LR_SURRENDER:
			    while (!menuStack.empty())
				menuStack.pop();
			    return ER_SURRENDER;
			case LR_NEWSPEED:
			    if (gameState->ai)
			    {
				// update speed on the fly, so the player
				// can see what they're getting themself
				// into...
				gameClock.rate = rateOfSpeed(settings.speed);
			    }
			    break;
			case LR_SETRES:
			    setVideoMode();
			    break;
			case LR_EXITMENU:
			    menuStack.pop();
			    break;
			case LR_SAVESETTINGS:
			    config.importSettings(settings);
			    config.write();
			    menuStack.pop();
			    break;
			case LR_NEWBACKGROUND:
			    return ER_NEWBACKGROUND;
			    break;
			default: ;
		    }
		}
	    }
    }
    return ER_MISC;
}

EventsReturn process_events(GameState* gameState, GameClock& gameClock)
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
	if (event.type == SDL_QUIT)
	    return ER_QUIT;
	else if (event.type == SDL_WINDOWEVENT &&
		 event.window.event == SDL_WINDOWEVENT_RESIZED)
	{
	    settings.width = event.window.data1;
	    settings.height = event.window.data2;
	    setVideoMode();
	    return ER_MISC;
	}
	else if (event.type == SDL_KEYDOWN)
	{
	    Key key(event.key.keysym);

	    if (settings.commandToBind != C_NONE)
	    {
		SDL_Scancode sc = SDL_GetScancodeFromKey(key.sym);
		if (sc >= SDL_SCANCODE_LCTRL && sc <= SDL_SCANCODE_RGUI)
		    // modifier - ignore
		    return ER_NONE;

		settings.keybindings[settings.commandToBind] = key;

		settings.commandToBind = C_NONE;
		return ER_MISC;
	    }

	    for (command c = C_FIRST; c <= C_LASTDEBUG; c = command(c+1))
		if (settings.keybindings.get(c) == key)
		    return handleCommand(c, gameState, gameClock);
	}
    }
    return ER_NONE;
}

void drawInfo(SDL_Surface* surface, GameState* gameState,
	GameClock& gameClock, float observedFPS)
{
    int shownFPS = int(round(observedFPS));
    char fpsStr[5+10+9];
    snprintf(fpsStr, 5+10+9, "fps: %d/%d%s", shownFPS,
	    settings.fps, gameClock.paused ? " [Paused]" : "");

    char ratingStr[8+20+7+3];
    snprintf(ratingStr, 8+20+7+3, "rating: %.1f %s (%s)",
	    gameState->rating, ratingString((int)(gameState->rating)),
	    speedStringShort(gameState->speed));

    char rateStr[6+5+10];
    snprintf(rateStr, 6+5+10, "speed: %d.%d%s", gameClock.rate/1000,
	    (gameClock.rate%1000)/100, settings.debug ? " [*DEBUG*]" : "");

    // if not enough room, try short version; if still too long don't display:
    if ((int)strlen(fpsStr) > screenGeom.infoMaxLength)
	snprintf(fpsStr, 5+10+9, "F: %d%s", shownFPS,
		gameClock.paused ? " [P]" : "" );
    if ((int)strlen(fpsStr) > screenGeom.infoMaxLength)
	*fpsStr = '\0';

    if ((int)strlen(ratingStr) > screenGeom.infoMaxLength)
	snprintf(ratingStr, 8+20+7+5, "R: %.1f (%s)", gameState->rating,
		speedStringShort(gameState->speed));
    if ((int)strlen(ratingStr) > screenGeom.infoMaxLength)
	*ratingStr = '\0';

    if ((int)strlen(rateStr) > screenGeom.infoMaxLength)
	snprintf(rateStr, 6+5+9, "S: %d.%d%s", gameClock.rate/1000,
		(gameClock.rate%1000)/100, settings.debug ? " [D]" : "");
    if ((int)strlen(rateStr) > screenGeom.infoMaxLength)
	*rateStr = '\0';

    gfxPrimitivesSetFont(fontSmall,7,13);

    int line = 0;
    if (settings.showFPS)
	stringColor(surface,
		screenGeom.info.x, screenGeom.info.y+15*line++,
		fpsStr, 0xffffffff);


    if (screenGeom.infoMaxLines > line)
    {
	stringColor(surface,
		screenGeom.info.x, screenGeom.info.y+15*line++,
		ratingStr, 0xffffffff);
    }

    if ( (settings.debug || gameClock.rate != rateOfSpeed(gameState->speed))
	    && screenGeom.infoMaxLines > line)
	stringColor(surface,
		screenGeom.info.x, screenGeom.info.y+15*line++,
		rateStr, 0xffffffff);
}

void drawSplash(SDL_Surface* surface)
{
    int x = screenGeom.centre.x - 19*10/2;
    int y = screenGeom.centre.y - screenGeom.rad/3;

    // fade over time, and dim if menu up
    const Uint32 titleColour = 0xff0000ff -
	std::min(0xa0, (int)(SDL_GetTicks()/500) +
		(menuStack.empty() ? 0 : 0x50));
    const Uint32 instructColour = 0xffffffa0 -
	std::min(0x60, (int)(SDL_GetTicks()/500) +
		(menuStack.empty() ? 0 : 0x40));

    gfxPrimitivesSetFont(fontBig, 10, 20);

    if (x > 0)
	stringColor(surface, x, y, "K U K L O M E N O S", titleColour);
    else
    {
	// Not enough room for that
	x = screenGeom.centre.x - 10*10/2;
	stringColor(surface, x, y, "KUKLOMENOS", titleColour);
    }

    static Overlay splashInstructOverlay(0.35);
    static bool setText = false;
    if (!setText)
    {
	splashInstructOverlay.push_back(
		string("  ") + settings.keybindings[C_STARTGAME].getString() +
		string(" to start  "));
	splashInstructOverlay.push_back(
		string("  ") + settings.keybindings[C_MENU].getString() +
		string(" for menu  "));
	setText = true;
    }

    splashInstructOverlay.colour = instructColour;

    splashInstructOverlay.draw(surface);
}


void initialize_system()
{
    /* Initialize SDL */
    if ( SDL_Init(SDL_INIT_EVERYTHING) < 0 ) {
	fprintf(stderr,
		"Couldn't initialize SDL: %s\n", SDL_GetError());
	exit(1);
    }
    atexit(SDL_Quit);			/* Clean up on exit */

    // set random seed
    srand(time(NULL));
}

void initialize_video()
{
    int w = (settings.width  > 0) ? settings.width  : 1024;
    int h = (settings.height > 0) ? settings.height : 768;

    sdl_window = SDL_CreateWindow("Kuklomenos",
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            w, h, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!sdl_window)
    {
        fprintf(stderr, "Couldn't create window: %s\n", SDL_GetError());
        exit(1);
    }

    screen = SDL_GetWindowSurface(sdl_window);
    if (!screen)
    {
        fprintf(stderr, "Couldn't get window surface: %s\n", SDL_GetError());
        exit(1);
    }

    settings.width  = screen->w;
    settings.height = screen->h;
    settings.bpp    = screen->format->BitsPerPixel;

    printf("Set %dx%dx%d mode\n", screen->w, screen->h,
            screen->format->BitsPerPixel);

    SDL_ShowCursor(SDL_DISABLE);

    if (!initFont())
    {
        fprintf(stderr, "Failed to load font data file\n");
        exit(1);
    }

    screenGeom = ScreenGeom(settings.width, settings.height);

    setBackground(screen);
    setDirty(screen, background);
}

bool haveInput()
{
    for (command c = C_FIRST; c <= C_LASTACTION; c = command(c+1))
	if (settings.keybindings.get(c).isPressed())
	    return true;
    return false;
}

void run_game()
{
    if (settings.requestedRating == 0 &&
	    (!settings.debug || settings.wizard))
	config.shouldUpdateRating = true;

    // set up a game for the AI to play...
    GameState* gameState = new GameState(settings.speed);
    gameState->ai = new BasicAI(gameState);
    GameClock gameClock(rateOfSpeed(settings.speed));

    // main loop
    Uint32 lastStateUpdate = 0, beforeDelay = 0;
    Uint32 ticksBefore, ticksAfter;
    Uint32 loopTicks = SDL_GetTicks();
    Uint32 AIEndTick = 0;
    bool splash = true;
    int timeTillNextFrame = 0;
    int delayTime, actualDelayed, updateTime;
    float avFrameTime = 1000/settings.fps;
    int fpsRegulatorTenthTicks = 0;
    const int avFrames = 10; // number of frames to average over
    EventsReturn eventsReturn = ER_NONE;
    bool wantScreenshot = false;
    bool wantVideo = false;
    int videoFrame = 1;
    bool quit = false;
    bool ended = false;
    bool forceFrame = false;

    Overlay victoryOverlay(-0.2);
    Overlay infoOverlay(0.2, 0xffffffff);

    const int MIN_INPUT_STEP = 30;
    const int MIN_GAME_STEP = 30;

    while ( !quit ) {
	forceFrame = false;
	while (timeTillNextFrame > 0)
	{
	    delayTime = std::min(timeTillNextFrame, MIN_INPUT_STEP);
	    beforeDelay = SDL_GetTicks();
	    SDL_Delay(delayTime);
	    actualDelayed = SDL_GetTicks() - beforeDelay;
	    timeTillNextFrame -= actualDelayed;

	    eventsReturn = process_events(gameState, gameClock);

	    switch (eventsReturn)
	    {
		case ER_RESTART:
		    {
			GameState* newGameState =
			    new GameState(settings.speed);
			delete gameState;
			gameState = newGameState;
			gameClock = GameClock(rateOfSpeed(settings.speed));
		    }
		    ended = false;
		    victoryOverlay.clear();
		    infoOverlay.clear();
		    splash = false;
		    drawBackground(screen);
		    lastStateUpdate = SDL_GetTicks();
		    break;
		case ER_QUIT:
		    quit = true;
		    break;
		case ER_SURRENDER:
		    if (!ended)
			gameState->end = END_DEAD;
		    break;
		case ER_SCREENSHOT:
		    wantScreenshot = true;
		    break;
		case ER_NEWBACKGROUND:
		    if (!background)
		    {
			setBackground(screen);
			setDirty(screen, background);
		    }
		    else
			drawBackground(screen);
		    forceFrame = true;
		    break;
		case ER_NOTIMETAKEN:
		    // pretend the time spent in process_events() didn't
		    // actually happen:
		    lastStateUpdate = SDL_GetTicks();
		    forceFrame = true;
		    break;
		case ER_MISC:
		    forceFrame = true;
		    break;
		default: ;
	    }

	    updateTime = std::max(1,
		    gameClock.scale(SDL_GetTicks() - lastStateUpdate));
	    lastStateUpdate = SDL_GetTicks();
	    if ( !gameClock.paused &&
		    ( (!settings.stopMotion || haveInput()) &&
		      menuStack.empty() ) || ended || gameState->ai ) 
	    {
		while (updateTime > 0)
		{
		    const int stepTime =
			std::min( MIN_GAME_STEP, updateTime );
		    gameState->update(stepTime, !menuStack.empty());
		    gameClock.updatePreScaled(stepTime);
		    updateTime -= stepTime;
		}
	    }
	}

	ticksBefore = SDL_GetTicks();
	if (!gameClock.paused || forceFrame)
	{
	    gameState->draw(screen);
	    drawInfo(screen, gameState, gameClock, 1000.0/avFrameTime);
	    victoryOverlay.draw(screen, menuStack.empty() ? 0xff : 0xa0);
	    infoOverlay.draw(screen, menuStack.empty() ? 0xff : 0xa0);
	    if (!menuStack.empty())
		drawMenu(screen, *menuStack.top());
	    if (splash)
		drawSplash(screen);
	    SDL_UpdateWindowSurface(sdl_window);

	    if (wantScreenshot)
	    {
		if (SDL_SaveBMP(screen, "screenshot.bmp") != 0)
		    fprintf(stderr, "Screenshot failed.\n");
		wantScreenshot = false;
	    }
	    if (wantVideo)
	    {
		char fname[16];
		snprintf(fname, 16, "frame%.5d.bmp", videoFrame);
		SDL_SaveBMP(screen, fname);
		videoFrame++;
	    }

	    // blank over what we've drawn:
	    blankDirty();
	}
	ticksAfter = SDL_GetTicks();

	const int renderingTicks = ticksAfter - ticksBefore;

	// Add in a manual tweak to the delay, to ensure we approximate the
	// requested fps (where possible):
	if (10000/avFrameTime < settings.fps * 10 &&
		1000/settings.fps - renderingTicks + fpsRegulatorTenthTicks/10
		> 1)
	    fpsRegulatorTenthTicks -= 1;
	else if (10000/avFrameTime > settings.fps * 10)
	    fpsRegulatorTenthTicks += 1;

	timeTillNextFrame =
	    std::max(1, (1000/settings.fps) - renderingTicks +
		    fpsRegulatorTenthTicks/10 + (rani(10) <
			fpsRegulatorTenthTicks%10));

	if (!ended && gameState->end && !gameState->ai)
	{
	    // Game over, man
	    ended = true;
	    switch (gameState->end)
	    {
		case END_DEAD:
		case END_EXTRACTED:
		    victoryOverlay.push_back("Failure.");
		    victoryOverlay.colour = 0xff4444ff;
		    break;
		case END_WIN:
		    victoryOverlay.push_back("Victory!");
		    victoryOverlay.colour = 0x88ff00ff;
		    break;
		default: ;
	    }

	    // give a hint if appropriate
	    const char* hint = gameState->getHint();
	    if (hint)
		infoOverlay.push_back(hint);

	    if (config.shouldUpdateRating)
	    {
		int old = (int)(gameState->rating);

		if (gameState->end == END_DEAD ||
			gameState->end == END_EXTRACTED)
		{
		    gameState->rating -= 0.1;
		    if (gameState->rating < 1)
			gameState->rating = 1.0;

		}
		else if (gameState->end == END_WIN)
		{
		    gameState->rating += 0.4;
		    if (gameState->rating < 5)
			// extra boost, so clawing your way back up from the
			// depths you sink to while first learning the game
			// isn't too arduous:
			gameState->rating += 0.2*(5 - (int)gameState->rating);
		}

		// deal with float inaccuracy - sometimes, 0.6+0.4 is just
		// less that 1.0...
		if ((gameState->rating - (int)(gameState->rating)) > 0.95)
		    gameState->rating = (int)(gameState->rating) + 1;

		if ((int)(gameState->rating) != old)
		{
		    ostringstream s;
		    s << "New rating: \"" <<
			ratingString( (int)(gameState->rating) ) << "\".";
		    infoOverlay.push_back(s.str());
		}
		if (config.shouldUpdateRating)
		{
		    // update ratings. Ratings are always kept ordered: lowest
		    // speed highest rating down to highest speed lowest
		    // rating.
		    bool newHigh = false;
		    for (int speed=0; speed<3; speed++)
			if ( (speed <= gameState->speed &&
				    config.rating[speed] < gameState->rating)
				|| (speed >= gameState->speed &&
				    config.rating[speed] > gameState->rating) )
			{
			    config.rating[speed] = gameState->rating;
			    if (config.highestRating[speed] < gameState->rating)
			    {
				newHigh = true;
				config.highestRating[speed] = gameState->rating;
			    }
			}
		    if (newHigh)
		    {
			infoOverlay.push_back("New high score!");
#ifdef HIGH_SCORE_REPORTING
			infoOverlay.push_back(settings.keybindings[C_REPORTHS].getString()
				+ string(" to report to server"));
#endif
		    }
		}
	    }
	}

	else if (gameState->end && gameState->ai)
	{
	    if (!ended)
	    {
		ended = true;
		AIEndTick = SDL_GetTicks();
	    }
	    else if (SDL_GetTicks() - AIEndTick > 5000)
	    {
		GameState* newGameState =
		    new GameState(settings.speed);
		delete gameState;
		gameState = newGameState;
		gameState->ai = new BasicAI(gameState);
		gameClock = GameClock(rateOfSpeed(settings.speed));
		drawBackground(screen);
		ended = false;
	    }
	}


	const int loopTime = SDL_GetTicks() - loopTicks;
	avFrameTime += (loopTime-avFrameTime)/avFrames;
	loopTicks = SDL_GetTicks();
    }

    if (!ended && gameState->extracted > 0)
    {
	// game is forfeit - reduce rating
	gameState->rating -= 0.1;
	if (gameState->rating < 1)
	    gameState->rating = 1.0;
    }

    config.write();

    delete gameState->ai;
    delete gameState;

    SDL_Quit();
}

#ifndef __APPLE__
int main(int argc, char** argv)
{
    load_settings(argc, argv);
    initialize_system();
    initialize_video();
    run_game();

    return 0;
}
#endif /* __APPLE__ */
