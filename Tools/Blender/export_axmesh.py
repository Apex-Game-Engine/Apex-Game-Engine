import sys
sys.path.append("X:\\ApexGameEngine-Vulkan\\Tools\\bin\\")

import ApexModel
import bpy
import bmesh

def preprocess_mesh(mesh):
    bm = bmesh.new()
    bm.from_mesh(mesh)
    bmesh.ops.triangulate(bm, faces=bm.faces)
    bm.to_mesh(mesh)
    bm.free()

def gather_mesh_data(mesh):

    mesh.calc_normals_split()
    mesh.calc_tangents()
    mesh.calc_loop_triangles()

    uv_layer = mesh.uv_layers.active.data

    vertices = []
    indices = []
    
    curIndex = 0

    for loop in mesh.loops:
        v = mesh.vertices[loop.vertex_index]
        # position
        vertices.append(v.co.x)
        vertices.append(v.co.y)
        vertices.append(v.co.z)
        # normal
        vertices.append(loop.normal.x)
        vertices.append(loop.normal.y)
        vertices.append(loop.normal.z)
        # texcoord
        vertices.append(uv_layer[loop.index].uv.x)
        vertices.append(uv_layer[loop.index].uv.y)

        indices.append(curIndex)
        curIndex += 1

    return vertices, indices


object = bpy.context.object
mesh = object.data
mesh.transform(object.matrix_basis)
preprocess_mesh(mesh)
vertices, indices = gather_mesh_data(mesh)

axmesh = ApexModel.Mesh()
axmesh.vertexCount = len(indices)
axmesh.attributes = [ ApexModel.Position, ApexModel.Normal, ApexModel.TexCoords0 ]
axmesh.new_stream(0, 3)
axmesh.vertices = vertices
axmesh.indices = indices

axmesh.export("X:\\Tests\\Monkey.axmesh")
