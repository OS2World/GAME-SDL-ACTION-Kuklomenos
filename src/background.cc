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

#include "background.h"
#include "random.h"
#include "coords.h"
#include "settings.h"

#include <vector>
using namespace std;

#include <SDL/SDL.h>
#include <SDL_gfxPrimitivesDirty.h>

SDL_Surface* background = NULL;

void setBackground(SDL_Surface* screen)
{
    if (background)
	SDL_FreeSurface(background);
    if (settings.bgType == BG_NONE)
	background = NULL;
    else
	background = SDL_CreateRGBSurface(SDL_SWSURFACE, screen->w, screen->h,
		screen->format->BitsPerPixel, 0,0,0,0);
    drawBackground(screen);
}

Uint32 randomStarColour(bool interesting=false);

void drawBackground(SDL_Surface* screen)
{
    if (!background)
    {
	// background==NULL: use black background
	SDL_FillRect(screen, NULL, 0);
	return;
    }

    SDL_FillRect(background, NULL, 0);

    if (settings.bgType == BG_SOLAR)
    {
	// Position a star offscreen (at least a screen's diagonal away from
	// any onscreen point)
	const double theta = ranf(2*PI);
	const double dist = (ranf(2)+3)*sqrt(background->w*background->w +
		background->h*background->h)/2;
	const double cx = background->w/2 + dist*cos(theta);
	const double cy = background->h/2 + dist*sin(theta);

	const Uint32 colour = randomStarColour(true);
	const int starColour[3] = {
	    (colour & 0xff000000) >> 24,
	    (colour & 0x00ff0000) >> 16,
	    (colour & 0x0000ff00) >> 8 };

	const Uint8 brightness = 0x70 + rani(0x20);
	const double closestSqDist = background->w*background->w +
	    background->h*background->h;

	// we want gaussian dither, but don't want to calculate a fresh
	// gaussian for each pixel. So we sample, then use random samples for
	// the dither.
	const int bpp = background->format->BitsPerPixel;
	static const int gaussianSampleSize = 200;
	int gaussianSample[gaussianSampleSize];
	for (int i=0; i < gaussianSampleSize; i++)
	    gaussianSample[i] = int(gaussian() * (
			bpp == 32 ? 0x02 :
			bpp == 24 ? 0x03 :
			0x04));

	for (int x=0; x < background->w; x++)
	    for (int y=0; y < background->h; y++)
	    {
		// inverse square law for the intensity, with a random
		// dithering effect to reduce ugly banding
		const double sqDist = ((x-cx)*(x-cx) + (y-cy)*(y-cy));
		const float intensity = closestSqDist / sqDist;

		int c[3] = {
		    int(starColour[0] * intensity) +
			gaussianSample[rani(gaussianSampleSize)],
		    int(starColour[1] * intensity) +
			gaussianSample[rani(gaussianSampleSize)],
		    int(starColour[2] * intensity) +
			gaussianSample[rani(gaussianSampleSize)] };

		for (int i=0; i<3; i++)
		{
		    if (c[i] > 0xff)
			c[i] = 0xff;
		    if (c[i] < 0)
			c[i] = 0;
		}
		pixelRGBA(background, x, y, c[0], c[1], c[2], brightness);
	    }
    }

    if (settings.bgType == BG_STARS || settings.bgType == BG_SOLAR)
	for (int i=0;
		i < (background->w * background->h / (400 + rani(800)));
		i++)
	{
	    pixelColor(background, rani(background->w), rani(background->h),
		    randomStarColour() + 0x30 + rani(0x90));
	}

    SDL_BlitSurface(background, NULL, screen, NULL);
}

Uint32 addColour(Uint32 base, int dr=0, int dg=0, int db=0, int da=0)
{
    Uint8 c[4] = { base >> 24, base >> 16 & 0xff, base >> 8 & 0xff, base & 0xff };
    int dc[4] = { dr, dg, db, da };
    for (int i = 0; i < 4; i++)
	c[i] = (
		0xff - c[i] < dc[i] ? 0xff :
		c[i] < -dc[i] ? 0 :
		c[i] + dc[i]);
    return (c[0] << 24) + (c[1] << 16) + (c[2] << 8) + c[3];
}

Uint32 randomStarColour(bool interesting)
{
    // classColours based on data due to Mitchell Charity
    // (http://www.vendian.org/mncharity/dir3/starcolor/)
    // (multiplied through by 15/16 to leave room for dithering)
    static const Uint32 classColours[] = { 0x91a5ef00, 0x9fb3ef00, 0xbdc9ef00,
	0xe7e7ef00, 0xefe4db00, 0xefc49600, 0xefbf6800 };

    int starClass;
    const float r = ranf();
    if (!interesting)
	// roughly accurate frequency data based on that given in
	// http://en.wikipedia.org/w/index.php?title=Stellar_classification&oldid=316377760
	// (not followed exactly)
	starClass =
	    r < 0.001 ? 0 : // Class O
	    r < 0.003 ? 1 : // Class B
	    r < 0.010 ? 2 : // Class A
	    r < 0.040 ? 3 : // Class F
	    r < 0.1 ? 4 :   // Class G
	    r < 0.25 ? 5 :  // Class K
	    6;              // Class M
    else
	// let's magnify the chances of unlikely classes
	starClass =
	    r < 0.02 ? 0 :  // Class O
	    r < 0.05 ? 1 :  // Class B
	    r < 0.1 ? 2 :   // Class A
	    r < 0.2 ? 3 :   // Class F
	    r < 0.3 ? 4 :   // Class G
	    r < 0.5 ? 5 :   // Class K
	    6;              // Class M

    const Uint32 baseColour = classColours[starClass];

    // Randomly tweak the colours a little
    return addColour(baseColour,
	    int(gaussian()*0x0b), int(gaussian()*0x0b), int(gaussian()*0x0b));
}

