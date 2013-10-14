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

#ifndef VOXIE_COLLISION_H
#define VOXIE_COLLISION_H

#include "glm.h"

inline void test_ray_plane(const vec3 & pos1, const vec3 & dir,
                           const vec3 & pos2, const vec3 & normal,
                           float & t)
{
    t = glm::dot(normal, pos2 - pos1) / glm::dot(normal, dir);
}

inline void test_ray_plane(const vec3 & pos1, const vec3 & dir,
                           const vec3 & pos2, const vec3 & normal,
                           vec3 & hit)
{
    float t;
    test_ray_plane(pos1, dir, pos2, normal, t);
    hit = pos1 + dir * t;
}

inline bool intersects(const vec3 & p, const vec3 & min, const vec3 & max)
{
    return p.x > min.x && p.x < max.x && p.y > min.y && p.y < max.y &&
           p.z > min.z && p.z < max.z;
}

inline bool intersects(const vec3 & min1, const vec3 & max1,
                       const vec3 & min2, const vec3 & max2)
{
    if (max1.x < min2.x || min1.x > max2.x ||
        max1.y < min2.y || min1.y > max2.y ||
        max1.z < min2.z || min1.z > max2.z)
        return false;
    return true;
}

inline bool test_aabb_frustum(const vec3 & min, const vec3 & max,
                              vec4 * planes)
{
    for (int i = 0; i < 6; ++i) {
        vec4 & plane = planes[i];

        vec3 pv(
            plane.x < 0 ? max.x : min.x,
            plane.y < 0 ? max.y : min.y,
            plane.z < 0 ? max.z : min.z
        );

        if (glm::dot(pv, vec3(plane.x, plane.y, plane.z)) + plane.w > 0)
            return false;
    }
    return true;
}

#endif // VOXIE_COLLISION_H
