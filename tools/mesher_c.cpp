/*
 Copyright (c) 2013 yvt

 This file is part of OpenSpades.

 OpenSpades is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 OpenSpades is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with OpenSpades.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
changed for the purpose of generating meshes for voxie
*/

#include "c2t.h"
#include <stdint.h>

class SliceGenerator
{
public:
    //0 - air
    //1 - unprocessed solid
    //2 - "inside" the processed area, but not connected
    //3 - found holes
    uint8_t *slice;
    int usize, vsize;
    int minU, maxU, minV, maxV;

    class Model
    {
    public:
        const uint8_t *slice;
        int usize, vsize;

        Model(const uint8_t *slice, int usize, int vsize)
        : slice(slice), usize(usize), vsize(vsize)
        {
        }

        inline int GetWidth()
        {
            return usize;
        }

        inline int GetHeight()
        {
            return vsize;
        }

        inline bool operator()(int x, int y) const {
            if (x < 0 || y < 0 || x >= usize || y >= vsize)
                return false;
            return slice[x * vsize + y] != 0;
        }
    };

    uint8_t & Slice(int u, int v)
    {
        return slice[u * vsize + v];
    }

    uint8_t GetSlice(int u, int v)
    {
        if (u < 0 || v < 0 || u >= usize || v >= vsize)
            return 0;
        return Slice(u, v);
    }

    void ProcessArea(std::vector<c2t::Point> & ret)
    {
        Model model(slice, usize, vsize);
        c2t::Trianglulator<Model> mesher(model, ret);
        mesher.triangulate();
    }
};

#define VOXEL_AIR 255

#include <iostream>

class MesherModel
{
public:
    unsigned char * data;
    int x_size, y_size, z_size;

    MesherModel()
    {
    }

    void init(char * data, int x_size, int y_size, int z_size)
    {
        this->data = (unsigned char*)data;
        this->x_size = x_size;
        this->y_size = y_size;
        this->z_size = z_size;
    }

    inline unsigned char get(int x, int y, int z)
    {
        if (x < 0 || y < 0 || z < 0 ||
            x >= x_size || y >= y_size || z >= z_size)
            return VOXEL_AIR;
        return data[z + y * z_size + x * z_size * y_size];
    }

    inline bool test(int x, int y, int z, unsigned char p)
    {
        unsigned char r = get(x, y, z);
        return r != VOXEL_AIR && r == p;
    }

    inline bool is_solid(int x, int y, int z, unsigned char p)
    {
        unsigned char r = get(x, y, z);
        return r != VOXEL_AIR;
    }
};

struct Vertex
{
    uint8_t x, y, z;
    int8_t nx, ny, nz;
};

class Mesher
{
public:
    std::vector<Vertex> vertices;
    std::vector<int> indices;

    static int64_t doubled_triangle_area(c2t::Point v1, c2t::Point v2,
                                         c2t::Point v3)
    {
        int64_t x1 = v1.x, y1 = v1.y;
        int64_t x2 = v2.x, y2 = v2.y;
        int64_t x3 = v3.x, y3 = v3.y;
        return (x1 - x3) * (y2 - y1) - (x1 - x2) * (y3 - y1);
    }

    void emit_slice(uint8_t *slice, int usize, int vsize,
                    int sx, int sy, int sz,
                    int ux, int uy, int uz,
                    int vx, int vy, int vz,
                    int mx, int my, int mz,
                    bool flip)
    {
        int minU = -1, minV = -1, maxU = -1, maxV = -1;

        for (int u = 0; u < usize; u++) {
            for (int v = 0; v < vsize; v++) {
                if (slice[u * vsize + v]) {
                    if (minU == -1 || u < minU)
                        minU = u;
                    if (maxU == -1 || u > maxU)
                        maxU = u;
                    if (minV == -1 || v < minV)
                        minV = v;
                    if (maxV == -1 || v > maxV)
                        maxV = v;
                }
            }
        }

        if (minU == -1) {
            // no face
            return;
        }

        int nx, ny, nz;
        nx = uy * vz - uz * vy;
        ny = uz * vx - ux * vz;
        nz = ux * vy - uy * vx;
        if (!flip) {
            nx = -nx;
            ny = -ny;
            nz = -nz;
        }

        SliceGenerator generator;
        generator.slice = slice;
        generator.usize = usize;
        generator.vsize = vsize;
        generator.minU = minU;
        generator.maxU = maxU;
        generator.minV = minV;
        generator.maxV = maxV;

        int tu = minU - 1;
        int tv = minV - 1;

        // TODO: optimize scan range
        std::vector<c2t::Point> polys;
        generator.ProcessArea(polys);
        for (std::size_t i = 0; i < polys.size(); i += 3) {
            uint32_t idx = (uint32_t)vertices.size();
            c2t::Point pt1 = polys[i + 0];
            c2t::Point pt2 = polys[i + 1];
            c2t::Point pt3 = polys[i + 2];

            // degenerate triangle
            if (doubled_triangle_area(pt1, pt2, pt3) == 0)
                continue;

            Vertex vtx;
            vtx.nx = nx; vtx.ny = ny; vtx.nz = nz;

            vtx.x = sx + (int)pt1.x * ux + (int)pt1.y * vx;
            vtx.y = sy + (int)pt1.x * uy + (int)pt1.y * vy;
            vtx.z = sz + (int)pt1.x * uz + (int)pt1.y * vz;
            vertices.push_back(vtx);

            vtx.x = sx + (int)pt2.x * ux + (int)pt2.y * vx;
            vtx.y = sy + (int)pt2.x * uy + (int)pt2.y * vy;
            vtx.z = sz + (int)pt2.x * uz + (int)pt2.y * vz;
            vertices.push_back(vtx);

            vtx.x = sx + (int)pt3.x * ux + (int)pt3.y * vx;
            vtx.y = sy + (int)pt3.x * uy + (int)pt3.y * vy;
            vtx.z = sz + (int)pt3.x * uz + (int)pt3.y * vz;
            vertices.push_back(vtx);

            if (!flip) {
                indices.push_back(idx+2);
                indices.push_back(idx+1);
                indices.push_back(idx);

            } else{
                indices.push_back(idx);
                indices.push_back(idx+1);
                indices.push_back(idx+2);
            }
        }

    }

