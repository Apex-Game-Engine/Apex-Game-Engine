#pragma once

#include "Containers/AxArray.h"
#include "Core/Macros.h"
#include "Core/Types.h"

#include "Graphics/GraphicsContext.h"

namespace apex {
namespace asset {
	
	struct MeshData;
	class MeshGpu;

	enum class MeshLoaderFlagBits : u32
	{
		None				= 0x00000000,
		Triangulate			= 0x00000001,
		FlipNormals			= 0x00000002,
		CalculateNormals	= 0x00000004,
		CalculateTangents	= 0x00000008,
	};
	using MeshLoaderFlags = Flags<MeshLoaderFlagBits>;
	DEFINE_BITWISE_OPERATORS(MeshLoaderFlagBits, MeshLoaderFlags);

	enum class MeshAttribute
	{
		Position,
		Normal,
		Tangent,
		TexCoords0,
		TexCoords1,
		Color,
		BoneWeights,
		BoneIndices,
	};

	struct MeshStream
	{
		uint32_t stride;
		uint32_t attributeStart;
		uint32_t attributeCount;
		uint32_t dataOffset;
	};

	struct VertexList
	{
		void* data;
		u32 stride;
		u32 count;
	};

	class MeshRef
	{
	public:
		~MeshRef();
		NON_COPYABLE(MeshRef);
		MeshRef(MeshRef&&) noexcept;
		MeshRef& operator=(MeshRef&&) noexcept;

		AxArrayRef<MeshStream> GetStreams() const;
		AxArrayRef<MeshAttribute> GetAttributes() const;
		AxArrayRef<u32> GetIndices() const;
		VertexList GetVertices() const;

		u32 CalculateVertexStride() const;

		MeshGpu UploadToGpu() const;

	private:
		constexpr MeshRef(MeshData* data) : m_data(data) {}

	private:
		MeshData* m_data;

		friend class MeshLoader;
	};

	class MeshGpu
	{
	public:
		MeshGpu() = default;
		~MeshGpu() = default;
		NON_COPYABLE(MeshGpu);
		
		MeshGpu(MeshGpu&& other) noexcept
			: m_vertexBuffer(std::move(other.m_vertexBuffer)) , m_indexBuffer(std::move(other.m_indexBuffer))
		{
		}
		
		MeshGpu& operator=(MeshGpu&& other) noexcept
		{
			m_vertexBuffer = std::move(other.m_vertexBuffer);
			m_indexBuffer = std::move(other.m_indexBuffer);
		}

		const gfx::BufferRef& GetVertexBuffer() const { return m_vertexBuffer; }
		const gfx::BufferRef& GetIndexBuffer() const { return m_indexBuffer; }

	private:
		MeshGpu(gfx::Buffer* vertex_buffer, gfx::Buffer* index_buffer)
			: m_vertexBuffer(vertex_buffer), m_indexBuffer(index_buffer)
		{
		}

	private:
		gfx::BufferRef m_vertexBuffer;
		gfx::BufferRef m_indexBuffer;

		friend class MeshRef;
	};

	using PFN_MeshLoaderUserAllocate = void*(*)(size_t);
	using PFN_MeshLoaderUserFree = void(*)(void*);

	class MeshLoader
	{
	public:
		static void SetUserMemoryCallbacks(PFN_MeshLoaderUserAllocate allocate_fn, PFN_MeshLoaderUserFree free_fn);

		constexpr MeshLoader(MeshLoaderFlags flags = MeshLoaderFlagBits::None) : m_flags(flags) {}

		MeshRef ReadFromFile(const char* filename);
		size_t WriteToFile(const MeshRef& ref, const char* filename);

	private:
		MeshLoaderFlags m_flags;
	};

}
}
