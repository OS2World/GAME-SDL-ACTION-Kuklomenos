#ifndef INC_GEOM_H
#define INC_GEOM_H

#include "coords.h"

struct ScreenGeom
{
    int width;
    int height;

    ScreenCoord centre;
    int rad;
    ScreenCoord info;
    int infoMaxLength;
    int infoMaxLines;
    int indicatorRsqLim1;
    int indicatorRsqLim2;
    int indicatorRsqLim3;
    int indicatorRsqLim4;

    ScreenGeom() {};
    ScreenGeom(int iwidth, int iheight);
};

extern ScreenGeom screenGeom;

extern int ARENA_RAD;

extern float AIM_MIN;
extern float ZOOM_MIN;
extern float AIM_MAX;

extern float ZOOMDIST_MAX;

extern CartCoord ARENA_CENTRE;

#endif /* INC_GEOM_H */
