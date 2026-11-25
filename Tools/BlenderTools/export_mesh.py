import sys
apex_path = "X:\\ApexGameEngine-Vulkan"
sys.path.append(f"{apex_path}\\Tools\\BlenderTools\\")

import bpy
from ApexBlenderTools import register

if sys.argv[0] == "blender":
	sources = sys.argv[4:]
else:
	sources = sys.argv[2:]

if bpy.ops.apex is None or bpy.ops.apex.export_mesh is None:
	register()

bpy.ops.object.select_all(action='SELECT')
bpy.ops.object.delete(use_global=False)

for source in sources:
	if source.endswith(".obj"):
		bpy.ops.wm.obj_import('EXEC_DEFAULT', filepath=source)
	if source.endswith(".fbx"):
		bpy.ops.import_scene.fbx('EXEC_DEFAULT', filepath=source)
			
for obj in bpy.data.objects:
	obj.select_set(True)
	if type(obj.data) == bpy.types.Mesh:
		objname = obj.name[0:obj.name.find(":")]
		bpy.context.view_layer.objects.active = obj
		bpy.ops.apex.export_mesh('EXEC_DEFAULT', filepath="x:\\Tests\\Assets\\Meshes\\exported\\"+objname+".axmesh")
