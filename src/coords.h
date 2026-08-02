#ifndef INC_COORDS_H
#define INC_COORDS_H

#include <cmath>

#define PI 3.14159265

struct RelCartCoord;

// vertically upwards is 0, anticlockwise is positive; units are PI/2 radians.
struct Angle
{
    float angle;

    Angle(float iangle=0) :
	angle(iangle)
    {
	normalise();
    }

    operator float() const { return angle; }

    Angle operator += (const Angle& a)
    {
	angle += a.angle;
	normalise();
	return *this;
    }
    Angle operator -= (const Angle& a)
    {
	angle -= a.angle;
	normalise();
	return *this;
    }
    Angle operator *= (float m)
    {
	angle *= m;
	normalise();
	return *this;
    }
    void normalise()
    {
	angle = fmod(angle,4);
	while (angle < 0)
	    angle += 4;
    }

    float sinf() const
    {
	// XXX: always use a.sinf() rather than sinf(a)!
	return ::sinf(angle*PI/2);
    }
};

struct RelPolarCoord
{
    Angle angle;
    float dist;

    RelPolarCoord(float iangle=0, float idist=0) :
	angle(iangle),dist(idist)
    {}

    RelPolarCoord(const RelCartCoord& c);

    RelPolarCoord operator *= (float l)
    {
	dist *= l;
	return *this;
    }

    RelPolarCoord operator * (float l) const
    {
	RelPolarCoord copy = *this;
	return (copy *= l);
    }

    float lengthsq() const
    {
	return dist*dist;
    }

    RelPolarCoord rotated(float angle) const
    {
	RelPolarCoord copy = *this;
	copy.angle += angle;
	return copy;
    }
};

struct RelCartCoord
{
    // dy increases upwards
    float dx;
    float dy;

    RelCartCoord(float idx=0, float idy=0) :
	dx(idx),dy(idy)
    {}

    RelCartCoord(const RelPolarCoord &rp);

    RelCartCoord operator *= (float l)
    {
	dx *= l;
	dy *= l;
	return *this;
    }

    RelCartCoord operator * (float l) const
    {
	RelCartCoord copy = *this;
	return (copy *= l);
    }

    RelCartCoord operator += (const RelCartCoord &rc)
    {
	dx += rc.dx;
	dy += rc.dy;
	return *this;
    }
    RelCartCoord operator + (const RelCartCoord &rc) const
    {
	RelCartCoord copy = *this;
	return (copy += rc);
    }
    RelCartCoord operator -= (const RelCartCoord &rc)
    {
	dx -= rc.dx;
	dy -= rc.dy;
	return *this;
    }
    RelCartCoord operator - (const RelCartCoord &rc) const
    {
	RelCartCoord copy = *this;
	return (copy -= rc);
    }

    float lengthsq() const
    {
	return dx*dx+dy*dy;
    }

    RelCartCoord rotated(float angle) const;
};

struct CartCoord
{
    // y increases upwards
    float x;
    float y;

    CartCoord(float ix=0, float iy=0) :
	x(ix),y(iy) {}

    CartCoord operator += (const RelCartCoord &rc)
    {
	x += rc.dx;
	y += rc.dy;
	return *this;
    }

    CartCoord operator + (const RelCartCoord &rc) const
    {
	CartCoord copy = *this;
	return (copy += rc);
    }

    RelCartCoord operator - (const CartCoord &c) const
    {
	RelCartCoord rc(x-c.x, y-c.y);
	return rc;
    }
};

struct ScreenCoord
{
    // y increases *downwards*
    int x;
    int y;

    ScreenCoord(int ix=0, int iy=0) :
	x(ix),y(iy)
    {}
};

float dist(RelPolarCoord c);
Angle angle(RelPolarCoord c);

float angleDiff(Angle a1, Angle a2);

#endif /* INC_COORDS_H */
