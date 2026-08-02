#ifndef INC_SETTINGS_H
#define INC_SETTINGS_H

#include <vector>
#include <string>
using namespace std;

#include "keybindings.h"

#include <SDL/SDL.h>

enum UseAALevel
{
    AA_NO,
    AA_YES,
    AA_FORCE
};

enum BGType
{
    BG_FIRST = 0,
    BG_NONE = 0,
    BG_STARS = 1,
    BG_SOLAR = 2,
    BG_LAST = 2
};

extern string bgTypeStrings[];

struct Settings
{
    bool debug;
    bool wizard;
    bool invuln;
    UseAALevel useAA;
    bool showGrid;
    bool zoomEnabled;
    bool rotatingView;
    float turnRateFactor;
    float requestedRating;
    int speed;
    bool stopMotion;

    Keybindings keybindings;
    command commandToBind;

    BGType bgType;

    int fps;
    bool showFPS;

    int width;
    int height;
    int bpp;
    bool fullscreen;
    Uint32 videoFlags;

    bool sound;
    float volume;
    int soundFreq;

    int clockRate;

    Settings();
};

int rateOfSpeed(int speed);
const char* speedString(int speed);
const char* speedStringShort(int speed);

extern Settings settings;

// Rect: convenience wrapper around SDL_Rect
struct Rect
{
    int w;
    int h;

    static int cmp(const Rect& r1, const Rect& r2);

    Rect(int w=0, int h=0) : w(w), h(h) {}
    Rect(SDL_Rect r) : w(r.w), h(r.h) {}
};

// getSDLModes: convenience wrapper around SDL_ListModes
vector<Rect> getSDLModes();

#ifndef __APPLE__
void load_settings(int argc, char** argv);
#endif

#endif /* INC_SETTINGS_H */
