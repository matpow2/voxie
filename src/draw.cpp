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

#include "draw.h"
#include "mathcommon.h"

void setup_lighting()
{
    vec3 v1 = glm::normalize(vec3(0.3f, -0.7f, -0.6f));
    GLfloat light1_position[] = {v1.x, v1.y, v1.z, 0.0f};
    float diffuse = 0.6f;
    GLfloat light_diffuse[] = {diffuse, diffuse, diffuse, 1.0f};
    GLfloat light_ambient[] = {0.0f, 0.0f, 0.0f, 1.0f};
    float specular = 0.0f;
    GLfloat light_specular[] = {specular, specular, specular, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, light1_position);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    vec3 v2 = glm::normalize(vec3(-0.3f, 0.7f, -0.6f));
    GLfloat light2_position[] = {v2.x, v2.y, v2.z, 0.0f};
    glLightfv(GL_LIGHT1, GL_POSITION, light2_position);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);
    float f = 0.6f;
    GLfloat light_model[] = {f, f, f, 1.0f};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, light_model);
}

void draw_cube(float x1, float y1, float z1, float x2, float y2, float z2,
               unsigned char r, unsigned char g, unsigned char b,
               unsigned char a)
{
    float r_f = r / 255.0f;
    float g_f = g / 255.0f;
    float b_f = b / 255.0f;
    float a_f = a / 255.0f;

    // vertex, color, normal

    GLfloat data[] = {
        // top face
        x1, y2, z1,
        r_f, g_f, b_f, a_f,
        0.0f, 1.0f, 0.0f,
        x1, y2, z2,
        r_f, g_f, b_f, a_f,
        0.0f, 1.0f, 0.0f,
        x2, y2, z2,
        r_f, g_f, b_f, a_f,
        0.0f, 1.0f, 0.0f,
        x2, y2, z1,
        r_f, g_f, b_f, a_f,
        0.0f, 1.0f, 0.0f,

        // bottom face
        x1, y1, z1,
        r_f, g_f, b_f, a_f,
        0.0f, -1.0f, 0.0f,
        x2, y1, z1,
        r_f, g_f, b_f, a_f,
        0.0f, -1.0f, 0.0f,
        x2, y1, z2,
        r_f, g_f, b_f, a_f,
        0.0f, -1.0f, 0.0f,
        x1, y1, z2,
        r_f, g_f, b_f, a_f,
        0.0f, -1.0f, 0.0f,

        // front face
        x1, y1, z2,
        r_f, g_f, b_f, a_f,
        0.0f, 0.0f, -1.0f,
        x2, y1, z2,
        r_f, g_f, b_f, a_f,
        0.0f, 0.0f, -1.0f,
        x2, y2, z2,
        r_f, g_f, b_f, a_f,
        0.0f, 0.0f, -1.0f,
        x1, y2, z2,
        r_f, g_f, b_f, a_f,
        0.0f, 0.0f, -1.0f,

        // back face
        x1, y1, z1,
        r_f, g_f, b_f, a_f,
        0.0f, 0.0f, 1.0f,
        x1, y2, z1,
        r_f, g_f, b_f, a_f,
        0.0f, 0.0f, 1.0f,
        x2, y2, z1,
        r_f, g_f, b_f, a_f,
        0.0f, 0.0f, 1.0f,
        x2, y1, z1,
        r_f, g_f, b_f, a_f,
        0.0f, 0.0f, 1.0f,

        // right face
        x2, y1, z1,
        r_f, g_f, b_f, a_f,
        1.0f, 0.0f, 0.0f,
        x2, y2, z1,
        r_f, g_f, b_f, a_f,
        1.0f, 0.0f, 0.0f,
        x2, y2, z2,
        r_f, g_f, b_f, a_f,
        1.0f, 0.0f, 0.0f,
        x2, y1, z2,
        r_f, g_f, b_f, a_f,
        1.0f, 0.0f, 0.0f,

        // left face
        x1, y1, z1,
        r_f, g_f, b_f, a_f,
        -1.0f, 0.0f, 0.0f,
        x1, y1, z2,
        r_f, g_f, b_f, a_f,
        -1.0f, 0.0f, 0.0f,
        x1, y2, z2,
        r_f, g_f, b_f, a_f,
        -1.0f, 0.0f, 0.0f,
        x1, y2, z1,
        r_f, g_f, b_f, a_f,
        -1.0f, 0.0f, 0.0f
    };

#define VERTEX_SIZE (sizeof(float) * 10)

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glVertexPointer(3, GL_FLOAT, VERTEX_SIZE, &data[0]);
    glColorPointer(4, GL_FLOAT, VERTEX_SIZE, &data[3]);
    glNormalPointer(GL_FLOAT, VERTEX_SIZE, &data[7]);
    glDrawArrays(GL_QUADS, 0, 4 * 6);
}

