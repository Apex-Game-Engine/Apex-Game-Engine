import sys
sys.path.append("x:\\ApexGameEngine-Vulkan\\Tools\\bin\\")

import PyApex as apex

filepath = sys.argv[1] if len(sys.argv) > 1 else "ExamplePlane.axmesh"

mesh = apex.import_mesh(filepath)
print(mesh.get_vertex_format())
print(mesh.get_streams())
print(mesh.get_indices())
print(mesh.get_vertices())
