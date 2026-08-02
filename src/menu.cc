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

#include <vector>
#include <string>
#include <sstream>
using namespace std;

#include "menu.h"
#include "settings.h"
#include "data.h"
#include "geom.h"
#include "keybindings.h"
#include "SDL_gfxPrimitivesDirty.h"

#include <config.h>

Menu::Menu(string title,
	Menu* m1, MenuLeaf* ml1,
	Menu* m2, MenuLeaf* ml2,
	Menu* m3, MenuLeaf* ml3,
	Menu* m4, MenuLeaf* ml4) :
    title(title)
{
    menus[0] = m1;
    menus[1] = m2;
    menus[2] = m3;
    menus[3] = m4;

    leaves[0] = ml1;
    leaves[1] = ml2;
    leaves[2] = ml3;
    leaves[3] = ml4;
}

string Menu::textOfDir(int dir) const
{
    Menu* submenu = menus[dir];
    MenuLeaf* leaf = leaves[dir];

    if (submenu)
	return submenu->title;
    else if (leaf)
	return leaf->name();
    else
	return "";
}

string MenuLeafToggleBool::name()
{
    return varName +
	((*var) ?
	 ": on" :
	 ": off");
}

LeafReturn MenuLeafToggleBool::act()
{
    *var = !*var;
    return LR_NONE;
}

string MenuLeafCycleAA::name()
{
    switch (settings.useAA)
    {
	case AA_NO: return "Anti-alias: none";
	case AA_YES: return "Anti-alias: some";
	case AA_FORCE: default: return "Anti-alias: all";
    }
}

LeafReturn MenuLeafCycleAA::act()
{
    settings.useAA =
	(settings.useAA == AA_NO) ? AA_YES :
	(settings.useAA == AA_YES) ? AA_FORCE :
	AA_NO;
    return LR_NONE;
}

MenuLeafToggleBool zoomToggle("Zoom", &settings.zoomEnabled);
MenuLeafToggleBool rotToggle("Rotate", &settings.rotatingView);
MenuLeafToggleBool gridToggle("Grid", &settings.showGrid);
Menu viewMenu("View",
	NULL, &zoomToggle,
	NULL, &rotToggle,
	NULL, &gridToggle
	);

#ifdef SOUND
string MenuLeafCycleFreq::name()
{
    ostringstream s;
    s << "Freq: " << settings.soundFreq;
    return s.str();
}
LeafReturn MenuLeafCycleFreq::act()
{
    settings.soundFreq =
	(settings.soundFreq >= 44100) ? 11025 :
	(settings.soundFreq >= 22050) ? 44100 :
	(settings.soundFreq >= 11025) ? 22050 :
	11025;
    return LR_NONE;
}
MenuLeafCycleFreq cycleFreq;

MenuLeafToggleBool soundToggle("Sound", &settings.sound);
MenuLeafIncVar<float> decVol(&settings.volume,
	"Volume", -0.1, 0.1, 1);
MenuLeafIncVar<float> incVol(&settings.volume,
	"Volume", 0.1, 0.1, 1);
MenuLeafShowVar<float> showVol(&settings.volume,
	"Volume");
Menu volumeMenu("Volume",
	NULL, &decVol,
	NULL, NULL,
	NULL, &incVol,
	NULL, &showVol
	);
Menu soundMenu("Sound",
	NULL, &cycleFreq,
	NULL, NULL,
	NULL, &soundToggle,
	&volumeMenu, NULL
	);
#endif

MenuLeafCycleAA cycleAALeaf;
MenuLeafIncVar<double> decAlphaLeaf(&antialiasGamma, "AA Gamma", -0.1, 0.1);
MenuLeafIncVar<double> incAlphaLeaf(&antialiasGamma, "AA Gamma", 0.1, 0.1);
MenuLeafShowVar<double> showAlphaLeaf(&antialiasGamma, "AA Gamma");
Menu AAMenu("Anti-Aliasing",
	NULL, &decAlphaLeaf,
	NULL, &cycleAALeaf,
	NULL, &incAlphaLeaf,
	NULL, &showAlphaLeaf);

MenuLeafToggleBool showFPSToggle("Show FPS", &settings.showFPS);
MenuLeafIncVar<int> decFPSLeaf(&settings.fps, "FPS", -1, 2);
MenuLeafIncVar<int> incFPSLeaf(&settings.fps, "FPS", 1, 2);
MenuLeafShowVar<int> showFPSLeaf(&settings.fps, "FPS");
Menu fpsMenu("Framerate",
	NULL, &decFPSLeaf,
	NULL, &showFPSToggle,
	NULL, &incFPSLeaf,
	NULL, &showFPSLeaf
	);

