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

#ifndef VOXIE_GLM_H
#define VOXIE_GLM_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/quaternion.hpp>
#include "include_gl.h"

using glm::mat4;
using glm::mat3;
using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::ivec2;
using glm::ivec3;
using glm::ivec4;
using glm::quat;

inline void multiply_matrix(const mat4 & mat)
{
    glMultMatrixf(glm::value_ptr(mat));
}

inline void multiply_matrix(const quat & q)
{
    multiply_matrix(glm::mat4_cast(q));
}

inline void load_matrix(const mat4 & mat)
{
    glLoadMatrixf(glm::value_ptr(mat));
}

inline vec3 unproject(const vec3 & win, const mat4 & inverse_mvp,
                      const vec4 & viewport)
{

    vec4 tmp = vec4(win, 1.0f);
    tmp.x = (tmp.x - viewport[0]) / viewport[2];
    tmp.y = (tmp.y - viewport[1]) / viewport[3];
    tmp = tmp * 2.0f - 1.0f;
    vec4 obj = inverse_mvp * tmp;
    obj /= obj.w;
    return vec3(obj);
}

inline bool project(const vec3 & obj, const mat4 & mvp, const vec4 & viewport,
                    vec2 & out)
{
    vec4 tmp = vec4(obj, 1.0f);
    tmp = mvp * tmp;
    if (tmp.w < 0)
        return false;
    tmp /= tmp.w;
    tmp = tmp * 0.5f + 0.5f;
    tmp[0] = tmp[0] * viewport[2] + viewport[0];
    tmp[1] = tmp[1] * viewport[3] + viewport[1];
    out = vec2(tmp);
    return true;
}

inline vec3 get_coordinates_z(const vec2 & win, float z,
                              const mat4 & inverse_mvp,
                              const vec4 & viewport)
{
    vec3 v1 = unproject(vec3(win, 0.0f), inverse_mvp, viewport);
    vec3 v2 = unproject(vec3(win, 1.0f), inverse_mvp, viewport);
    vec3 v = v2 - v1;
    float t = (z - v1.z) / v.z;
    float x = v1.x + v.x * t;
    float y = v1.y + v.y * t;
    return vec3(x, y, z);
}

inline void get_window_ray(const vec2 & win, const mat4 & inverse_mvp,
                           const vec4 & viewport, vec3 & pos, vec3 & dir)
{
    pos = unproject(vec3(win, 0.0f), inverse_mvp, viewport);
    vec3 pos2 = unproject(vec3(win, 1.0f), inverse_mvp, viewport);
    dir = glm::normalize(pos2 - pos);
}

#include <btBulletDynamicsCommon.h>

inline btVector3 convert_vec(const vec3 & value)
{
    return btVector3(value.x, value.y, value.z);
}

inline vec3 convert_vec(const btVector3 & value)
{
    return vec3(value.x(), value.y(), value.z());
}

#endif // VOXIE_GLM_H
