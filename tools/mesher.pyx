# distutils: language = c++

from libcpp.vector cimport vector

cdef extern from "mesher_c.cpp":
    cdef struct Vertex:
        int x, y, z
        int nx, ny, nz

    cdef cppclass MesherModel:
        MesherModel()
        void init(char * data, int x, int y, int z)

    cdef cppclass Mesher:
        void build_vertices(MesherModel * model, unsigned char p)
        vector[Vertex] vertices
        vector[int] indices

def mesh(in_model):
    cdef Mesher mesher
    cdef MesherModel model
    model.init(in_model.data, in_model.x_size, in_model.y_size,
               in_model.z_size)
    cdef set colors = set(in_model.blocks.values())

    cdef unsigned char v

    cdef dict sets = {}
    cdef Vertex * vertex
    cdef unsigned int i

    cdef list points

    for v in colors:
        points = []
        mesher.build_vertices(&model, v)
        for i in range(mesher.indices.size()):
            vertex = &mesher.vertices[mesher.indices[i]]
            points.append((vertex.x, vertex.y, vertex.z,
                           vertex.nx, vertex.ny, vertex.nz))
        sets[v] = points
    return sets