LeafReturn MenuLeafCycleBpp::act()
{
    settings.bpp += 8;
    if (settings.bpp > 32)
	settings.bpp = 8;
    return LR_NONE;
}
MenuLeafCycleBpp cycleBppLeaf;
string MenuLeafSetRes::name()
{
    std::stringstream ss;
    ss << "Set " << settings.width << "x" << settings.height << "x" << settings.bpp <<
	((settings.videoFlags & SDL_FULLSCREEN) ? " fs" : "");
    return ss.str();
}
MenuLeafSetRes setResLeaf;

LeafReturn MenuLeafToggleFullscreen::act()
{
    if (settings.videoFlags & SDL_FULLSCREEN)
	settings.videoFlags &= ~SDL_FULLSCREEN;
    else
	settings.videoFlags |= SDL_FULLSCREEN;
    return LR_NONE;
}
MenuLeafToggleFullscreen toggleFullscreen;

LeafReturn MenuLeafCycleRes::act()
{
    std::vector<Rect> modes = getSDLModes();
    if (modes.empty())
	return LR_NONE;

    // set to next biggest mode if exists, else smallest mode
    Rect currentMode(settings.width, settings.height);

    Rect nextBiggestMode;
    Rect smallestMode;
    for (std::vector<Rect>::iterator it = modes.begin();
	    it != modes.end(); it++)
    {
	if (Rect::cmp(*it, currentMode) == 1 &&
		(nextBiggestMode.w == 0 ||
		 Rect::cmp(*it, nextBiggestMode) == -1))
	    nextBiggestMode = *it;
	if (smallestMode.w == 0 || Rect::cmp(*it, smallestMode) == -1)
	    smallestMode = *it;
    }

    const Rect newMode = nextBiggestMode.w ? nextBiggestMode : smallestMode;
    if (newMode.w == 0)
	return LR_NONE;

    settings.width = newMode.w;
    settings.height = newMode.h;
    return LR_NONE;
}
MenuLeafCycleRes cycleRes;

string MenuLeafCycleBGType::name()
{
    return string("Background: " + bgTypeStrings[settings.bgType]);
}
LeafReturn MenuLeafCycleBGType::act()
{
    if (settings.bgType == BG_LAST)
	settings.bgType = BG_FIRST;
    else
	settings.bgType = BGType(settings.bgType + 1);
    return LR_NEWBACKGROUND;
}
MenuLeafCycleBGType cycleBGType;

Menu resolutionMenu("Display",
	NULL, &cycleRes,
	NULL, &toggleFullscreen,
	NULL, &cycleBppLeaf,
	NULL, &setResLeaf
	);

Menu graphicsMenu("Graphics",
	&resolutionMenu, NULL,
	NULL, &cycleBGType,
	&fpsMenu, NULL,
	&AAMenu, NULL
	);

string MenuLeafSpeed::name()
{
    return (string("Speed: ") +
	    speedString(settings.speed));
}
LeafReturn MenuLeafSpeed::act()
{
    settings.speed = (settings.speed + 1) % 3;
    return LR_NEWSPEED;
}
MenuLeafSpeed speed;

MenuLeafIncVar<float> decTurnRateLeaf(&settings.turnRateFactor,
	"Turn Rate", -0.1, 0.1, 1);
MenuLeafIncVar<float> incTurnRateLeaf(&settings.turnRateFactor,
	"Turn Rate", 0.1, 0.1, 1);
MenuLeafShowVar<float> showTurnRateLeaf(&settings.turnRateFactor,
	"Turn Rate");
Menu turnRateMenu("Turn rate",
	NULL, &decTurnRateLeaf,
	NULL, NULL,
	NULL, &incTurnRateLeaf,
	NULL, &showTurnRateLeaf
	);

string MenuLeafSetKey::name()
{
    if (settings.commandToBind == C_NONE)
	return string(longCommandNames[selectedKey]) + string(": ") +
	    settings.keybindings[selectedKey].getString();
    else
	return string(longCommandNames[selectedKey]) + string(": <press a key>");
}
LeafReturn MenuLeafSetKey::act()
{
    settings.commandToBind = selectedKey;
    return LR_NONE;
}
LeafReturn MenuLeafPrevKey::act()
{
    if (*commandp == C_FIRST)
	*commandp = settings.debug ? C_LASTDEBUG : C_LAST;
    else
	*commandp = command(*commandp - 1);
    return LR_NONE;
}
LeafReturn MenuLeafNextKey::act()
{
    if (*commandp == (settings.debug ? C_LASTDEBUG : C_LAST))
	*commandp = C_FIRST;
    else
	*commandp = command(*commandp + 1);
    return LR_NONE;
}

