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

#include "color.h"
#include "mathcommon.h"
#include "include_gl.h"

// RGBColor

RGBColor::RGBColor()
: r(0), g(0), b(0)
{
}

RGBColor::RGBColor(int r, int g, int b)
: r(r), g(g), b(b)
{
}

RGBColor::RGBColor(unsigned char r, unsigned char g, unsigned char b)
: r(r), g(g), b(b)
{
}

RGBColor::RGBColor(unsigned int v)
{
    RGBColor & other = (RGBColor&)v;
    r = other.r;
    g = other.g;
    b = other.b;
}

RGBColor::RGBColor(float r, float g, float b)
: r((unsigned char)(r * 255.0f)),
  g((unsigned char)(g * 255.0f)),
  b((unsigned char)(b * 255.0f))
{
}

void RGBColor::multiply(float mul)
{
    mul_component(r, mul);
    mul_component(g, mul);
    mul_component(b, mul);
}

void RGBColor::mix(const RGBColor & other, float mul)
{
    r = mix_component(r, other.r, mul);
    g = mix_component(g, other.g, mul);
    b = mix_component(b, other.b, mul);
}

void RGBColor::set_current(unsigned char a) const
{
    glColor4ub(r, g, b, a);
}

void RGBColor::set_current(float a) const
{
    set_current((unsigned char)(a * 255.0f));
}

// color conversion

void rgb_to_hsv(float r, float g, float b,
                float & h, float & s, float & v)
{
    float min, max, delta;
    min = std::min<float>(r, std::min<float>(g, b));
    max = std::max<float>(r, std::max<float>(g, b));
    v = max;
    delta = max - min;
    if(delta != 0)
        s = delta / max;
    else {
        // r = g = b = 0
        // s = 0, v is undefined
        h = 0.0f;
        s = 0.0f;
        v = min;
        return;
    }
    if (r == max)
        h = (g - b) / delta; // between yellow & magenta
    else if(g == max)
        h = 2 + (b - r) / delta; // between cyan & yellow
    else
        h = 4 + (r - g) / delta; // between magenta & cyan
    h *= 60; // degrees
    if(h < 0)
        h += 360;
}

void hsv_to_rgb(float h, float s, float v,
                       float & r, float & g, float & b)
{
    if (s == 0) {
        // achromatic (grey)
        r = g = b = v;
        return;
    }
    int i;
    float f, p, q, t;
    h /= 60; // sector 0 to 5
    i = int(floor(h));
    f = h - i; // factorial part of h
    p = v * (1 - s);
    q = v * (1 - s * f);
    t = v * (1 - s * ( 1 - f ));
    switch (i) {
        case 0:
            r = v;
            g = t;
            b = p;
            break;
        case 1:
            r = q;
            g = v;
            b = p;
            break;
        case 2:
            r = p;
            g = v;
            b = t;
            break;
        case 3:
            r = p;
            g = q;
            b = v;
            break;
        case 4:
            r = t;
            g = p;
            b = v;
            break;
        case 5:
            r = v;
            g = p;
            b = q;
            break;
    }
}