    void build_vertices(MesherModel * model, unsigned char p)
    {
        vertices.clear();
        indices.clear();

        int w = model->x_size;
        int h = model->y_size;
        int d = model->z_size;

        std::vector<uint8_t> slice;

        // x-slice
        slice.resize(h * d);
        std::fill(slice.begin(), slice.end(), 0);
        for (int x = 0; x < w; x++) {
            for (int y = 0; y < h; y++) {
                for (int z = 0; z < d; z++) {
                    uint8_t& s = slice[y * d + z];
                    if (x == 0)
                        s = model->test(x, y, z, p) ? 1 : 0;
                    else
                        s = (model->test(x, y, z, p) &&
                             !model->is_solid(x - 1, y, z, p))
                        ? 1 : 0;
                }
            }
            emit_slice(slice.data(), h, d,
                       x, 0, 0,
                       0, 1, 0,
                       0, 0, 1,
                       x, 0, 0,
                       false);

            for (int y = 0; y < h; y++) {
                for (int z = 0; z < d; z++) {
                    uint8_t& s = slice[y * d + z];
                    if (x == w - 1)
                        s = model->test(x, y, z, p) ? 1 : 0;
                    else
                        s = (model->test(x, y, z, p) &&
                             !model->is_solid(x + 1, y, z, p))
                        ? 1 : 0;
                }
            }
            emit_slice(slice.data(), h, d,
                       x + 1, 0, 0,
                       0, 1, 0,
                       0, 0, 1,
                       x, 0, 0,
                       true);
        }

        // y-slice
        slice.resize(w * d);
        std::fill(slice.begin(), slice.end(), 0);
        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
                for (int z = 0; z < d; z++) {
                    uint8_t& s = slice[x * d + z];
                    if (y == 0)
                        s = model->test(x, y, z, p) ? 1 : 0;
                    else
                        s = (model->test(x, y, z, p) &&
                             !model->is_solid(x, y - 1, z, p))
                        ? 1 : 0;
                }
            }
            emit_slice(slice.data(), w, d,
                       0, y, 0,
                       1, 0, 0,
                       0, 0, 1,
                       0, y, 0,
                       true);

            for (int x = 0; x < w; x++) {
                for (int z = 0; z < d; z++) {
                    uint8_t& s = slice[x * d + z];
                    if (y == h - 1)
                        s = model->test(x, y, z, p) ? 1 : 0;
                    else
                        s = (model->test(x, y, z, p) &&
                             !model->is_solid(x, y + 1, z, p))
                        ? 1 : 0;
                }
            }
            emit_slice(slice.data(), w, d,
                       0, y + 1, 0,
                       1, 0, 0,
                       0, 0, 1,
                       0, y, 0,
                       false);
        }

        // z-slice
        slice.resize(w * h);
        std::fill(slice.begin(), slice.end(), 0);
        for (int z = 0; z < d; z++) {
            for (int x = 0; x < w; x++) {
                for (int y = 0; y < h; y++) {
                    uint8_t& s = slice[x * h + y];
                    if (z == 0)
                        s = model->test(x, y, z, p) ? 1 : 0;
                    else
                        s = (model->test(x, y, z, p) &&
                             !model->is_solid(x, y, z - 1, p))
                        ? 1 : 0;
                }
            }
            emit_slice(slice.data(), w, h,
                       0, 0, z,
                       1, 0, 0,
                       0, 1, 0,
                       0, 0, z,
                       false);

            for (int x = 0; x < w; x++) {
                for (int y = 0; y < h; y++) {
                    uint8_t& s = slice[x * h + y];
                    if (z == d - 1)
                        s = model->test(x, y, z, p) ? 1 : 0;
                    else
                        s = (model->test(x, y, z, p) &&
                             !model->is_solid(x, y, z + 1, p))
                        ? 1 : 0;
                }
            }
            emit_slice(slice.data(), w, h,
                       0, 0, z + 1,
                       1, 0, 0,
                       0, 1, 0,
                       0, 0, z,
                       true);
        }
    }
};
