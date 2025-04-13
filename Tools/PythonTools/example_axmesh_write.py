import sys
sys.path.append("x:\\ApexGameEngine-Vulkan\\Tools\\bin\\")

from PyModel import Mesh, MeshAttributeType, MeshStream

mesh = Mesh()
mesh.vertexCount = 4
mesh.attributes = [ MeshAttributeType.Position, MeshAttributeType.Normal, MeshAttributeType.TexCoords0 ]
mesh.new_stream(0, 3)
mesh.indices = [ 0, 1, 2, 2, 3, 0 ]
vertices = []

positions = [
    (-1, -1, 0),
    ( 1, -1, 0),
    ( 1,  1, 0),
    (-1,  1, 0),
]

normals = [
    (0, 0, 1),
    (0, 0, 1),
    (0, 0, 1),
    (0, 0, 1),
]

uvs = [
    (0, 0),
    (1, 0),
    (1, 1),
    (0, 1),
]

for pos, nor, uv in zip(positions, normals, uvs):
    vertices.extend(pos)
    vertices.extend(nor)
    vertices.extend(uv)

mesh.vertices = vertices

print(mesh.stride)
print(mesh.streams)
print(mesh.attributes)
print(mesh.indices)
print(mesh.vertices)


filepath = sys.argv[1] if len(sys.argv) > 1 else "ExamplePlane.axmesh"

mesh.export(filepath)
