#ifndef INC_BACKGROUND_H
#define INC_BACKGROUND_H

#include <SDL/SDL.h>

extern SDL_Surface* background;

// setBackground: allocate and draw background
void setBackground(SDL_Surface* screen);

// drawBackground: draw new background, keeping old screen format
void drawBackground(SDL_Surface* screen);

#endif /* INC_BACKGROUND_H */
