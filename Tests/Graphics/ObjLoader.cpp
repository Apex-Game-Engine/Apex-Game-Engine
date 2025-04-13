#include "ObjLoader.h"
#include "Core/Asserts.h"
#include "Core/Files.h"
#include "Math/Vector3.h"
#include "Memory/AxHandle.h"
#include "Memory/MemoryManager.h"
#include "Memory/MemoryManagerImpl.h"

void ReadFileCallback(void* ctx, const char* filename, int is_mtl, const char* obj_filename, char** buf, size_t* len)
{
	apex::File file = apex::File::OpenExisting(filename);
	apex::AxArray<char>& contents = *static_cast<apex::AxArray<char>*>(ctx);
	file.Read(contents);
	*buf = contents.dataMutable();
	*len = contents.size();
}

void apex::ObjLoader::LoadFile(const char* filename)
{
	tinyobj_attrib_t attrib;
	tinyobj_shape_t* shapes;
	tinyobj_material_t* materials;
	size_t numShapes;
	size_t numMaterials;

	AxArray<char> contents;

	tinyobj_attrib_init(&attrib);

	int result = tinyobj_parse_obj(&attrib, &shapes, &numShapes, &materials, &numMaterials, filename, ReadFileCallback, &contents, TINYOBJ_FLAG_TRIANGULATE);
	axVerifyFmt(TINYOBJ_SUCCESS == result, "tinyobj_parse_obj failed with error code: {}", result);

	size_t numTriangles = attrib.num_face_num_verts;
	size_t stride = 3 + 3 + 2;
	size_t faceOffset = 0;

	m_vertices.resize(numTriangles * stride * 3);

	for (size_t i = 0; i < attrib.num_face_num_verts; i++)
	{
		for (int f = 0; f < attrib.face_num_verts[i] / 3; f++)
		{
			math::Vector3 v[3];
			math::Vector3 n[3];
			math::Vector2 uv[3];

			const tinyobj_vertex_index_t idx0 = attrib.faces[faceOffset + 3 * f + 0];
			const tinyobj_vertex_index_t idx1 = attrib.faces[faceOffset + 3 * f + 1];
			const tinyobj_vertex_index_t idx2 = attrib.faces[faceOffset + 3 * f + 2];

			// Positions
			{
				int f0 = idx0.v_idx;
				int f1 = idx1.v_idx;
				int f2 = idx2.v_idx;

				for (size_t k = 0; k < 3; k++)
				{
					v[0][k] = attrib.vertices[3 * (size_t)f0 + k];
					v[1][k] = attrib.vertices[3 * (size_t)f1 + k];
					v[2][k] = attrib.vertices[3 * (size_t)f2 + k];
				}
			}

			// Normals
			if (attrib.num_normals > 0)
			{
				int f0 = idx0.vn_idx;
				int f1 = idx1.vn_idx;
				int f2 = idx2.vn_idx;

				if (f0 >= 0 && f1 >= 0 && f2 >= 0)
				{
					axAssert((size_t)f0 < attrib.num_normals);
					axAssert((size_t)f1 < attrib.num_normals);
					axAssert((size_t)f2 < attrib.num_normals);

					for (size_t k = 0; k < 3; k++)
					{
						n[0][k] = attrib.normals[3 * (size_t)f0 + k];
						n[1][k] = attrib.normals[3 * (size_t)f1 + k];
						n[2][k] = attrib.normals[3 * (size_t)f2 + k];
					}
				}
				else
				{
					n[0] = math::cross(v[2] - v[0], v[1] - v[0]);
					n[1] = n[0];
					n[2] = n[0];
				}
			}
			else
			{
				n[0] = math::cross(v[2] - v[0], v[1] - v[0]);
				n[1] = n[0];
				n[2] = n[0];
			}

			// Texture Coordinates
			if (attrib.num_texcoords > 0)
			{
				int f0 = idx0.vt_idx;
				int f1 = idx1.vt_idx;
				int f2 = idx2.vt_idx;

				for (size_t k = 0; k < 2; k++)
				{
					uv[0][k] = attrib.texcoords[2 * (size_t)f0 + k];
					uv[1][k] = attrib.texcoords[2 * (size_t)f1 + k];
					uv[2][k] = attrib.texcoords[2 * (size_t)f2 + k];
				}
			}

			for (size_t k = 0; k < 3; k++)
			{
				const size_t triangleOffset = (3 * i + k) * stride;

				m_vertices[triangleOffset + 0] = v[k][0];
				m_vertices[triangleOffset + 1] = v[k][1];
				m_vertices[triangleOffset + 2] = v[k][2];
				m_vertices[triangleOffset + 3] = n[k][0];
				m_vertices[triangleOffset + 4] = n[k][1];
				m_vertices[triangleOffset + 5] = n[k][2];
				m_vertices[triangleOffset + 6] = uv[k][0];
				m_vertices[triangleOffset + 7] = uv[k][1];
			}
		}
		faceOffset += (size_t)attrib.face_num_verts[i];
	}

	m_indices.resize(numTriangles * 3);

	for (size_t i = 0; i < numTriangles * 3; i++)
	{
		m_indices[i] = i;
	}

	tinyobj_attrib_free(&attrib);
	if (shapes) tinyobj_shapes_free(shapes, numShapes);
	if (materials) tinyobj_materials_free(materials, numMaterials);
}

apex::AxArrayRef<float> apex::ObjLoader::GetVertexBufferData()
{
	return make_array_ref(m_vertices);
}

apex::AxArrayRef<unsigned> apex::ObjLoader::GetIndexBufferData()
{
	return make_array_ref(m_indices);
}

apex::AxArrayRef<const float> apex::ObjLoader::GetVertexBufferData() const
{
	return make_array_ref(m_vertices);
}

apex::AxArrayRef<const unsigned> apex::ObjLoader::GetIndexBufferData() const
{
	return make_array_ref(m_indices);
}


extern "C" {

	void* apex_malloc(size_t size)
	{
		return apex::AxHandle(size).getAs<void>();
	}

	void* apex_calloc(size_t num, size_t size)
	{
		const size_t totalSize = num * size;
		void* mem = apex::AxHandle(totalSize).getAs<void>();
		return memset(mem, 0, totalSize);
	}

	void* apex_realloc_sized(void* mem, size_t old_size, size_t new_size)
	{
		if (mem != nullptr)
		{
			auto& pool = apex::mem::MemoryManager::getImplInstance().getMemoryPoolFromPointer(mem);
			old_size = old_size ? old_size : pool.getBlockSize();
			if (pool.getBlockSize() > new_size)
			{
				return mem;
			}
		}
		void* newMem = apex::AxHandle(new_size).getAs<void>();
		memcpy_s(newMem, new_size, mem, old_size);
		return newMem;
	}

	void* apex_realloc(void* mem, size_t new_size)
	{
		return apex_realloc_sized(mem, 0, new_size);
	}

	void apex_free(void* mem)
	{
		axStrongAssert(apex::mem::MemoryManager::canFree(mem));
		apex::mem::MemoryManager::free(mem);
	}

}

#define TINYOBJ_LOADER_C_IMPLEMENTATION
#define TINYOBJ_MALLOC apex_malloc
#define TINYOBJ_REALLOC apex_realloc
#define TINYOBJ_CALLOC apex_calloc
#define TINYOBJ_FREE apex_free
#define TINYOBJ_REALLOC_SIZED apex_realloc_sized
#include <tinyobj_loader_c.h>
