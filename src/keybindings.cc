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

#include <string>
#include <map>
using namespace std;

#include "keybindings.h"

const command C_FIRST = C_LEFT;
const command C_LAST = C_SCREENSHOT;
const command C_LASTACTION = C_STARTGAME;
const command C_LASTDEBUG = C_WIN;

string longCommandNames[] = {
    "BUG",
    "Turn left",
    "Turn right",
    "Dezoom",
    "De-aim",
    "Shoot green",
    "Shoot yellow",
    "Shoot red",
    "Shoot pod",
    "Pause",
    "Quit",
    "Menu",
    "Menu left",
    "Menu up",
    "Menu right",
    "Menu down",
    "Start game",
#ifdef SOUND
    "Toggle sound",
#endif
    "Toggle zoom",
    "Toggle rotate",
    "Toggle grid",
    "Decrease anti-aliasing",
    "Increase anti-aliasing",
    "Decrease fps",
    "Increase fps",
    "Decrease game rate",
    "Increase game rate",
    "Reset game rate",
#ifdef HIGH_SCORE_REPORTING
    "Report high score",
#endif
    "Take screenshot",
    "[DEBUG] Invulnerability",
    "[DEBUG] Win",
};
string shortCommandNames[] = {
    "BUG",
    "left",
    "right",
    "dezoom",
    "deaim",
    "shoot1",
    "shoot2",
    "shoot3",
    "shoot4",
    "pause",
    "quit",
    "menu",
    "menu_left",
    "menu_up",
    "menu_right",
    "menu_down",
    "start",
#ifdef SOUND
    "sound",
#endif
    "zoom",
    "rotate",
    "grid",
    "decaa",
    "incaa",
    "decfps",
    "incfps",
    "decrate",
    "incrate",
    "resetrate",
#ifdef HIGH_SCORE_REPORTING
    "reporths",
#endif
    "screenshot",
    "invuln",
    "win"
};

command commandOfString(const string& str)
{
    for (command c = C_FIRST; c <= C_LASTDEBUG; c = command(c+1))
	if (str == shortCommandNames[c])
	    return c;
    return C_NONE;
}

bool isMenuCommand(const command& c)
{
    return (c >= C_M_LEFT && c <= C_M_DOWN);
}

SDLMod Key::simpleMod(const SDLMod& in) const
{
    return SDLMod(((in & KMOD_SHIFT) ? KMOD_SHIFT : KMOD_NONE) |
	    ((in & KMOD_CTRL) ? KMOD_CTRL : KMOD_NONE) |
	    (((in & KMOD_ALT) | (in & KMOD_META)) ?
	     KMOD_ALT : KMOD_NONE));
}

Key::Key(SDL_keysym keysym)
{
    // care only about shift, ctrl, and alt/meta.
    sym = keysym.sym;
    mod = simpleMod(SDLMod(keysym.mod));
}

struct strcomp {
    bool operator() (const string& str1, const string& str2) const
    { return (str1.compare(str2) < 0); }
};

