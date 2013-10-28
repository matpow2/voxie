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

#include <iostream>
#include <fstream>
#include <sstream>

#include "voxel.h"
#include <QDataStream>

RGBColor * global_palette = NULL;

#include <glm/gtc/noise.hpp>

// VoxelModel

VoxelModel::VoxelModel(VoxelFile * file)
: file(file), value(0)
{
}

void VoxelModel::draw_immediate(float alpha, bool offset)
{
    int x_offset, y_offset, z_offset;
    if (offset) {
        x_offset = file->x_offset;
        y_offset = file->y_offset;
        z_offset = file->z_offset;
    } else
        x_offset = y_offset = z_offset = 0;

    glBegin(GL_QUADS);
    unsigned char alpha_c = (unsigned char)(alpha * 255.0f);
    int x, y, z;
    for (x = 0; x < file->x_size; x++)
    for (y = 0; y < file->y_size; y++)
    for (z = 0; z < file->z_size; z++) {
        unsigned char color = file->get(x, y, z);
        if (color == VOXEL_AIR)
            continue;
        RGBColor & color2 = global_palette[color];

        float noise = glm::simplex(vec3(x, y, z));
        vec3 color3 = vec3(color2.r, color2.g, color2.b);
        color3 *= (1.0f + noise * 0.01f);
        color3 = glm::clamp(color3, 0, 255);

        glColor4ub(int(color3.x), int(color3.y), int(color3.z), alpha_c);

        float gl_x1 = float(x + x_offset);
        float gl_x2 = gl_x1 + 1.0f;
        float gl_y1 = float(y + y_offset);
        float gl_y2 = gl_y1 + 1.0f;
        float gl_z1 = float(z + z_offset);
        float gl_z2 = gl_z1 + 1.0f;

        // Left Face
        if (!file->is_solid(x, y + 1, z)) {
            glNormal3f(0.0f, 1.0f, 0.0f);
            glVertex3f(gl_x1, gl_y2, gl_z1);
            glVertex3f(gl_x1, gl_y2, gl_z2);
            glVertex3f(gl_x2, gl_y2, gl_z2);
            glVertex3f(gl_x2, gl_y2, gl_z1);
        }

        // Right face
        if (!file->is_solid(x, y - 1, z)) {
            glNormal3f(0.0f, -1.0f, 0.0f);
            glVertex3f(gl_x1, gl_y1, gl_z1); // Top right
            glVertex3f(gl_x2, gl_y1, gl_z1); // Top left
            glVertex3f(gl_x2, gl_y1, gl_z2); // Bottom left
            glVertex3f(gl_x1, gl_y1, gl_z2); // Bottom right
        }

        // Top face
        if (!file->is_solid(x, y, z + 1)) {
            glNormal3f(0.0f, 0.0f, -1.0f);
            glVertex3f(gl_x1, gl_y1, gl_z2); // Bottom left
            glVertex3f(gl_x2, gl_y1, gl_z2); // Bottom right
            glVertex3f(gl_x2, gl_y2, gl_z2); // Top right
            glVertex3f(gl_x1, gl_y2, gl_z2); // Top left
        }

        // Bottom face
        if (!file->is_solid(x, y, z - 1)) {
            glNormal3f(0.0f, 0.0f, 1.0f);
            glVertex3f(gl_x1, gl_y1, gl_z1); // Bottom right
            glVertex3f(gl_x1, gl_y2, gl_z1); // Top right
            glVertex3f(gl_x2, gl_y2, gl_z1); // Top left
            glVertex3f(gl_x2, gl_y1, gl_z1); // Bottom left
        }

        // Right face
        if (!file->is_solid(x + 1, y, z)) {
            glNormal3f(1.0f, 0.0f, 0.0f);
            glVertex3f(gl_x2, gl_y1, gl_z1); // Bottom right
            glVertex3f(gl_x2, gl_y2, gl_z1); // Top right
            glVertex3f(gl_x2, gl_y2, gl_z2); // Top left
            glVertex3f(gl_x2, gl_y1, gl_z2); // Bottom left
        }

        // Left Face
        if (!file->is_solid(x - 1, y, z)) {
            glNormal3f(-1.0f, 0.0f, 0.0f);
            glVertex3f(gl_x1, gl_y1, gl_z1); // Bottom left
            glVertex3f(gl_x1, gl_y1, gl_z2); // Bottom right
            glVertex3f(gl_x1, gl_y2, gl_z2); // Top right
            glVertex3f(gl_x1, gl_y2, gl_z1); // Top left
        }
    }
    glEnd();
}

