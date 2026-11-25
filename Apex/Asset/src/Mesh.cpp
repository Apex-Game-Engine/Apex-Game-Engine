#include "Asset/Mesh.h"
#include "Core/Asserts.h"
#include "Core/Files.h"
#include "String/AxStream.h"

constexpr static apex::u32 MAGIC = 'APEX';
constexpr static apex::u32 SIGNATURE_MESH = 'MESH'; // Mesh
constexpr static apex::u32 SIGNATURE_MATL = 'MATL'; // Material
constexpr static apex::u32 SIGNATURE_SKEL = 'SKEL'; // Skeleton
constexpr static apex::u32 SIGNATURE_ANIM = 'ANIM'; // Animation

namespace apex::asset {

	//namespace detail
	//{
	//	const PFN_MeshLoaderUserAllocate defaultAlloc = mem::GlobalMemoryOperators::OperatorNew;
	//	const PFN_MeshLoaderUserFree defaultFree = mem::GlobalMemoryOperators::OperatorDelete;

	//	PFN_MeshLoaderUserAllocate memAlloc = defaultAlloc;
	//	PFN_MeshLoaderUserFree memFree = defaultFree;
	//}

	struct MeshData
	{
		gfx::VertexFormat vertexFormat;
		u32 vertexCount;
		u32 indexCount;
		void *pVertices;
		u32  *pIndices;
	};

	static char* ReadFileContents(const char* filename, size_t* out_size)
	{
		File file = File::OpenExisting(filename);
		size_t size = file.GetSize();
		char* buffer = apex_new char[size];
		file.Read(buffer, size);
		*out_size = size;
		return buffer;
	}

	static size_t WriteFileContents(const char* filename, char* buffer, size_t size)
	{
		File file = File::CreateOrOpen(filename);
		const size_t sizeWritten = file.Write(buffer, size);
		return sizeWritten;
	}

	ReadOnlyMeshData::~ReadOnlyMeshData()
	{
		if (m_data == nullptr || (m_flags & NEEDS_DELETION_BIT) == 0)
			return;
		if (m_flags & SEPARATE_ALLOCATIONS_BIT)
		{
			delete[] static_cast<u8*>(m_data->pVertices);
			delete[] m_data->pIndices;
		}
		if (m_flags & HAS_HEADER_BIT)
			delete[] (reinterpret_cast<char*>(m_data) - sizeof(MAGIC) - sizeof(SIGNATURE_MESH));
		else
			delete[] reinterpret_cast<char*>(m_data);
	}

	ReadOnlyMeshData::ReadOnlyMeshData(ReadOnlyMeshData&& other) noexcept
		: m_data(other.m_data), m_flags(0)
	{
		other.m_data = nullptr;
	}

	ReadOnlyMeshData& ReadOnlyMeshData::operator=(ReadOnlyMeshData&& other) noexcept
	{
		m_data = other.m_data;
		other.m_data = nullptr;
		return *this;
	}

	ReadOnlyMeshStream ReadOnlyMeshData::GetStream(u32 index) const
	{
		return { m_data, index };
	}

	gfx::VertexFormat ReadOnlyMeshData::GetVertexFormat() const
	{
		return m_data->vertexFormat;
	}

	u32 ReadOnlyMeshData::GetIndexCount() const
	{
		return m_data->indexCount;
	}

	u32* ReadOnlyMeshData::GetIndices() const
	{
		return m_data->pIndices;
	}

	u32 ReadOnlyMeshData::GetVertexCount() const
	{
		return m_data->vertexCount;
	}

	void* ReadOnlyMeshData::GetVertices() const
	{
		return m_data->pVertices;
	}

