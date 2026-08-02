#ifndef INC_GFX_H
#define INC_GFX_H

#include <SDL/SDL.h>

#include "coords.h"

struct View
{
    CartCoord centre;
    float zoom;
    float angle;

    View() {}
    View(CartCoord icentre, float izoom=1, float iangle=0);

    ScreenCoord coord(const CartCoord &c) const;
    bool inView(const CartCoord &c, float in=0) const;
};

struct Line
{
    CartCoord start;
    CartCoord end;
    Uint32 colour;

    Line(CartCoord istart, CartCoord iend, Uint32 icolour=0xffffffff) :
	start(istart),end(iend),colour(icolour) {}

    int draw(SDL_Surface* surface, const View& view, View* boundView=NULL,
	bool noAA=false);
};

struct Circle
{
    CartCoord centre;
    float r;
    Uint32 colour;
    bool filled;

    Circle(CartCoord icentre, float ir, Uint32 icolour=0xffffffff,
	    bool ifilled=false) :
	centre(icentre), r(ir), colour(icolour), filled(ifilled)
    {}

    int draw(SDL_Surface* surface, const View& view, View* boundView=NULL,
	bool noAA=false);
};

struct Polygon
{
    CartCoord* points;
    int n;
    Uint32 colour;
    bool filled;

    Polygon(CartCoord* ipoints, int in, Uint32 icolour=0xffffffff,
	    bool ifilled=false) :
	points(ipoints), n(in), colour(icolour), filled(ifilled)
    {}

    int draw(SDL_Surface* surface, const View& view, View* boundView=NULL,
	bool noAA=false);
};

struct Pixel
{
    CartCoord point;
    Uint32 colour;
    Pixel(CartCoord point, Uint32 colour=0xffffffff) :
	point(point), colour(colour)
    {}

    int draw(SDL_Surface* surface, const View& view, View* boundView=NULL,
	bool noAA=false);
};

#endif /* INC_GFX_H */