void VoxelModel::update(bool force)
{
    if (!force && value != 0)
        return;
    glNewList(value, GL_COMPILE);
    draw_immediate();
    glEndList();
}

void VoxelModel::draw()
{
    if (value == 0) {
        value = glGenLists(1);
        update();
    }
    glDisable(GL_TEXTURE_2D);
    glCallList(value);
}

VoxelModel::~VoxelModel()
{
    glDeleteLists(value, 1);
}

ReferencePoint * VoxelModel::get_point(const QString & name)
{
    return file->get_point(name);
}

// ReferencePoint

ReferencePoint::ReferencePoint(const QString & name,int x, int y, int z)
: name(name), x(x), y(y), z(z)
{

}
void ReferencePoint::translate()
{
    glTranslatef(float(x) + 0.01f, float(y) + 0.01f, float(z) + 0.01f);
}

// VoxelFile

#define PALETTE_FILE "palette.dat"

void VoxelFile::load_palette()
{
    if (global_palette != NULL)
        return;
    global_palette = new RGBColor[256];
    QFile fp(PALETTE_FILE);
    if (!fp.open(QIODevice::ReadOnly))
        return;
    global_palette = new RGBColor[256];
    for (int i = 0; i < 64; i++)
        fp.read((char*)&global_palette[i], sizeof(RGBColor));
    fp.close();
}

void VoxelFile::save_palette()
{
    if (global_palette == NULL)
        return;
    QFile fp(PALETTE_FILE);
    if (!fp.open(QIODevice::WriteOnly))
        return;
    for (int i = 0; i < 64; i++)
        fp.write((char*)&global_palette[i], sizeof(RGBColor));
    fp.close();
}

unsigned char VoxelFile::get_closest_index(RGBColor c)
{
    load_palette();
    vec3 color_vec(c.r, c.g, c.b);
    bool is_set = false;
    float dist;
    unsigned char current_index = 0;
    for (int i = 0; i < 256; i++) {
        RGBColor & pal_color = global_palette[i];
        vec3 new_vec(pal_color.r, pal_color.g, pal_color.b);
        float new_dist = glm::distance(color_vec, new_vec);
        if (is_set && new_dist >= dist)
            continue;
        is_set = true;
        dist = new_dist;
        current_index = i;
    }
    return current_index;
}

VoxelFile::VoxelFile()
: data(NULL), model(NULL), shape(NULL)
{
    load_palette();
}

VoxelFile::VoxelFile(QFile & fp)
: data(NULL), model(NULL), shape(NULL)
{
    load_palette();
    load_fp(fp);
}

VoxelFile::VoxelFile(const QString & filename)
: data(NULL), model(NULL), shape(NULL)
{
    load_palette();
    load(filename);
}

VoxelFile::VoxelFile(int x_size, int y_size, int z_size)
: x_offset(0), y_offset(0), z_offset(0), data(NULL), model(NULL), shape(NULL)
{
    load_palette();
    reset(x_size, y_size, z_size);
}

VoxelFile::~VoxelFile()
{
    delete shape;
    delete[] data;
}

void VoxelFile::update_model()
{
    if (model == NULL)
        return;
    model->update();
}

void VoxelFile::reset(int x_size, int y_size, int z_size)
{
    this->x_size = x_size;
    this->y_size = y_size;
    this->z_size = z_size;
    if (data != NULL)
        delete[] data;
    data = new unsigned char[x_size * y_size * z_size];
    memset(data, 255, x_size * y_size * z_size);
    points.clear();
    reset_shape();
}

void VoxelFile::add_point(const QString & name,
                          int x, int y, int z)
{
    points.push_back(ReferencePoint(name, x, y, z));
}

void VoxelFile::remove_point(size_t i)
{
    points.erase(points.begin() + i);
}

ReferencePoint * VoxelFile::get_point(const QString & name)
{
    // XXX use fast_map
    ReferencePoints::iterator it;
    for (it = points.begin(); it != points.end(); it++) {
        if ((*it).name == name)
            return &(*it);
    }
    return NULL;
}

ReferencePoint * VoxelFile::get_point(int i)
{
    if (i < 0 || i >= int(points.size()))
        return NULL;
    return &points[i];
}