Key::Key(string keyString)
{
    static map<string,SDLKey,strcomp> keynameMap;
    if (keynameMap.size() == 0)
    {
	// initialise keynameMap; big long list of keys taken from
	// SDL_keyboard.c (SDL version 1.2.13).
	// Sadly we can't just use SDL_GetKeyName itself, as we want to be
	// able to run this before initialising video.
	keynameMap.insert( pair<string,SDLKey>("backspace",SDLK_BACKSPACE) );
	keynameMap.insert( pair<string,SDLKey>("tab",SDLK_TAB) );
	keynameMap.insert( pair<string,SDLKey>("clear",SDLK_CLEAR) );
	keynameMap.insert( pair<string,SDLKey>("return",SDLK_RETURN) );
	keynameMap.insert( pair<string,SDLKey>("pause",SDLK_PAUSE) );
	keynameMap.insert( pair<string,SDLKey>("escape",SDLK_ESCAPE) );
	keynameMap.insert( pair<string,SDLKey>("space",SDLK_SPACE) );
	keynameMap.insert( pair<string,SDLKey>("!",SDLK_EXCLAIM) );
	keynameMap.insert( pair<string,SDLKey>("\"",SDLK_QUOTEDBL) );
	keynameMap.insert( pair<string,SDLKey>("#",SDLK_HASH) );
	keynameMap.insert( pair<string,SDLKey>("$",SDLK_DOLLAR) );
	keynameMap.insert( pair<string,SDLKey>("&",SDLK_AMPERSAND) );
	keynameMap.insert( pair<string,SDLKey>("'",SDLK_QUOTE) );
	keynameMap.insert( pair<string,SDLKey>("(",SDLK_LEFTPAREN) );
	keynameMap.insert( pair<string,SDLKey>(")",SDLK_RIGHTPAREN) );
	keynameMap.insert( pair<string,SDLKey>("*",SDLK_ASTERISK) );
	keynameMap.insert( pair<string,SDLKey>("+",SDLK_PLUS) );
	keynameMap.insert( pair<string,SDLKey>(",",SDLK_COMMA) );
	keynameMap.insert( pair<string,SDLKey>("-",SDLK_MINUS) );
	keynameMap.insert( pair<string,SDLKey>(".",SDLK_PERIOD) );
	keynameMap.insert( pair<string,SDLKey>("/",SDLK_SLASH) );
	keynameMap.insert( pair<string,SDLKey>("0",SDLK_0) );
	keynameMap.insert( pair<string,SDLKey>("1",SDLK_1) );
	keynameMap.insert( pair<string,SDLKey>("2",SDLK_2) );
	keynameMap.insert( pair<string,SDLKey>("3",SDLK_3) );
	keynameMap.insert( pair<string,SDLKey>("4",SDLK_4) );
	keynameMap.insert( pair<string,SDLKey>("5",SDLK_5) );
	keynameMap.insert( pair<string,SDLKey>("6",SDLK_6) );
	keynameMap.insert( pair<string,SDLKey>("7",SDLK_7) );
	keynameMap.insert( pair<string,SDLKey>("8",SDLK_8) );
	keynameMap.insert( pair<string,SDLKey>("9",SDLK_9) );
	keynameMap.insert( pair<string,SDLKey>(":",SDLK_COLON) );
	keynameMap.insert( pair<string,SDLKey>(";",SDLK_SEMICOLON) );
	keynameMap.insert( pair<string,SDLKey>("<",SDLK_LESS) );
	keynameMap.insert( pair<string,SDLKey>("=",SDLK_EQUALS) );
	keynameMap.insert( pair<string,SDLKey>(">",SDLK_GREATER) );
	keynameMap.insert( pair<string,SDLKey>("?",SDLK_QUESTION) );
	keynameMap.insert( pair<string,SDLKey>("@",SDLK_AT) );
	keynameMap.insert( pair<string,SDLKey>("[",SDLK_LEFTBRACKET) );
	keynameMap.insert( pair<string,SDLKey>("\\",SDLK_BACKSLASH) );
	keynameMap.insert( pair<string,SDLKey>("]",SDLK_RIGHTBRACKET) );
	keynameMap.insert( pair<string,SDLKey>("^",SDLK_CARET) );
	keynameMap.insert( pair<string,SDLKey>("_",SDLK_UNDERSCORE) );
	keynameMap.insert( pair<string,SDLKey>("`",SDLK_BACKQUOTE) );
	keynameMap.insert( pair<string,SDLKey>("a",SDLK_a) );
	keynameMap.insert( pair<string,SDLKey>("b",SDLK_b) );
	keynameMap.insert( pair<string,SDLKey>("c",SDLK_c) );
	keynameMap.insert( pair<string,SDLKey>("d",SDLK_d) );
	keynameMap.insert( pair<string,SDLKey>("e",SDLK_e) );
	keynameMap.insert( pair<string,SDLKey>("f",SDLK_f) );
	keynameMap.insert( pair<string,SDLKey>("g",SDLK_g) );
	keynameMap.insert( pair<string,SDLKey>("h",SDLK_h) );
	keynameMap.insert( pair<string,SDLKey>("i",SDLK_i) );
	keynameMap.insert( pair<string,SDLKey>("j",SDLK_j) );
	keynameMap.insert( pair<string,SDLKey>("k",SDLK_k) );
	keynameMap.insert( pair<string,SDLKey>("l",SDLK_l) );
	keynameMap.insert( pair<string,SDLKey>("m",SDLK_m) );
	keynameMap.insert( pair<string,SDLKey>("n",SDLK_n) );
	keynameMap.insert( pair<string,SDLKey>("o",SDLK_o) );
	keynameMap.insert( pair<string,SDLKey>("p",SDLK_p) );
	keynameMap.insert( pair<string,SDLKey>("q",SDLK_q) );
	keynameMap.insert( pair<string,SDLKey>("r",SDLK_r) );
	keynameMap.insert( pair<string,SDLKey>("s",SDLK_s) );
	keynameMap.insert( pair<string,SDLKey>("t",SDLK_t) );
	keynameMap.insert( pair<string,SDLKey>("u",SDLK_u) );
	keynameMap.insert( pair<string,SDLKey>("v",SDLK_v) );
	keynameMap.insert( pair<string,SDLKey>("w",SDLK_w) );
	keynameMap.insert( pair<string,SDLKey>("x",SDLK_x) );
	keynameMap.insert( pair<string,SDLKey>("y",SDLK_y) );
	keynameMap.insert( pair<string,SDLKey>("z",SDLK_z) );
	keynameMap.insert( pair<string,SDLKey>("delete",SDLK_DELETE) );

	keynameMap.insert( pair<string,SDLKey>("world 0",SDLK_WORLD_0) );
	keynameMap.insert( pair<string,SDLKey>("world 1",SDLK_WORLD_1) );
	keynameMap.insert( pair<string,SDLKey>("world 2",SDLK_WORLD_2) );
	keynameMap.insert( pair<string,SDLKey>("world 3",SDLK_WORLD_3) );
	keynameMap.insert( pair<string,SDLKey>("world 4",SDLK_WORLD_4) );
	keynameMap.insert( pair<string,SDLKey>("world 5",SDLK_WORLD_5) );
	keynameMap.insert( pair<string,SDLKey>("world 6",SDLK_WORLD_6) );
	keynameMap.insert( pair<string,SDLKey>("world 7",SDLK_WORLD_7) );
	keynameMap.insert( pair<string,SDLKey>("world 8",SDLK_WORLD_8) );
	keynameMap.insert( pair<string,SDLKey>("world 9",SDLK_WORLD_9) );
	keynameMap.insert( pair<string,SDLKey>("world 10",SDLK_WORLD_10) );
	keynameMap.insert( pair<string,SDLKey>("world 11",SDLK_WORLD_11) );
	keynameMap.insert( pair<string,SDLKey>("world 12",SDLK_WORLD_12) );
	keynameMap.insert( pair<string,SDLKey>("world 13",SDLK_WORLD_13) );
	keynameMap.insert( pair<string,SDLKey>("world 14",SDLK_WORLD_14) );
	keynameMap.insert( pair<string,SDLKey>("world 15",SDLK_WORLD_15) );
	keynameMap.insert( pair<string,SDLKey>("world 16",SDLK_WORLD_16) );
	keynameMap.insert( pair<string,SDLKey>("world 17",SDLK_WORLD_17) );
	keynameMap.insert( pair<string,SDLKey>("world 18",SDLK_WORLD_18) );
	keynameMap.insert( pair<string,SDLKey>("world 19",SDLK_WORLD_19) );
	keynameMap.insert( pair<string,SDLKey>("world 20",SDLK_WORLD_20) );
	keynameMap.insert( pair<string,SDLKey>("world 21",SDLK_WORLD_21) );
	keynameMap.insert( pair<string,SDLKey>("world 22",SDLK_WORLD_22) );
	keynameMap.insert( pair<string,SDLKey>("world 23",SDLK_WORLD_23) );
	keynameMap.insert( pair<string,SDLKey>("world 24",SDLK_WORLD_24) );
	keynameMap.insert( pair<string,SDLKey>("world 25",SDLK_WORLD_25) );
	keynameMap.insert( pair<string,SDLKey>("world 26",SDLK_WORLD_26) );
	keynameMap.insert( pair<string,SDLKey>("world 27",SDLK_WORLD_27) );
	keynameMap.insert( pair<string,SDLKey>("world 28",SDLK_WORLD_28) );
	keynameMap.insert( pair<string,SDLKey>("world 29",SDLK_WORLD_29) );
	keynameMap.insert( pair<string,SDLKey>("world 30",SDLK_WORLD_30) );
	keynameMap.insert( pair<string,SDLKey>("world 31",SDLK_WORLD_31) );
	keynameMap.insert( pair<string,SDLKey>("world 32",SDLK_WORLD_32) );
	keynameMap.insert( pair<string,SDLKey>("world 33",SDLK_WORLD_33) );
	keynameMap.insert( pair<string,SDLKey>("world 34",SDLK_WORLD_34) );
	keynameMap.insert( pair<string,SDLKey>("world 35",SDLK_WORLD_35) );
	keynameMap.insert( pair<string,SDLKey>("world 36",SDLK_WORLD_36) );
	keynameMap.insert( pair<string,SDLKey>("world 37",SDLK_WORLD_37) );
	keynameMap.insert( pair<string,SDLKey>("world 38",SDLK_WORLD_38) );
	keynameMap.insert( pair<string,SDLKey>("world 39",SDLK_WORLD_39) );
	keynameMap.insert( pair<string,SDLKey>("world 40",SDLK_WORLD_40) );
	keynameMap.insert( pair<string,SDLKey>("world 41",SDLK_WORLD_41) );
	keynameMap.insert( pair<string,SDLKey>("world 42",SDLK_WORLD_42) );
	keynameMap.insert( pair<string,SDLKey>("world 43",SDLK_WORLD_43) );
	keynameMap.insert( pair<string,SDLKey>("world 44",SDLK_WORLD_44) );
	keynameMap.insert( pair<string,SDLKey>("world 45",SDLK_WORLD_45) );
	keynameMap.insert( pair<string,SDLKey>("world 46",SDLK_WORLD_46) );
	keynameMap.insert( pair<string,SDLKey>("world 47",SDLK_WORLD_47) );
	keynameMap.insert( pair<string,SDLKey>("world 48",SDLK_WORLD_48) );
	keynameMap.insert( pair<string,SDLKey>("world 49",SDLK_WORLD_49) );
	keynameMap.insert( pair<string,SDLKey>("world 50",SDLK_WORLD_50) );
	keynameMap.insert( pair<string,SDLKey>("world 51",SDLK_WORLD_51) );
	keynameMap.insert( pair<string,SDLKey>("world 52",SDLK_WORLD_52) );
	keynameMap.insert( pair<string,SDLKey>("world 53",SDLK_WORLD_53) );
	keynameMap.insert( pair<string,SDLKey>("world 54",SDLK_WORLD_54) );
	keynameMap.insert( pair<string,SDLKey>("world 55",SDLK_WORLD_55) );
	keynameMap.insert( pair<string,SDLKey>("world 56",SDLK_WORLD_56) );
	keynameMap.insert( pair<string,SDLKey>("world 57",SDLK_WORLD_57) );
	keynameMap.insert( pair<string,SDLKey>("world 58",SDLK_WORLD_58) );
	keynameMap.insert( pair<string,SDLKey>("world 59",SDLK_WORLD_59) );
	keynameMap.insert( pair<string,SDLKey>("world 60",SDLK_WORLD_60) );
	keynameMap.insert( pair<string,SDLKey>("world 61",SDLK_WORLD_61) );
	keynameMap.insert( pair<string,SDLKey>("world 62",SDLK_WORLD_62) );
	keynameMap.insert( pair<string,SDLKey>("world 63",SDLK_WORLD_63) );
	keynameMap.insert( pair<string,SDLKey>("world 64",SDLK_WORLD_64) );
	keynameMap.insert( pair<string,SDLKey>("world 65",SDLK_WORLD_65) );
	keynameMap.insert( pair<string,SDLKey>("world 66",SDLK_WORLD_66) );
	keynameMap.insert( pair<string,SDLKey>("world 67",SDLK_WORLD_67) );
	keynameMap.insert( pair<string,SDLKey>("world 68",SDLK_WORLD_68) );
	keynameMap.insert( pair<string,SDLKey>("world 69",SDLK_WORLD_69) );
	keynameMap.insert( pair<string,SDLKey>("world 70",SDLK_WORLD_70) );
	keynameMap.insert( pair<string,SDLKey>("world 71",SDLK_WORLD_71) );
	keynameMap.insert( pair<string,SDLKey>("world 72",SDLK_WORLD_72) );
	keynameMap.insert( pair<string,SDLKey>("world 73",SDLK_WORLD_73) );
	keynameMap.insert( pair<string,SDLKey>("world 74",SDLK_WORLD_74) );
	keynameMap.insert( pair<string,SDLKey>("world 75",SDLK_WORLD_75) );
	keynameMap.insert( pair<string,SDLKey>("world 76",SDLK_WORLD_76) );
	keynameMap.insert( pair<string,SDLKey>("world 77",SDLK_WORLD_77) );
	keynameMap.insert( pair<string,SDLKey>("world 78",SDLK_WORLD_78) );
	keynameMap.insert( pair<string,SDLKey>("world 79",SDLK_WORLD_79) );
	keynameMap.insert( pair<string,SDLKey>("world 80",SDLK_WORLD_80) );
	keynameMap.insert( pair<string,SDLKey>("world 81",SDLK_WORLD_81) );
	keynameMap.insert( pair<string,SDLKey>("world 82",SDLK_WORLD_82) );
	keynameMap.insert( pair<string,SDLKey>("world 83",SDLK_WORLD_83) );
	keynameMap.insert( pair<string,SDLKey>("world 84",SDLK_WORLD_84) );
	keynameMap.insert( pair<string,SDLKey>("world 85",SDLK_WORLD_85) );
	keynameMap.insert( pair<string,SDLKey>("world 86",SDLK_WORLD_86) );
	keynameMap.insert( pair<string,SDLKey>("world 87",SDLK_WORLD_87) );
	keynameMap.insert( pair<string,SDLKey>("world 88",SDLK_WORLD_88) );
	keynameMap.insert( pair<string,SDLKey>("world 89",SDLK_WORLD_89) );
	keynameMap.insert( pair<string,SDLKey>("world 90",SDLK_WORLD_90) );
	keynameMap.insert( pair<string,SDLKey>("world 91",SDLK_WORLD_91) );
	keynameMap.insert( pair<string,SDLKey>("world 92",SDLK_WORLD_92) );
	keynameMap.insert( pair<string,SDLKey>("world 93",SDLK_WORLD_93) );
	keynameMap.insert( pair<string,SDLKey>("world 94",SDLK_WORLD_94) );
	keynameMap.insert( pair<string,SDLKey>("world 95",SDLK_WORLD_95) );

	keynameMap.insert( pair<string,SDLKey>("[0]",SDLK_KP0) );
	keynameMap.insert( pair<string,SDLKey>("[1]",SDLK_KP1) );
	keynameMap.insert( pair<string,SDLKey>("[2]",SDLK_KP2) );
	keynameMap.insert( pair<string,SDLKey>("[3]",SDLK_KP3) );
	keynameMap.insert( pair<string,SDLKey>("[4]",SDLK_KP4) );
	keynameMap.insert( pair<string,SDLKey>("[5]",SDLK_KP5) );
	keynameMap.insert( pair<string,SDLKey>("[6]",SDLK_KP6) );
	keynameMap.insert( pair<string,SDLKey>("[7]",SDLK_KP7) );
	keynameMap.insert( pair<string,SDLKey>("[8]",SDLK_KP8) );
	keynameMap.insert( pair<string,SDLKey>("[9]",SDLK_KP9) );
	keynameMap.insert( pair<string,SDLKey>("[.]",SDLK_KP_PERIOD) );
	keynameMap.insert( pair<string,SDLKey>("[/]",SDLK_KP_DIVIDE) );
	keynameMap.insert( pair<string,SDLKey>("[*]",SDLK_KP_MULTIPLY) );
	keynameMap.insert( pair<string,SDLKey>("[-]",SDLK_KP_MINUS) );
	keynameMap.insert( pair<string,SDLKey>("[+]",SDLK_KP_PLUS) );
	keynameMap.insert( pair<string,SDLKey>("enter",SDLK_KP_ENTER) );
	keynameMap.insert( pair<string,SDLKey>("equals",SDLK_KP_EQUALS) );

	keynameMap.insert( pair<string,SDLKey>("up",SDLK_UP) );
	keynameMap.insert( pair<string,SDLKey>("down",SDLK_DOWN) );
	keynameMap.insert( pair<string,SDLKey>("right",SDLK_RIGHT) );
	keynameMap.insert( pair<string,SDLKey>("left",SDLK_LEFT) );
	keynameMap.insert( pair<string,SDLKey>("down",SDLK_DOWN) );
	keynameMap.insert( pair<string,SDLKey>("insert",SDLK_INSERT) );
	keynameMap.insert( pair<string,SDLKey>("home",SDLK_HOME) );
	keynameMap.insert( pair<string,SDLKey>("end",SDLK_END) );
	keynameMap.insert( pair<string,SDLKey>("page up",SDLK_PAGEUP) );
	keynameMap.insert( pair<string,SDLKey>("page down",SDLK_PAGEDOWN) );

	keynameMap.insert( pair<string,SDLKey>("f1",SDLK_F1) );
	keynameMap.insert( pair<string,SDLKey>("f2",SDLK_F2) );
	keynameMap.insert( pair<string,SDLKey>("f3",SDLK_F3) );
	keynameMap.insert( pair<string,SDLKey>("f4",SDLK_F4) );
	keynameMap.insert( pair<string,SDLKey>("f5",SDLK_F5) );
	keynameMap.insert( pair<string,SDLKey>("f6",SDLK_F6) );
	keynameMap.insert( pair<string,SDLKey>("f7",SDLK_F7) );
	keynameMap.insert( pair<string,SDLKey>("f8",SDLK_F8) );
	keynameMap.insert( pair<string,SDLKey>("f9",SDLK_F9) );
	keynameMap.insert( pair<string,SDLKey>("f10",SDLK_F10) );
	keynameMap.insert( pair<string,SDLKey>("f11",SDLK_F11) );
	keynameMap.insert( pair<string,SDLKey>("f12",SDLK_F12) );
	keynameMap.insert( pair<string,SDLKey>("f13",SDLK_F13) );
	keynameMap.insert( pair<string,SDLKey>("f14",SDLK_F14) );
	keynameMap.insert( pair<string,SDLKey>("f15",SDLK_F15) );

	keynameMap.insert( pair<string,SDLKey>("numlock",SDLK_NUMLOCK) );
	keynameMap.insert( pair<string,SDLKey>("caps lock",SDLK_CAPSLOCK) );
	keynameMap.insert( pair<string,SDLKey>("scroll lock",SDLK_SCROLLOCK) );
	keynameMap.insert( pair<string,SDLKey>("right shift",SDLK_RSHIFT) );
	keynameMap.insert( pair<string,SDLKey>("left shift",SDLK_LSHIFT) );
	keynameMap.insert( pair<string,SDLKey>("right ctrl",SDLK_RCTRL) );
	keynameMap.insert( pair<string,SDLKey>("left ctrl",SDLK_LCTRL) );
	keynameMap.insert( pair<string,SDLKey>("right alt",SDLK_RALT) );
	keynameMap.insert( pair<string,SDLKey>("left alt",SDLK_LALT) );
	keynameMap.insert( pair<string,SDLKey>("right meta",SDLK_RMETA) );
	keynameMap.insert( pair<string,SDLKey>("left meta",SDLK_LMETA) );
	keynameMap.insert( pair<string,SDLKey>("left super",SDLK_LSUPER) );
	keynameMap.insert( pair<string,SDLKey>("right super",SDLK_RSUPER) );	
	keynameMap.insert( pair<string,SDLKey>("alt gr",SDLK_MODE) );
	keynameMap.insert( pair<string,SDLKey>("compose",SDLK_COMPOSE) );

	keynameMap.insert( pair<string,SDLKey>("help",SDLK_HELP) );
	keynameMap.insert( pair<string,SDLKey>("print screen",SDLK_PRINT) );
	keynameMap.insert( pair<string,SDLKey>("sys req",SDLK_SYSREQ) );
	keynameMap.insert( pair<string,SDLKey>("break",SDLK_BREAK) );
	keynameMap.insert( pair<string,SDLKey>("menu",SDLK_MENU) );
	keynameMap.insert( pair<string,SDLKey>("power",SDLK_POWER) );
	keynameMap.insert( pair<string,SDLKey>("euro",SDLK_EURO) );
	keynameMap.insert( pair<string,SDLKey>("undo",SDLK_UNDO) );
    }

    mod = KMOD_NONE;
    size_t oldlen;
    do {
	oldlen = keyString.length();
	if (keyString.compare(0, 6, "shift+") == 0)
	{ keyString.erase(0, 6); mod = SDLMod(mod | KMOD_SHIFT); }
	if (keyString.compare(0, 5, "ctrl+") == 0)
	{ keyString.erase(0, 5); mod = SDLMod(mod | KMOD_CTRL); }
	if (keyString.compare(0, 4, "alt+") == 0)
	{ keyString.erase(0, 4); mod = SDLMod(mod | KMOD_ALT); }
    } while (keyString.length() < oldlen);

    map<string,SDLKey,strcomp>::iterator it = keynameMap.find(keyString);
    if ( it != keynameMap.end() )
	sym = it->second;
    else
	sym = SDLK_UNKNOWN;
}

