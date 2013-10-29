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

from byteio import ByteReader
import os

class ReferencePoint(object):
    def __init__(self, reader):
        self.name = reader.read_string()
        self.x = reader.read_int32()
        self.y = reader.read_int32()
        self.z = reader.read_int32()

class Palette(object):
    def __init__(self, reader, has_names=True):
        self.palette = []
        for _ in xrange(256):
            r = reader.read_uint8()
            g = reader.read_uint8()
            b = reader.read_uint8()
            self.palette.append((r, g, b))

        if not has_names:
            self.names = None
            return

        self.names = []
        for _ in xrange(256):
            self.names.append(reader.read_string())

    def write(self, writer):
        for (r, g, b) in self.palette:
            writer.write_uint8(r)
            writer.write_uint8(g)
            writer.write_uint8(b)

        if not self.names:
            return

        for name in self.names:
            writer.write_string(name)

PALETTE_FILE = os.path.join(os.path.dirname(__file__), '..', 'palette.dat')

def read_global_palette(filename=None):
    if filename is None:
        filename = PALETTE_FILE
    data = open(filename, 'rb').read()
    reader = ByteReader(data)
    return Palette(reader)

class VoxelModel(object):
    def __init__(self, reader):
        self.x_size = reader.read_uint32()
        self.y_size = reader.read_uint32()
        self.z_size = reader.read_uint32()
        self.x_offset = reader.read_int32()
        self.y_offset = reader.read_int32()
        self.z_offset = reader.read_int32()

        self.blocks = {}
        self.palette = []

        for x in xrange(self.x_size):
            for y in xrange(self.y_size):
                for z in xrange(self.z_size):
                    v = reader.read_uint8()
                    if v == 255:
                        continue
                    self.blocks[(x, y, z)] = v

        self.palette = Palette(reader, False).palette

        self.points = []
        for _ in xrange(reader.read_uint8()):
            self.points.append(ReferencePoint(reader))

    def is_solid(self, x, y, z):
        return self.blocks.get((x, y, z), None) is not None