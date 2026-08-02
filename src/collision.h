#ifndef INC_COLLISION_H
#define INC_COLLISION_H

#include "coords.h"

// pointHitsCircle: given a point starting at (x,y) moving with velocity
// (vx,vy), determines when the point first is in the circle at the origin
// with radius rad. Returns -1 for "never". If et is non-negative, also
// returns -1 for "after et".
float pointHitsCircle(float x, float y, float vx, float vy, float rad, float et=-1);

// pointHitsPolygon: as for pointHitsCircleWithin. 'points' must march
// anticlockwise around the polygon.
float pointHitsPolygon(CartCoord* points, int n, CartCoord p, RelCartCoord v, float et=-1);

struct CollisionCircle;
struct CollisionPolygon;

// CollisionObject: Virtual representing an object moving with constant velocity.
struct CollisionObject
{
    CartCoord startPos;
    RelCartCoord velocity;
    virtual float pointHits(CartCoord p, RelCartCoord v, float et=-1) const
	=0;
    float pointHits(float x, float y, float vx, float vy, float et) const;
    bool pointIn(float x, float y) const;
    bool pointIn(CartCoord p) const;

    virtual bool circleIntersects(CartCoord c, float rad) const =0;

    bool objectCollides(const CollisionCircle& other) const;
    bool objectCollides(const CollisionPolygon& other) const;

    CollisionObject(CartCoord startPos, RelCartCoord velocity)
	: startPos(startPos), velocity(velocity) {};
    CollisionObject() {}
    virtual ~CollisionObject() {}
};

struct CollisionCircle : public CollisionObject
{
    float radius;

    CollisionCircle(CartCoord istartPos, RelCartCoord ivelocity, float iradius=1)
	: CollisionObject(startPos, velocity), radius(iradius)
    {}

    bool circleIntersects(CartCoord c, float rad) const;

    float pointHits(CartCoord p, RelCartCoord v, float et=-1) const;
};

struct CollisionPolygon : public CollisionObject
{
    int numPoints;
    RelCartCoord* points;
    float angle;

    CollisionPolygon(int inumPoints, RelCartCoord* ipoints, float iangle=0) :
	numPoints(inumPoints), points(ipoints), angle(iangle) {}

    bool circleIntersects(CartCoord c, float rad) const;

    float pointHits(CartCoord p, RelCartCoord v, float et=-1) const;
};
#endif /* INC_COLLISION_H */