string Key::getString() const
{
    if (sym == SDLK_UNKNOWN)
	return string("none");

    string s("");

    if (mod & KMOD_SHIFT)
	s += string("shift+");
    if (mod & KMOD_CTRL)
	s += string("ctrl+");
    if (mod & KMOD_ALT)
	s += string("alt+");
    s += string(SDL_GetKeyName(sym));

    return s;
}

bool Key::operator!=(Key const& other) const
{
    return (!((*this) == other));
}
bool Key::operator==(Key const& other) const
{
    return (sym == other.sym && mod == other.mod);
}

bool Key::isPressed()
{
    return (SDL_GetKeyboardState(NULL)[SDL_GetScancodeFromKey(sym)] &&
            simpleMod(SDL_GetModState()) == mod);
}

Key KEY_NONE;

// get(command): returns Key bound to command if in the map, else the null Key
Key Keybindings::get(command com) const
{
    map<command,Key>::const_iterator it = find(com);
    if (it == end())
	return Key();
    else
	return it->second;
}

// XXX: see http://www.parashift.com/c++-faq-lite/ctors.html#faq-10.12 if
// you're wondering why we don't just make defaultKeybindings an extern
Keybindings& defaultKeybindings()
{
    static Keybindings defaultKeybindings;
    if (defaultKeybindings.empty())
    {
	//initialise
	defaultKeybindings[C_LEFT] = Key(SDLK_LEFT);
	defaultKeybindings[C_RIGHT] = Key(SDLK_RIGHT);
	defaultKeybindings[C_DEZOOM] = Key(SDLK_UP);
	defaultKeybindings[C_DEAIM] = Key(SDLK_DOWN);
	defaultKeybindings[C_SHOOT_GREEN] = Key(SDLK_1);
	defaultKeybindings[C_SHOOT_YELLOW] = Key(SDLK_2);
	defaultKeybindings[C_SHOOT_RED] = Key(SDLK_3);
	defaultKeybindings[C_SHOOT_POD] = Key(SDLK_4);

	defaultKeybindings[C_PAUSE] = Key(SDLK_p);
	defaultKeybindings[C_QUIT] = Key(SDLK_q);

	defaultKeybindings[C_MENU] = Key(SDLK_ESCAPE);
	defaultKeybindings[C_M_LEFT] = Key(SDLK_LEFT);
	defaultKeybindings[C_M_UP] = Key(SDLK_UP);
	defaultKeybindings[C_M_RIGHT] = Key(SDLK_RIGHT);
	defaultKeybindings[C_M_DOWN] = Key(SDLK_DOWN);

	defaultKeybindings[C_STARTGAME] = Key(SDLK_SPACE);

#ifdef SOUND
	defaultKeybindings[C_SOUND] = Key(SDLK_s);
#endif

	defaultKeybindings[C_ZOOM] = Key(SDLK_z);
	defaultKeybindings[C_ROTATE] = Key(SDLK_r);
	defaultKeybindings[C_GRID] = Key(SDLK_g);
	defaultKeybindings[C_DECAA] = Key(SDLK_a, SDLMod(KMOD_SHIFT));
	defaultKeybindings[C_INCAA] = Key(SDLK_a);

	defaultKeybindings[C_DECFPS] = Key(SDLK_LEFTBRACKET);
	defaultKeybindings[C_INCFPS] = Key(SDLK_RIGHTBRACKET);

	defaultKeybindings[C_DECRATE] = Key(SDLK_MINUS);
	defaultKeybindings[C_INCRATE] = Key(SDLK_EQUALS, SDLMod(KMOD_SHIFT));
	defaultKeybindings[C_RESETRATE] = Key(SDLK_EQUALS);

#ifdef HIGH_SCORE_REPORTING
	defaultKeybindings[C_REPORTHS] = Key(SDLK_F10);
#endif

	defaultKeybindings[C_SCREENSHOT] = Key(SDLK_F12);

	defaultKeybindings[C_INVULN] = Key(SDLK_F4);
	defaultKeybindings[C_WIN] = Key(SDLK_F5);
    }

    return defaultKeybindings;
}

