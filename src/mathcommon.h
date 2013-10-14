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

#ifndef VOXIE_MATHCOMMON_H
#define VOXIE_MATHCOMMON_H

#define _USE_MATH_DEFINES
#include <math.h>
#include <algorithm>

#define PI_F (float)(M_PI)

inline float mod(float a, float b)
{
    return a - b * floor(a / b);
}

inline float rad(float x)
{
    return x * float(PI_F/180.0f);
}

inline float deg(float x)
{
    return x * float(180.0f/PI_F);
}

inline float sin_deg(float x)
{
    return sin(rad(x));
}

inline float cos_deg(float x)
{
    return cos(rad(x));
}

inline float atan2_deg(float a, float b)
{
    return deg(atan2(a, b));
}

inline int int_round(float d)
{
    int v = (int)floor(d + 0.5);
    if (d - v > 0.5)
        v++;
    return v;
}

inline int int_min(int value1, int value2)
{
    return std::min<int>(value1, value2);
}

inline int int_max(int value1, int value2)
{
    return std::max<int>(value1, value2);
}

template <class T>
inline T clamp(T val)
{
    return std::max<T>(0, std::min<T>(val, 1));
}

template <class T>
inline float distance(T x1, T y1, T x2, T y2)
{
    float x = float(x2 - x1);
    float y = float(y2 - y1);
    return sqrt(x*x + y*y);
}

inline float circle_tangent_angle(float x1, float y1, float x2, float y2,
                                  float d)
{
    float dy = y1 - y2;
    float dx = x1 - x2;
    float val = -asin(d / sqrt(dx*dx + dy*dy));
    val += atan2(dy, dx) - PI_F * 0.5f;
    return deg(val);
}

template <class T>
inline float sin_wave(T v, float a = 0.0f, float b = 1.0f,
                      float length = 1.0f)
{
    float value = float(v);
    value /= length;
    value = mod(value, (2.0f * PI_F));
    return (sin(value) + 1.0f) / 2.0f * (b - a) + a;
}

template <class T>
inline float cos_wave(T v, float a = 0.0f, float b = 1.0f,
                      float length = 1.0f)
{
    float value = float(v);
    value /= length;
    value = mod(value, (2.0f * PI_F));
    return (cos(value) + 1.0f) / 2.0f * (b - a) + a;
}

inline unsigned char mix_component(unsigned char a, unsigned char b,
                                   float mul)
{
    float v = float(a) + float(b - a) * mul;
    return std::max<int>(0, std::min<int>(255, int(v)));
}

inline void mix_color(unsigned char r1, unsigned char g1, unsigned char b1,
                      unsigned char r2, unsigned char g2, unsigned char b2,
                      unsigned char &r3, unsigned char &g3, unsigned char &b3,
                      float mul)
{
    r3 = mix_component(r1, r2, mul);
    g3 = mix_component(g1, g2, mul);
    b3 = mix_component(b1, b2, mul);
}

inline void mul_component(unsigned char & v, float mul)
{
    v = std::max<int>(0, std::min<int>(255, int(float(v) * mul)));
}

inline void mul_color(unsigned char & r, unsigned char & g, unsigned char & b,
                      float mul)
{
    mul_component(r, mul);
    mul_component(g, mul);
    mul_component(b, mul);
}

inline unsigned int next_pow2(unsigned int v)
{
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}

inline unsigned int prev_pow2(unsigned int x)
{
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return x - (x >> 1);
}

#endif // VOXIE_MATHCOMMON_H
