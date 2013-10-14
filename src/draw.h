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

#ifndef VOXIE_DRAW_H
#define VOXIE_DRAW_H

#include "color.h"
#include "glm.h"
#include "include_gl.h"

void setup_lighting();

void draw_rounded_rect(float x1, float y1, float x2, float y2,
    unsigned char r, unsigned char g, unsigned char b, unsigned char a,
    float radius = 5.0f);

void draw_rounded_rect(int x1, int y1, int x2, int y2,
    unsigned char r, unsigned char g, unsigned char b, unsigned char a,
    float radius = 5.0f);

void draw_rounded_rect(float x1, float y1, float x2, float y2,
    const RGBColor & color, unsigned char a, float radius = 5.0f);

void draw_rounded_rect(int x1, int y1, int x2, int y2,
    const RGBColor & color, unsigned char a, float radius = 5.0f);

void draw_pointer(float x, float y);
void draw_pointer(float x, float y,
                  unsigned char r, unsigned char g, unsigned char b);

inline void draw_rect(float x1, float y1, float x2, float y2,
    unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    glColor4ub(r, g, b, a);
    glBegin(GL_QUADS);
    glVertex2f(x1, y1);
    glVertex2f(x2, y1);
    glVertex2f(x2, y2);
    glVertex2f(x1, y2);
    glEnd();
}

inline void draw_rect(float x1, float y1, float z1, float x2, float y2, float z2,
    unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    glColor4ub(r, g, b, a);
    glBegin(GL_QUADS);
    glVertex3f(x1, y1, z1);
    glVertex3f(x2, y1, z1);
    glVertex3f(x2, y2, z2);
    glVertex3f(x1, y2, z2);
    glEnd();
}

inline void draw_rect(int x1, int y1, int x2, int y2,
    unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    glColor4ub(r, g, b, a);
    glBegin(GL_QUADS);
    glVertex2i(x1, y1);
    glVertex2i(x2, y1);
    glVertex2i(x2, y2);
    glVertex2i(x1, y2);
    glEnd();
}

inline void draw_rect_outline(float x1, float y1, float x2, float y2, float size,
    unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    draw_rect(x1, y1, x1 + size, y2, r, g, b, a);
    draw_rect(x2 - size, y1, x2, y2, r, g, b, a);
    draw_rect(x1 + size, y1, x2 - size, y1 + size, r, g, b, a);
    draw_rect(x1 + size, y2 - size, x2 - size, y2, r, g, b, a);
}

void draw_cube(float x1, float y1, float z1, float x2, float y2, float z2,
               unsigned char r, unsigned char g, unsigned char b,
               unsigned char a);

void draw_cube(float x, float y, float z, float size,
               unsigned char r, unsigned char g, unsigned char b,
               unsigned char a);

inline void draw_wireframe_cube(float x1, float y1, float z1, float x2,
    float y2, float z2, unsigned char r, unsigned char g, unsigned char b,
    unsigned char a)
{
    glLineWidth(1.0f);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    draw_cube(x1, y1, z1, x2, y2, z2, r, g, b, a);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

inline void draw_wireframe_cube(float x, float y, float z, float size,
    unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    float half_size = size * 0.5f;
    draw_wireframe_cube(x - half_size, y - half_size, z - half_size,
                        x + half_size, y + half_size, z + half_size,
                        r, g, b, a);
}

void setup_opengl();

void draw_cone(const vec3 & a, const vec3 & b, float rd, int n = 8);

#endif // VOXIE_DRAW_H
