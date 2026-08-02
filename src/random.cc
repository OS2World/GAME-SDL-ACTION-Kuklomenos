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

#include <cmath>
#include <cstdlib>

float ranf()
{
    return (float)(rand())/RAND_MAX;
}
float ranf(float m)
{
    return m*ranf();
}

// rani(n) returns uniformly random integer in [0,n).
int rani(int m)
{
    float r;
    do
    { r = ranf(); } while (r == 1);

    return (int)floorf(r*m);
}

/* Returns a random number with distribution N(0,1). Uses Box-Muller. Code
 * taken from http://www.taygeta.com/random/gaussian.html.
 */
float gaussian()
{
    float x1, x2, w, y;
    static float spare_y = 1000;

    // the algorithm below returns two independent normally distributed
    // numbers, so we store the spare one in a static variable for use on the
    // subsequent call.
    if (spare_y != 1000)
    {
	float ret = spare_y;
	spare_y = 1000;
	return ret;
    }

    do {
	x1 = 2.0 * ranf() - 1.0;
	x2 = 2.0 * ranf() - 1.0;
	w = x1 * x1 + x2 * x2;
    } while ( w >= 1.0 );

    w = sqrt( (-2.0 * log( w ) ) / w );
    y = x1 * w;
    spare_y = x2 * w;

    return y;
}