command MenuLeafKeyConflicts::getConflictingCommand()
{
    const bool menu = isMenuCommand(*commandp);
    const Key k = settings.keybindings.get(*commandp);

    if (k == KEY_NONE)
	return C_NONE;

    for (command c = C_FIRST; c <= C_LAST; c = command(c+1))
	if (settings.keybindings.get(c) == k && c != *commandp
		&& isMenuCommand(c) == menu)
	    return c;
    return C_NONE;
}
string MenuLeafKeyConflicts::name()
{
    const command c = getConflictingCommand();
    if (c == C_NONE)
	return string("");
    else
	return string("Conflicts with ") + longCommandNames[c];
}
LeafReturn MenuLeafKeyConflicts::act()
{
    const command c = getConflictingCommand();
    if (c != C_NONE)
	*commandp = c;
    return LR_NONE;
}

MenuLeafSetKey setKey;
MenuLeafPrevKey prevKey(&setKey.selectedKey);
MenuLeafNextKey nextKey(&setKey.selectedKey);
MenuLeafKeyConflicts keyConflicts(&setKey.selectedKey);

Menu keybindingsMenu("Keybindings",
	NULL, &prevKey,
	NULL, &keyConflicts,
	NULL, &nextKey,
	NULL, &setKey
	);

Menu controlMenu("Control",
	NULL, NULL,
	&turnRateMenu, NULL,
	NULL, NULL,
	&keybindingsMenu, NULL
	);

Menu interfaceMenu("Interface",
	&graphicsMenu, NULL,
	NULL, NULL,
	&controlMenu, NULL,
#ifdef SOUND
	&soundMenu, NULL
#else
	NULL, NULL
#endif
	);

MenuLeafReturn saveLeaf("Save settings", LR_SAVESETTINGS);
Menu settingsMenu("Settings",
	&interfaceMenu, NULL,
	NULL, &saveLeaf,
	&viewMenu, NULL,
	NULL, &speed
	);

MenuLeafReturn quitLeaf("Quit", LR_QUIT);
MenuLeafReturn surrenderLeaf("Surrender", LR_SURRENDER);
Menu quitMenu("Abort",
	NULL, &quitLeaf,
	NULL, NULL,
	NULL, &surrenderLeaf
	);

MenuLeafReturn resumeLeaf("Resume", LR_EXITMENU);

Menu topMenu("top",
	NULL, NULL,
	&settingsMenu, NULL,
	NULL, NULL,
	&quitMenu, NULL
	);

void drawMenu(SDL_Surface* surface, const Menu& menu)
{
    static const Uint32 colour = 0xffffffd0;
    int fw = 10, fh = 20;
    int menuRad = std::max(fh, screenGeom.rad/8);
    const int textRoom = std::max(std::max(std::max(
	    menu.textOfDir(0).length()*fw,
	    menu.textOfDir(1).length()*fw/2),
	    menu.textOfDir(2).length()*fw),
	    menu.textOfDir(3).length()*fw/2);

    // check we have room:
    if (screenGeom.centre.x - menuRad - textRoom > screenGeom.rad/5)
	gfxPrimitivesSetFont(fontBig, 10, 20);
    else
    {
	// use smaller font
	fw = 7;
	fh = 13;
	menuRad = std::max(fh, screenGeom.rad/10);
	gfxPrimitivesSetFont(fontSmall, 7, 13);
    }

    filledCircleColor(surface, screenGeom.centre.x, screenGeom.centre.y+fh/2, screenGeom.rad/30,
	    colour - 0x20);
    aacircleColor(surface, screenGeom.centre.x, screenGeom.centre.y+fh/2, screenGeom.rad/30,
	    colour - 0x20);

    const int textSizeHoriz = std::max(
	    menu.textOfDir(0).length()*fw,
	    menu.textOfDir(2).length()*fw);
    for (int dir = 0; dir < 4; dir++)
    {
	const string text = menu.textOfDir(dir);
	if (!text.empty())
	{
	    int x=0, y=0;
	    switch (dir)
	    {
		case 0:
		    x = screenGeom.centre.x - menuRad - textSizeHoriz/2 - text.length()*fw/2;
		    y = screenGeom.centre.y;
		    break;
		case 1:
		    x = screenGeom.centre.x - text.length()*fw/2;
		    y = screenGeom.centre.y - menuRad;
		    break;
		case 2:
		    x = screenGeom.centre.x + menuRad + textSizeHoriz/2 - text.length()*fw/2;
		    y = screenGeom.centre.y;
		    break;
		case 3:
		    x = screenGeom.centre.x - text.length()*fw/2;
		    y = screenGeom.centre.y + menuRad;
		    break;
		default:;
	    }
	    stringColor(surface, x, y, text.c_str(), colour);
	}
    }
}