void VoxelFile::resize(int x1, int y1, int z1, int new_x, int new_y, int new_z)
{
    if (x1 == 0 && y1 == 0 && z1 == 0 &&
        x_size == new_x && y_size == new_y && z_size == new_z)
        return;

    unsigned char * new_data = new unsigned char[new_x * new_y * new_z];
    int nx, ny, nz;
    for (int x = x1; x < x1 + new_x; x++)
    for (int y = y1; y < y1 + new_y; y++)
    for (int z = z1; z < z1 + new_z; z++) {
        unsigned char c;
        if (x < 0 || x >= x_size ||
            y < 0 || y >= y_size ||
            z < 0 ||z >= z_size)
            c = VOXEL_AIR;
        else
            c = get(x, y, z);
        nx = x - x1;
        ny = y - y1;
        nz = z - z1;
        new_data[nz + ny * new_z + nx * new_z * new_y] = c;
    }
    delete[] data;
    data = new_data;
    x_size = new_x;
    y_size = new_y;
    z_size = new_z;
    x_offset += x1;
    y_offset += y1;
    z_offset += z1;
}

void VoxelFile::scale(float sx, float sy, float sz)
{
    if (sx == 1.0f && sy == 1.0f && sz == 1.0f)
        return;

    int new_x = int(x_size * sx);
    int new_y = int(y_size * sy);
    int new_z = int(z_size * sz);

    unsigned char * new_data = new unsigned char[new_x * new_y * new_z];

    for (int x = 0; x < new_x; x++)
    for (int y = 0; y < new_y; y++)
    for (int z = 0; z < new_z; z++) {
        int x2 = int(x / sx);
        int y2 = int(y / sy);
        int z2 = int(z / sz);
        new_data[z + y * new_z + x * new_z * new_y] = get(x2, y2, z2);
    }
    delete[] data;
    data = new_data;
    x_size = new_x;
    y_size = new_y;
    z_size = new_z;
    x_offset = int(x_offset * sx);
    y_offset = int(y_offset * sy);
    z_offset = int(z_offset * sz);
}

void VoxelFile::set_offset(int new_x, int new_y, int new_z)
{
    x_offset = new_x;
    y_offset = new_y;
    z_offset = new_z;
}

vec3 VoxelFile::get_min()
{
    return vec3(x_offset, y_offset, z_offset);
}

vec3 VoxelFile::get_max()
{
    return vec3(x_offset + x_size, y_offset + y_size, z_offset + z_size);
}

void VoxelFile::optimize()
{
    int x1, y1, z1, x2, y2, z2, x, y, z;

    // min coordinates

    for (x1 = 0; x1 < x_size; x1++)
    for (y = 0; y < y_size; y++)
    for (z = 0; z < z_size; z++)
        if (is_solid_fast(x1, y, z))
            goto x1_end;
x1_end:

    for (y1 = 0; y1 < y_size; y1++)
    for (x = 0; x < x_size; x++)
    for (z = 0; z < z_size; z++)
        if (is_solid_fast(x, y1, z))
            goto y1_end;
y1_end:

    for (z1 = 0; z1 < z_size; z1++)
    for (x = 0; x < x_size; x++)
    for (y = 0; y < y_size; y++)
        if (is_solid_fast(x, y, z1))
            goto z1_end;
z1_end:

    // max coordinates

    for (x2 = x_size - 1; x2 >= x1; x2--)
    for (y = y1; y < y_size; y++)
    for (z = z1; z < z_size; z++)
        if (is_solid_fast(x2, y, z))
            goto x2_end;
x2_end:

    for (y2 = y_size - 1; y2 >= y1; y2--)
    for (x = x1; x < x_size; x++)
    for (z = z1; z < z_size; z++)
        if (is_solid_fast(x, y2, z))
            goto y2_end;
y2_end:

    for (z2 = z_size - 1; z2 >= z1; z2--)
    for (x = x1; x < x_size; x++)
    for (y = y1; y < y_size; y++)
        if (is_solid_fast(x, y, z2))
            goto z2_end;
z2_end:

    resize(x1, y1, z1, x2 - x1 + 1, y2 - y1 + 1, z2 - z1 + 1);
}

