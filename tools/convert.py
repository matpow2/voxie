# Copyright (c) 2013 Mathias Kaerlev
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

import byteio
from voxmodel import VoxelModel
from collada import *
import numpy
import shutil
import math
import argparse

def swap_coord(x, y, z):
    return x, z, y

class MaterialSet(object):
    def __init__(self, name, mesh, color):
        self.postfix = name
        r, g, b = color
        color = (r / 255.0, g / 255.0, b / 255.0)
        self.effect = material.Effect("effect%s" % name, [], "phong", 
                                      diffuse=color, specular=(0, 1, 0))
        name = "material%s" % name
        self.mat = material.Material(name, name, self.effect)
        self.indices = []
        self.vertices = []
        self.normals = []
        self.mesh = mesh

    def add_quad(self, nx, ny, nz, x1, y1, z1, x2, y2, z2,
                 x3, y3, z3, x4, y4, z4):
        vert_index = len(self.vertices) / 3
        normal = (nx, ny, nz)
        try:
            normal_index = self.normals.index(normal)
        except ValueError:
            self.normals.append(normal)
            normal_index = len(self.normals) - 1
        self.vertices.extend([x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4])
        self.indices.extend([vert_index, normal_index,
                             vert_index + 1, normal_index,
                             vert_index + 2, normal_index,
                             vert_index + 2, normal_index,
                             vert_index + 3, normal_index,
                             vert_index, normal_index])

    def apply(self):
        self.mesh.effects.append(self.effect)
        self.mesh.materials.append(self.mat)

        postfix = self.postfix

        vert_name = 'vertices%s' % postfix
        vert_src = source.FloatSource(vert_name, numpy.array(self.vertices),
                                      ('X', 'Y', 'Z'))
        normal_name = 'normals%s' % postfix
        normal_src = source.FloatSource(normal_name, numpy.array(self.normals),
                                        ('X', 'Y', 'Z'))
        name = "geometry%s" % postfix
        geom = geometry.Geometry(self.mesh, name, name, [vert_src, normal_src])
        input_list = source.InputList()
        input_list.addInput(0, 'VERTEX', "#" + vert_name)
        input_list.addInput(1, 'NORMAL', "#" + normal_name)
        materialref = 'material%s' % postfix
        triset = geom.createTriangleSet(numpy.array(self.indices), input_list,
                                        materialref)
        geom.primitives.append(triset)
        self.mesh.geometries.append(geom)
        matnode = scene.MaterialNode(materialref, self.mat, inputs=[])
        return scene.GeometryNode(geom, [matnode])

PALETTE_FILE = os.path.join(os.path.dirname(__file__), '..', 'palette.dat')
OUT_PATH = r'C:\Users\Mathias\Documents\The Cosmonaut\Assets\Models'
META_PATH = r'C:\Users\Mathias\Documents\The Cosmonaut\Assets\Meta'

used_palette_entries = set()

def convert_file(filename, out_dir):
    basename = os.path.basename(filename)
    name = os.path.splitext(basename)[0]
    out_path = os.path.join(out_dir, '%s.dae' % name)
    if not is_file_changed(filename, out_path):
        print 'Skipping', basename
        return False

    v = open(filename, 'rb').read()
    reader = byteio.ByteReader(v)
    f = VoxelModel(reader)
    mesh = Collada()

    scale = 1.0
    gx = gy = gz = 0.0

    sets = {}

    x_off, y_off, z_off = swap_coord(f.x_offset, f.y_offset, f.z_offset)

    for (x, y, z), v in f.blocks.iteritems():
        v = f.blocks.get((x, y, z), None)
        if v is None:
            continue
        used_palette_entries.add(v)
        color = f.palette[v]

        xx, yy, zz = swap_coord(x, y, z)
        x1 = (xx + x_off) * scale + gx
        y1 = (yy + y_off) * scale + gy
        z1 = (zz + z_off) * scale + gz
        x2 = (xx + x_off + 1) * scale + gx
        y2 = (yy + y_off + 1) * scale + gy
        z2 = (zz + z_off + 1) * scale + gz

        try:
            s = sets[color]
        except KeyError:
            s = MaterialSet(v, mesh, color)
            sets[color] = s

        # Left face
        if not f.is_solid(x, y, z+1):
            s.add_quad(0.0, 1.0, 0.0,
                       x1, y2, z1,
                       x1, y2, z2,
                       x2, y2, z2,
                       x2, y2, z1)

        # Right face
        if not f.is_solid(x, y, z-1):
            s.add_quad(0.0, -1.0, 0.0,
                       x1, y1, z1,
                       x2, y1, z1,
                       x2, y1, z2,
                       x1, y1, z2)

        # Top face
        if not f.is_solid(x, y+1, z):
            s.add_quad(0.0, 0.0, 1.0,
                       x1, y1, z2,
                       x2, y1, z2,
                       x2, y2, z2,
                       x1, y2, z2)

        # Bottom face
        if not f.is_solid(x, y-1, z):
            s.add_quad(0.0, 0.0, -1.0,
                       x1, y1, z1,
                       x1, y2, z1,
                       x2, y2, z1,
                       x2, y1, z1)

        # Right face
        if not f.is_solid(x + 1, y, z):
            s.add_quad(1.0, 0.0, 0.0,
                       x2, y1, z1,
                       x2, y2, z1,
                       x2, y2, z2,
                       x2, y1, z2)

        # Left Face
        if not f.is_solid(x - 1, y, z):
            s.add_quad(-1.0, 0.0, 0.0,
                       x1, y1, z1,
                       x1, y1, z2,
                       x1, y2, z2,
                       x1, y2, z1)

    nodes = []
    for s in sets.values():
        nodes.append(s.apply())
    node = scene.Node("node0", children=nodes)
    myscene = scene.Scene("scene", [node])
    mesh.scenes.append(myscene)
    mesh.scene = myscene

    mesh.write(out_path)
    shutil.copystat(filename, out_path)
    print 'Converted', basename
    return True

def convert_meta(src, out_dir):
    if out_dir is None:
        return
    name = os.path.splitext(os.path.basename(src))[0]
    meta = os.path.join(out_dir, '%s.bytes' % name)
    shutil.copy2(src, meta)

def is_file_changed(src, dst):
    if not os.path.isfile(dst):
        return True
    diff = os.stat(src).st_mtime - os.stat(dst).st_mtime
    return math.fabs(diff) > 0.01

def main():
    parser = argparse.ArgumentParser(description='Converts .vxi to '
                                                 'COLLADA .dae models')
    parser.add_argument('input', metavar='input', type=str,
                        help='input file or directory')
    parser.add_argument('out_dir', metavar='out_dir', type=str,
                        help='output directory')
    parser.add_argument('--meta', metavar='meta_dir', type=str, default=None,
                        help='metafile directory for Unity3D')
    args = parser.parse_args()

    if os.path.isfile(args.input):
        convert_file(args.input, args.out_dir)
        convert_meta(args.input, args.meta_dir)
    else:
        for root, dirs, files in os.walk(args.input):
            for f in files:
                if not f.endswith('.vxi'):
                    continue
                path = os.path.join(root, f)
                if convert_file(path, args.out_dir):
                    convert_meta(path, args.meta)

if __name__ == '__main__':
    main()
