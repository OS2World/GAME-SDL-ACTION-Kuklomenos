#ifndef INC_OVERLAY_H
#define INC_OVERLAY_H

#include <string>
#include <vector>
#include <SDL/SDL.h>
using namespace std;

class Overlay : public vector<string>
{
    private:
	bool drawWithFont(SDL_Surface* surface, const void* fontdata, int cw,
		int ch, Uint8 alpha=0, bool force=false);
    public:
	float offy; // y offset from centre, in screen radii
	Uint32 colour;

	// drawstr: convenience function - make 'str' the only entry, then
	// draw, then clear again
	void drawstr(SDL_Surface* surface, string str, Uint8 alpha=0);

	void draw(SDL_Surface* surface, Uint8 alpha=0);
	Overlay(float offy=0, Uint32 colour=0xffffffff) : offy(offy),
	    colour(colour) {}
};

#endif /* INC_OVERLAY_H */