void VoxelFile::rotate()
{
    int new_x = y_size;
    int new_y = x_size;
    int new_z = z_size;

    unsigned char * new_data = new unsigned char[new_x * new_y * new_z];
    for (int x = 0; x < x_size; x++)
    for (int y = 0; y < y_size; y++)
    for (int z = 0; z < z_size; z++) {
        int nx = y_size - y - 1;
        int ny = x;
        int nz = z;
        new_data[nz + ny * new_z + nx * new_z * new_y] = get(x, y, z);
    }
    delete[] data;
    data = new_data;
    int x_off = x_offset;
    int y_off = y_offset;
    x_offset = -y_off - y_size;
    y_offset = x_off;
    x_size = new_x;
    y_size = new_y;
    z_size = new_z;
}

bool VoxelFile::load(const QString & filename)
{
    QFile fp(filename);
    if (!fp.open(QIODevice::ReadOnly))
        return false;
    load_fp(fp);
    fp.close();
    return true;
}

void VoxelFile::load_fp(QFile & fp)
{
    QDataStream stream(&fp);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream >> x_size;
    stream >> y_size;
    stream >> z_size;
    stream >> x_offset;
    stream >> y_offset;
    stream >> z_offset;
    if (data != NULL)
        delete[] data;
    data = new unsigned char[x_size * y_size * z_size];
    stream.readRawData((char*)data, x_size * y_size * z_size);
    stream.skipRawData(256 * 3);

    // reference points
    points.clear();
    quint8 point_count;
    if (stream.atEnd())
        point_count = 0;
    else
        stream >> point_count;
    QString name;
    qint32 x, y, z;
    for (int i = 0; i < point_count; i++) {
        stream >> name;
        stream >> x;
        stream >> y;
        stream >> z;
        points.push_back(ReferencePoint(name, x, y, z));
    }
}

void VoxelFile::save(const QString & filename)
{
    QFile fp(filename);
    if (!fp.open(QIODevice::WriteOnly))
        return;
    save_fp(fp);
    fp.close();
}

void VoxelFile::save_fp(QFile & fp)
{
    QDataStream stream(&fp);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << x_size;
    stream << y_size;
    stream << z_size;
    stream << x_offset;
    stream << y_offset;
    stream << z_offset;
    stream.writeRawData((char*)data, x_size * y_size * z_size);
    stream.writeRawData((char*)global_palette, 256 * 3);
    stream << quint8(points.size());
    ReferencePoints::const_iterator it;
    for (it = points.begin(); it != points.end(); it++) {
        const ReferencePoint & point = *it;
        stream << point.name;
        stream << point.x;
        stream << point.y;
        stream << point.z;
    }
    save_palette();
}

VoxelModel * VoxelFile::get_model()
{
    if (model != NULL)
        return model;

    model = new VoxelModel(this);
    return model;
}

btCompoundShape * VoxelFile::get_shape()
{
    if (shape != NULL)
        return shape;
    btCompoundShape * shape = new btCompoundShape(true);
    const float s = 0.5f;
    static btBoxShape * box_shape = new btBoxShape(btVector3(s, s, s));
    btTransform transform;
    for (int x = 0; x < x_size; x++)
    for (int y = 0; y < y_size; y++)
    for (int z = 0; z < z_size; z++) {
        if (!is_solid(x, y, z))
            continue;
        // ignore if not an exposed block
        if (is_solid(x + 1, y, z) &&
            is_solid(x - 1, y, z) &&
            is_solid(x, y + 1, z) &&
            is_solid(x, y - 1, z) &&
            is_solid(x, y, z + 1) &&
            is_solid(x, y, z - 1))
            continue;

        transform.setIdentity();
        transform.setOrigin(btVector3(x + x_offset + 0.5f,
                                      y + y_offset + 0.5f,
                                      z + z_offset + 0.5f));
        shape->addChildShape(transform, box_shape);
    }
    this->shape = shape;
    return shape;
}

void VoxelFile::reset_shape()
{
    delete shape;
    shape = NULL;
}

void VoxelFile::clone(VoxelFile & other)
{
    x_size = other.x_size;
    y_size = other.y_size;
    z_size = other.z_size;
    x_offset = other.x_offset;
    y_offset = other.y_offset;
    z_offset = other.z_offset;
    points = other.points;
    delete[] data;
    size_t size = x_size * y_size * z_size;
    data = new unsigned char[size];
    memcpy(data, other.data, size);
}
