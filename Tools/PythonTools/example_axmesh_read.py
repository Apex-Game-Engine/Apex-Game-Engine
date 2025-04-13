import sys
sys.path.append("x:\\ApexGameEngine-Vulkan\\Tools\\bin\\")

import PyModel

filepath = sys.argv[1] if len(sys.argv) > 1 else "ExamplePlane.axmesh"

imesh = PyModel.import_mesh(filepath)
print(imesh.stride)
print(imesh.streams)
print(imesh.attributes)
print(imesh.indices)
print(imesh.vertices)