	ReadOnlyMeshStream::ReadOnlyMeshStream(MeshData* data, u32 stream_index)
		: m_data(data), m_streamIndex(stream_index)
	{
		m_data->vertexFormat.RetrieveStreamAtIndex(m_streamIndex, m_offsetAndStride);
	}

#if APEX_TOOL
	ReadOnlyMeshData MeshDataBuilder::Build(gfx::VertexFormat vertex_format, u32 size, u8* vertices, u32 count, u32* indices) const
	{
		MeshData* data = apex_new MeshData;
		data->vertexFormat = vertex_format;
		data->vertexCount = size / data->vertexFormat.CalcVertexStride();
		data->indexCount = count;
		data->pVertices = apex_new u8[size];
		data->pIndices = apex_new u32[count];
		apex::memcpy_s<u8>(data->pVertices, size, vertices, size);
		apex::memcpy_s<u32>(data->pIndices, count, indices, count);
		return { data, ReadOnlyMeshData::NEEDS_DELETION_BIT | ReadOnlyMeshData::SEPARATE_ALLOCATIONS_BIT };
	}
#endif // APEX_TOOL

	/*void MeshLoader::SetUserMemoryCallbacks(PFN_MeshLoaderUserAllocate allocate_fn, PFN_MeshLoaderUserFree free_fn)
	{
		detail::memAlloc = allocate_fn ? allocate_fn : detail::defaultAlloc;
		detail::memFree = free_fn ? free_fn : detail::defaultFree;
	}*/

	APEX_EXPORT ReadOnlyMeshData APEX_API MeshLoader::ReadFromFile(const char* filename, MeshExporterFlags flags)
	{
		size_t size;
		char* buffer = ReadFileContents(filename, &size);
		axAssert(buffer && size);

		AxStreamReader reader { buffer, size };

		if (const u32 magic = *reader.ReadObject<u32>(); !axVerify(magic == MAGIC))
		{
			delete[] buffer;
			return { nullptr, 0 };
		}

		if (const u32 signature = *reader.ReadObject<u32>(); !axVerify(signature == SIGNATURE_MESH))
		{
			delete[] buffer;
			return { nullptr, 0 };
		}

		MeshData* outMesh = reader.ReadObject<MeshData>();
		const u32 vertexStride = outMesh->vertexFormat.CalcVertexStride();
		outMesh->pIndices = reader.ReadArray<u32>(outMesh->indexCount);
		outMesh->pVertices = reader.ReadArray<char>((size_t)outMesh->vertexCount * vertexStride);

		return { outMesh, ReadOnlyMeshData::NEEDS_DELETION_BIT | ReadOnlyMeshData::HAS_HEADER_BIT };
	}

	APEX_EXPORT size_t APEX_API MeshLoader::WriteToFile(const ReadOnlyMeshData& ref, const char* filename, MeshExporterFlags flags)
	{
		const MeshData* mesh = ref.m_data;

		const u32 vertexStride = mesh->vertexFormat.CalcVertexStride();

		const size_t size = sizeof(MAGIC) + sizeof(SIGNATURE_MESH) + sizeof(MeshData)
						+ sizeof(u32) * mesh->indexCount
						+ (size_t)vertexStride * mesh->vertexCount;

		char* buffer = (char*)apex_new char[size];

		AxStreamWriter writer { buffer, size };
		AxStreamReader reader { buffer, size };

		writer.WriteObject<u32>(&MAGIC);
		// axCriticalAssert(*reader.ReadObject<u32>() == MAGIC);
		axInfoFmt("magic = {:x} | MAGIC = {:x}", *reader.ReadObject<u32>(), MAGIC);
		writer.WriteObject<u32>(&SIGNATURE_MESH);
		axInfoFmt("signature_mesh = {:x} | SIGNATURE_MESH = {:x}", *reader.ReadObject<u32>(), SIGNATURE_MESH);
		writer.WriteObject(mesh);
		writer.WriteArray(mesh->pIndices, mesh->indexCount);
		writer.WriteArray((char*)mesh->pVertices, (size_t)mesh->vertexCount * vertexStride);

		size_t bytesWritten = WriteFileContents(filename, buffer, size);

		delete[] buffer;

		return bytesWritten;
	}

}
