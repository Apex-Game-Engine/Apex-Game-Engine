#include "Asset/Mesh.h"
#include "Core/Asserts.h"
#include "Core/Files.h"
#include "String/AxStream.h"

constexpr static uint32_t MAGIC = 0x58455041; // 'APEX'
constexpr static uint32_t SIGNATURE_MESH = 0x00000001;

namespace apex::asset {

	namespace detail
	{
		const PFN_MeshLoaderUserAllocate defaultAlloc = mem::GlobalMemoryOperators::OperatorNew;
		const PFN_MeshLoaderUserFree defaultFree = mem::GlobalMemoryOperators::OperatorDelete;

		PFN_MeshLoaderUserAllocate memAlloc = defaultAlloc;
		PFN_MeshLoaderUserFree memFree = defaultFree;
	}

	struct MeshData
	{
		uint32_t streamCount;
		uint32_t attributeCount;
		uint32_t indexCount;
		uint32_t vertexCount;
		MeshStream* pStreams;
		MeshAttribute* pAttributes;
		uint32_t* pIndices;
		void* pVertices;
	};

	static char* ReadFileContents(const char* filename, size_t* out_size)
	{
		File file = File::OpenExisting(filename);
		size_t size = file.GetSize();
		char* buffer = (char*)detail::memAlloc(sizeof(char) * size);
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

	u32 CalculateVertexStride(const MeshData* mesh)
	{
		uint32_t vertexStride = 0;
		for (uint32_t i = 0; i < mesh->streamCount; i++)
	    {
	        vertexStride += mesh->pStreams[i].stride;
	    }
		return vertexStride;
	}

	MeshRef::~MeshRef()
	{
		if (m_data == nullptr)
			return;
		detail::memFree((char*)m_data - sizeof(MAGIC) - sizeof(SIGNATURE_MESH));
	}

	MeshRef::MeshRef(MeshRef&& other) noexcept
	{
		*this = std::move(other);
	}

	MeshRef& MeshRef::operator=(MeshRef&& other) noexcept
	{
		m_data = other.m_data;
		other.m_data = nullptr;
		return *this;
	}

	AxArrayRef<MeshStream> MeshRef::GetStreams() const
	{
		return { m_data->pStreams, m_data->streamCount };
	}

	AxArrayRef<MeshAttribute> MeshRef::GetAttributes() const
	{
		return { m_data->pAttributes, m_data->attributeCount };
	}

	AxArrayRef<u32> MeshRef::GetIndices() const
	{
		return { m_data->pIndices, m_data->indexCount };
	}

	VertexList MeshRef::GetVertices() const
	{
		return { m_data->pVertices, CalculateVertexStride(), m_data->vertexCount };
	}

	u32 MeshRef::CalculateVertexStride() const
	{
		return asset::CalculateVertexStride(m_data);
	}

	void MeshLoader::SetUserMemoryCallbacks(PFN_MeshLoaderUserAllocate allocate_fn, PFN_MeshLoaderUserFree free_fn)
	{
		detail::memAlloc = allocate_fn ? allocate_fn : detail::defaultAlloc;
		detail::memFree = free_fn ? free_fn : detail::defaultFree;
	}

	MeshRef MeshLoader::ReadFromFile(const char* filename)
	{
		size_t size;
		char* buffer = ReadFileContents(filename, &size);
		axAssert(buffer && size);

		AxStreamReader reader { buffer, size };

		if (const uint32_t magic = *reader.ReadObject<uint32_t>(); !axVerify(magic == MAGIC))
		{
			detail::memFree(buffer);
			return { nullptr };
		}

		if (const uint32_t signature = *reader.ReadObject<uint32_t>(); !axVerify(signature == SIGNATURE_MESH))
		{
			detail::memFree(buffer);
			return { nullptr };
		}

		MeshData* outMesh = reader.ReadObject<MeshData>();
		outMesh->pStreams = reader.ReadArray<MeshStream>(outMesh->streamCount);
		const uint32_t vertexStride = CalculateVertexStride(outMesh);
		outMesh->pAttributes = reader.ReadArray<MeshAttribute>(outMesh->attributeCount);
		outMesh->pIndices = reader.ReadArray<uint32_t>(outMesh->indexCount);
		outMesh->pVertices = reader.ReadArray<char>((size_t)outMesh->vertexCount * vertexStride);

		return { outMesh };
	}

	size_t MeshLoader::WriteToFile(const MeshRef& ref, const char* filename)
	{
		const MeshData* mesh = ref.m_data;

		const uint32_t vertexStride = CalculateVertexStride(mesh);

		const size_t size = sizeof(MAGIC) + sizeof(SIGNATURE_MESH) + sizeof(MeshData)
						+ sizeof(MeshStream) * mesh->streamCount
						+ sizeof(MeshAttribute) * mesh->attributeCount
						+ sizeof(uint32_t) * mesh->indexCount
						+ (size_t)vertexStride * mesh->vertexCount;

		char* buffer = (char*)detail::memAlloc(sizeof(char) * size);

		AxStreamWriter writer { buffer, size };

		writer.WriteObject(&MAGIC);
		writer.WriteObject(&SIGNATURE_MESH);
		writer.WriteObject(mesh);
		writer.WriteArray(mesh->pStreams, mesh->streamCount);
		writer.WriteArray(mesh->pAttributes, mesh->attributeCount);
		writer.WriteArray(mesh->pIndices, mesh->indexCount);
		writer.WriteArray((char*)mesh->pVertices, (size_t)mesh->vertexCount * vertexStride);

		size_t bytesWritten = WriteFileContents(filename, buffer, size);

		detail::memFree(buffer);

		return bytesWritten;
	}
}
