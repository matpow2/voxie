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

#ifndef VOXIE_COLOR_H
#define VOXIE_COLOR_H

#include <algorithm>

#pragma pack(push)
#pragma pack(1)

class RGBColor
{
public:
    unsigned char r, g, b;

    RGBColor();
    RGBColor(int r, int g, int b);
    RGBColor(unsigned char r, unsigned char g, unsigned char b);
    RGBColor(unsigned int v);
    RGBColor(float r, float g, float b);
    void multiply(float mul);
    void mix(const RGBColor & other, float mul);
    void set_current(unsigned char alpha = 255) const;
    void set_current(float alpha) const;
};

#pragma pack(pop)

const RGBColor team1_color(72, 175, 69);
const RGBColor team2_color(72, 69, 175);
const RGBColor spec_color(127, 127, 127);
const RGBColor server_color(240, 100, 100);
const RGBColor hud_color(20, 20, 20);
const RGBColor white_color(255, 255, 255);
const RGBColor smoke_color(30, 30, 30);
const RGBColor fire_color(255, 119, 0);

const RGBColor & get_team_color(int team);

// r, g, b values are from 0 to 1
// h = [0, 360], s = [0, 1], v = [0, 1]
// if s == 0, then h = -1 (undefined)

void rgb_to_hsv(float r, float g, float b,
                float & h, float & s, float & v);

void hsv_to_rgb(float h, float s, float v,
                float & r, float & g, float & b);

#endif // VOXIE_COLOR_H
