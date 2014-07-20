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
from voxmodel import VoxelModel, read_global_palette
from collada import *
import numpy
import shutil
import math
import argparse
import mesher

def swap_coord(x, y, z, f):
    x = -x
    return x, z, y

class MaterialSet(object):
    def __init__(self, set_name, index, mesh, color, vertices, normals):
        self.vertices = vertices
        self.normals = normals
        self.postfix = index
        r, g, b = color
        color = (r / 255.0, g / 255.0, b / 255.0)
        self.effect = material.Effect("effect%s" % index, [], "phong",
                                      diffuse=color, specular=(0, 1, 0))
        self.set_name = set_name
        self.mat = material.Material(set_name, set_name, self.effect)
        self.indices = []
        self.mesh = mesh

    def add_vertex(self, x, y, z, nx, ny, nz):
        vertex = (x, y, z)
        try:
            vert_index = self.vertices.index(vertex)
        except ValueError:
            vert_index = len(self.vertices)
            self.vertices.append(vertex)

        normal = (nx, ny, nz)
        try:
            normal_index = self.normals.index(normal)
        except ValueError:
            normal_index = len(self.normals)
            self.normals.append(normal)

        self.indices.extend([vert_index, normal_index])


def convert_file(filename, out_dir, palette, force):
    basename = os.path.basename(filename)
    name = os.path.splitext(basename)[0]
    out_path = os.path.join(out_dir, '%s.dae' % name)
    if not force and not is_file_changed(filename, out_path):
        print('Skipping', basename)
        return False

    v = open(filename, 'rb').read()
    reader = byteio.ByteReader(v)
    f = VoxelModel(reader)
    mesh = Collada()

    scale = 1.0

    sets = {}

    x_off, y_off, z_off = swap_coord(f.x_offset, f.y_offset, f.z_offset, f)

    vertices = []
    normals = []

    out = mesher.mesh(f)

    for v, points in out.items():
        set_name = '%s - %s' % (v, palette.names[v].replace('/', ' or '))
        color = palette.palette[v]
        s = MaterialSet(set_name, v, mesh, color, vertices, normals)
        sets[color] = s

        for point in reversed(points):
            x, y, z, nx, ny, nz = point
            xx, yy, zz = swap_coord(x, y, z, f)
            nx, ny, nz = swap_coord(nx, ny, nz, f)
            s.add_vertex(xx + x_off, yy + y_off, zz + z_off, nx, ny, nz)

    vert_name = 'vertices'
    vert_src = source.FloatSource(vert_name, numpy.array(vertices),
                                  ('X', 'Y', 'Z'))
    normal_name = 'normals'
    normal_src = source.FloatSource(normal_name,
                                    numpy.array(normals),
                                    ('X', 'Y', 'Z'))
    geom_name = "geometry"
    geom = geometry.Geometry(mesh, name, name, [vert_src, normal_src])

    matnodes = []
    for s in list(sets.values()):
        mesh.effects.append(s.effect)
        mesh.materials.append(s.mat)

        postfix = s.postfix
        input_list = source.InputList()
        input_list.addInput(0, 'VERTEX', "#" + vert_name)
        input_list.addInput(1, 'NORMAL', "#" + normal_name)
        materialref = 'material%s' % postfix
        triset = geom.createTriangleSet(numpy.array(s.indices), input_list,
                                        materialref)
        geom.primitives.append(triset)
        mesh.geometries.append(geom)
        matnodes.append(scene.MaterialNode(materialref, s.mat, inputs=[]))



    geom_node = scene.GeometryNode(geom, matnodes)
    geom_parent_node = scene.Node(name, children=[geom_node])
    nodes = [geom_parent_node]

    for point in f.points:
        x, y, z = swap_coord(point.x, point.y, point.z, f)
        transform = scene.TranslateTransform(x, y, z)
        ref_node = scene.Node(point.name, transforms=[transform])
        nodes.append(ref_node)

    myscene = scene.Scene("scene", nodes)
    mesh.scenes.append(myscene)
    mesh.scene = myscene
    mesh.write(out_path)

    shutil.copystat(filename, out_path)
    print('Converted', basename)
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
    parser.add_argument('--force', action='store_const',
                        const=True, default=False,
                        help='force conversion even if file was not updated')
    args = parser.parse_args()

    palette = read_global_palette()

    if os.path.isfile(args.input):
        convert_file(args.input, args.out_dir, palette)
        convert_meta(args.input, args.meta_dir)
    else:
        for root, dirs, files in os.walk(args.input):
            for f in files:
                if not f.endswith('.vxi'):
                    continue
                path = os.path.join(root, f)
                if convert_file(path, args.out_dir, palette, args.force):
                    convert_meta(path, args.meta)

if __name__ == '__main__':
    main()
