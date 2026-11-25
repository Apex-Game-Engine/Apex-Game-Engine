import sys
sys.path.append("x:\\ApexGameEngine-Vulkan\\Tools\\bin\\")

import traceback

try:
    import PyApex as apex

    positions = [
        -1, -1, 0,
         1, -1, 0,
         1,  1, 0,
        -1,  1, 0,
    ]

    normals = [
        0, 0, 1,
        0, 0, 1,
        0, 0, 1,
        0, 0, 1,
    ]

    uvs = [
        0, 0,
        1, 0,
        1, 1,
        0, 1,
    ]

    indices = [
        0, 1, 2,
        2, 3, 0
    ]

    print("here...")

    mb = apex.MeshBuilder()
    mesh = mb.begin_stream()\
                .add_attribute(apex.POSITION, apex.Format.R32G32B32_SFLOAT)\
             .end_stream()\
             .begin_stream()\
                .add_attribute(apex.NORMAL, apex.Format.R32G32B32_SFLOAT)\
                .add_attribute(apex.TEXCOORD0, apex.Format.R32G32_SFLOAT)\
             .end_stream()\
             .set_data(apex.POSITION, positions)\
             .set_data(apex.NORMAL, normals)\
             .set_data(apex.TEXCOORD0, uvs)\
             .set_indices(indices)\
             .build()
    
    print("Mesh created")
    print(mesh)

    exp = apex.MeshExporter()
    exp.export(mesh, "X:\\Tests\\Assets\\Meshes\\exported\\plane.axmesh")
except Exception as e:
    print("Error : ", e)
    print(traceback.format_exc())
finally:
    print("Ending")


# filepath = sys.argv[1] if len(sys.argv) > 1 else "ExamplePlane.axmesh"

# mesh.export(filepath)