void draw_cube(float x, float y, float z, float size,
               unsigned char r, unsigned char g, unsigned char b,
               unsigned char a)
{
    float half_size = size * 0.5f;
    draw_cube(x - half_size, y - half_size, z - half_size,
              x + half_size, y + half_size, z + half_size,
              r, g, b, a);
}

void draw_rounded_rect(float x1, float y1, float x2, float y2,
    unsigned char r, unsigned char g, unsigned char b, unsigned char a,
    float radius)
{
    glColor4ub(r, g, b, a);

    glBegin(GL_POLYGON);
    glVertex2f(x1+radius,y1);
    glVertex2f(x2-radius,y1);
    for(float i=(float)M_PI*1.5f;i<M_PI*2;i+=0.1f)
        glVertex2f(x2-radius+cos(i)*radius,y1+radius+sin(i)*radius);
    glVertex2f(x2,y1+radius);
    glVertex2f(x2,y2-radius);
    for(float i=0;i<(float)M_PI*0.5f;i+=0.1f)
        glVertex2f(x2-radius+cos(i)*radius,y2-radius+sin(i)*radius);
    glVertex2f(x2-radius,y2);
    glVertex2f(x1+radius,y2);
    for(float i=(float)M_PI*0.5f;i<M_PI;i+=0.1f)
        glVertex2f(x1+radius+cos(i)*radius,y2-radius+sin(i)*radius);
    glVertex2f(x1,y2-radius);
    glVertex2f(x1,y1+radius);
    for(float i=(float)M_PI;i<M_PI*1.5f;i+=0.1f)
        glVertex2f(x1+radius+cos(i)*radius,y1+radius+sin(i)*radius);
    glEnd();
}

void draw_rounded_rect(int x1, int y1, int x2, int y2,
    unsigned char r, unsigned char g, unsigned char b, unsigned char a,
    float radius)
{
    draw_rounded_rect(float(x1), float(y1), float(x2), float(y2),
                      r, g, b, a, radius);
}

void draw_rounded_rect(float x1, float y1, float x2, float y2,
    const RGBColor & color, unsigned char a,
    float radius)
{
    draw_rounded_rect(x1, y1, x2, y2, color.r, color.g, color.b, a, radius);
}

void draw_rounded_rect(int x1, int y1, int x2, int y2,
    const RGBColor & color, unsigned char a,
    float radius)
{
    draw_rounded_rect(float(x1), float(y1), float(x2), float(y2),
                      color, a, radius);
}

#define POINTER_SIZE 5

void draw_pointer(float x, float y,
                  unsigned char r, unsigned char g, unsigned char b)
{
    x = floor(x - POINTER_SIZE * 0.5f);
    y = floor(y - POINTER_SIZE * 0.5f);
    draw_rect(x, y, x + POINTER_SIZE, y + POINTER_SIZE,
              0, 0, 0, 255);
    draw_rect(x + 1, y + 1, x + POINTER_SIZE - 1, y + POINTER_SIZE - 1,
              r, g, b, 255);
}

void draw_pointer(float x, float y)
{
    draw_pointer(x, y, 255, 255, 255);
}

void setup_opengl()
{
    // OpenGL settings
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthFunc(GL_LEQUAL);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glClearDepth(1.0f);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glEnable(GL_LINE_SMOOTH);

    // lighting
    glShadeModel(GL_FLAT);
    glEnable(GL_NORMALIZE);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
}

vec3 perp(const vec3 & v) 
{
    float min = fabs(v.x);
    vec3 cardinal_axis(1.0f, 0.0f, 0.0f);
 
    if(fabs(v.y) < min) {
        min = fabs(v.y);
        cardinal_axis = vec3(0.0f, 1.0f, 0.0f);
    }
 
    if(fabs(v.z) < min)
        cardinal_axis = vec3(0.0f, 0.0f, 1.0f);
 
    return glm::cross(v, cardinal_axis);
}

void draw_cone(const vec3 & c, const vec3 & a, float rd, int n)
{

    vec3 d = glm::normalize(a - c);
    vec3 e0 = perp(d);
    vec3 e1 = glm::cross(e0, d);
    float ang_inc = 360.0f / n * (float(M_PI) / 180.0f);
 
    // draw cone top
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(a.x, a.y, a.z);
    for(int i = 0; i < n+1; i++) {
        float rad = ang_inc * i;
        vec3 p = c + (((e0 * cosf(rad)) + (e1 * sinf(rad))) * rd);
        glVertex3f(p.x, p.y, p.z);
    }
    glEnd();
 
    // draw cone bottom
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(c.x, c.y, c.z);
    for(int i = n; i >= 0; i--) {
        float rad = ang_inc * i;
        vec3 p = c + (((e0 * cosf(rad)) + (e1 * sinf(rad))) * rd);
        glVertex3f(p.x, p.y, p.z);
    }
    glEnd();
}