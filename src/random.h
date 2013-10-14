/*
Copyright (c) 2013 Mathias Kaerlev

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#ifndef VOXIE_RANDOM_H
#define VOXIE_RANDOM_H

#define _USE_MATH_DEFINES
#include <math.h>
#include <stdlib.h>

#include "glm.h"

inline float randf()
{
    return float(rand()) / float(RAND_MAX);
}

inline float randrange(float a, float b)
{
    return a + (b - a) * randf();
}

inline float randrange(float a)
{
    return a * randf();
}

inline int randint(int a)
{
    return int(randrange(float(a)));
}

inline int randint(int a, int b)
{
    return int(randrange(float(a), float(b)));
}

inline void vecrand(vec3 & a)
{
    // uniform spherical randomization
    float f;
    a.z = -(float)rand()/(float)(RAND_MAX>>1) + 1.0f;
    float rad = rand()*(float(M_PI)*2.0f/float(RAND_MAX));
    a.x = cos(rad);
    a.y = sin(rad);
    f = sqrtf(1.0f - a.z*a.z);
    a.x *= f;
    a.y *= f;
}

#endif // VOXIE_RANDOM_H
