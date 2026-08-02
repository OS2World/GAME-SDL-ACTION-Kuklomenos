#ifndef INC_KEYBINDINGS_H
#define INC_KEYBINDINGS_H

#include <config.h>
#ifdef HAVE_LIBCURL
#define HIGH_SCORE_REPORTING 1
#endif

#include <string>
#include <vector>
#include <map>
using namespace std;

#include <SDL/SDL.h>

enum command
{
    C_NONE,

    C_LEFT,
    C_RIGHT,
    C_DEZOOM,
    C_DEAIM,

    C_SHOOT_GREEN,
    C_SHOOT_YELLOW,
    C_SHOOT_RED,
    C_SHOOT_POD,
    
    C_PAUSE,
    C_QUIT,

    C_MENU,
    C_M_LEFT,
    C_M_UP,
    C_M_RIGHT,
    C_M_DOWN,

    C_STARTGAME,

#ifdef SOUND
    C_SOUND,
#endif

    C_ZOOM,
    C_ROTATE,
    C_GRID,
    C_DECAA,
    C_INCAA,

    C_DECFPS,
    C_INCFPS,

    C_DECRATE,
    C_INCRATE,
    C_RESETRATE,

#ifdef HIGH_SCORE_REPORTING
    C_REPORTHS,
#endif

    C_SCREENSHOT,

    C_INVULN,
    C_WIN
};

extern const command C_FIRST;
extern const command C_LAST;
extern const command C_LASTACTION;
extern const command C_LASTDEBUG;

extern string longCommandNames[];
extern string shortCommandNames[];

bool isMenuCommand(const command& c);

command commandOfString(const string& str);

struct Key
{
    SDLKey sym;
    SDLMod mod;

    string getString() const;

    bool operator==(const Key& other) const;
    bool operator!=(const Key& other) const;

    bool isPressed();

    Key(SDL_keysym keysym);
    Key(string keyString);
    Key(SDLKey sym=SDLK_UNKNOWN, SDLMod mod=KMOD_NONE) : sym(sym), mod(mod) {}

    private:
	SDLMod simpleMod(const SDLMod& in) const;
};

extern Key KEY_NONE;

struct Keybindings : public map<command,Key>
{
    Key get(command command) const;
};

Keybindings& defaultKeybindings();

#endif /* INC_KEYBINDINGS_H */
