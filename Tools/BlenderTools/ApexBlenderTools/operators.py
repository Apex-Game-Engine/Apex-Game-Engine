import bpy
import bpy_extras
import bmesh

def get_preprocessed_mesh(obj):
	bm = bmesh.new()
	bm.from_mesh(obj.data)
	bmesh.ops.triangulate(bm, faces=bm.faces)
	tmp_mesh = bpy.data.meshes.new('tmp_mesh')
	bm.to_mesh(tmp_mesh)
	bm.free()
	tmp_mesh.transform(obj.matrix_world)
	tmp_mesh.calc_tangents()
	tmp_mesh.calc_loop_triangles()
	return tmp_mesh

def export_axmesh(context, filepath):
	from . import PyApex as apex
	obj = context.object

	mesh = get_preprocessed_mesh(obj)

	uv_layer = mesh.uv_layers.active.data

	positions = []
	normals = []
	tangents = []
	bitangents = []
	uvs = []
	triangle_count = 0
	for loop in mesh.loops:
		v = mesh.vertices[loop.vertex_index]
		# position
		positions.extend([v.co.x, v.co.y, v.co.z])
		# normal
		normals.extend([loop.normal.x, loop.normal.y, loop.normal.z])
		# tangent
		tangents.extend([loop.tangent.x, loop.tangent.y, loop.tangent.z])
		# bitangent
		bitangent = loop.bitangent_sign * loop.normal.cross(loop.tangent)
		bitangents.extend([bitangent.x, bitangent.y, bitangent.z])
		# texcoord
		uvs.extend([uv_layer[loop.index].uv.x, uv_layer[loop.index].uv.y])
		# triangle index
		triangle_count += 1

	mesh = apex.MeshBuilder()\
				.begin_stream()\
					.add_attribute(apex.POSITION, apex.Format.R32G32B32_SFLOAT)\
					.add_attribute(apex.TEXCOORD0, apex.Format.R32G32_SFLOAT)\
				.end_stream()\
				.begin_stream()\
					.add_attribute(apex.NORMAL, apex.Format.R32G32B32_SFLOAT)\
					.add_attribute(apex.TANGENT, apex.Format.R32G32B32_SFLOAT)\
					.add_attribute(apex.BITANGENT, apex.Format.R32G32B32_SFLOAT)\
				.end_stream()\
				.set_data(apex.POSITION, positions)\
				.set_data(apex.TEXCOORD0, uvs)\
				.set_data(apex.NORMAL, normals)\
				.set_data(apex.TANGENT, tangents)\
				.set_data(apex.BITANGENT, bitangents)\
				.set_indices(list(range(0, triangle_count)))\
				.build()

	apex.MeshExporter().export(mesh, filepath)
	print(f"Exported mesh to {filepath}")
	return {'FINISHED'}

class Apex_OT_ExportMesh(bpy.types.Operator, bpy_extras.io_utils.ExportHelper):
	bl_idname = "apex.export_mesh"
	bl_label = "Export Apex Mesh"
	bl_description = "Exports mesh that can be imported in Apex Game Engine"


	filename_ext = ".axmesh"

	filter_glob: bpy.props.StringProperty(
		default="*.axmesh",
		options={'HIDDEN'},
		maxlen=255,
	)

	def execute(self, context):
		return export_axmesh(context, self.filepath)


def menu_func_export(self, context):
	self.layout.operator(Apex_OT_ExportMesh.bl_idname, text="Apex Mesh (.axmesh)")

def register():
	bpy.utils.register_class(Apex_OT_ExportMesh)
	bpy.types.TOPBAR_MT_file_export.append(menu_func_export)

def unregister():
	bpy.utils.unregister_class(Apex_OT_ExportMesh)
	bpy.types.TOPBAR_MT_file_export.remove(menu_func_export)
