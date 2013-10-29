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

"""
High-level byte read/writing and pack/unpacking from files and data
"""

from cStringIO import StringIO
import struct

INT8 = struct.Struct('<b')
UINT8 = struct.Struct('<B')
INT32 = struct.Struct('<i')
UINT32 = struct.Struct('<I')

class ByteWriter(object):
    def __init__(self, fp=None):
        if fp is None:
            fp = StringIO()
        self.fp = fp
        self.write = fp.write
        self.seek = fp.seek
        self.close = fp.close

    def write_struct(self, f, value):
        self.write(f.pack(value))

    def write_int8(self, value):
        self.write_struct(INT8, value)

    def write_uint8(self, value):
        self.write_struct(UINT8, value)

    def write_int32(self, value):
        self.write_struct(INT32, value)

    def write_uint32(self, value):
        self.write_struct(UINT32, value)

    def write_string(self, value):
        self.write(value)
        self.write_uint8(0)

class ByteReader(object):
    def __init__(self, data=None, fp=None):
        if data is not None:
            fp = StringIO(data)
        if fp is None:
            raise ValueError('need to specify data or fp')
        self.fp = fp
        self.read = fp.read
        self.seek = fp.seek
        self.close = fp.close

    def read_struct(self, f):
        value = f.unpack(self.read(f.size))
        if len(value) == 1:
            return value[0]
        return value

    def read_int8(self):
        return self.read_struct(INT8)

    def read_uint8(self):
        return self.read_struct(UINT8)

    def read_int32(self):
        return self.read_struct(INT32)

    def read_uint32(self):
        return self.read_struct(UINT32)

    def read_string(self):
        value = ''
        while True:
            c = self.read(1)
            if c in ('\x00', ''):
                break
            value += c
        return value
