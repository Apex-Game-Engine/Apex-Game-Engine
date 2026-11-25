#pragma once

#include "Containers/AxArray.h"
#include "Core/Macros.h"
#include "Core/Types.h"

#include "Graphics/GraphicsContext.h"
#include "Graphics/Renderer.h"

namespace apex {
namespace asset {
	
	struct MeshData;

	enum class MeshExporterFlagBits : u32
	{
		None				= 0x00000000,
		Triangulate			= 0x00000001,
		FlipNormals			= 0x00000002,
		CalculateNormals	= 0x00000004,
		CalculateTangents	= 0x00000008,
	};
	using MeshExporterFlags = Flags<MeshExporterFlagBits>;
	DEFINE_BITWISE_OPERATORS(MeshExporterFlagBits, MeshExporterFlags);

	// Forward declarations
	class ReadOnlyMeshData;
	class ReadOnlyMeshStream;

	class ReadOnlyMeshData
	{
		enum FlagBits
		{
			NONE						= 0x00,
			NEEDS_DELETION_BIT			= 0x01,
			HAS_HEADER_BIT				= 0x02,
			SEPARATE_ALLOCATIONS_BIT	= 0x04,
		};
		typedef u32 Flags;

	public:
		~ReadOnlyMeshData();
		NON_COPYABLE(ReadOnlyMeshData);
		ReadOnlyMeshData(ReadOnlyMeshData&&) noexcept;
		ReadOnlyMeshData& operator=(ReadOnlyMeshData&&) noexcept;

		ReadOnlyMeshStream	GetStream(u32 index) const;
		gfx::VertexFormat	GetVertexFormat() const;
		u32					GetIndexCount() const;
		u32*				GetIndices() const;
		u32					GetVertexCount() const;
		void*				GetVertices() const;

	private:
		constexpr ReadOnlyMeshData(MeshData* data, Flags flags) : m_data(data), m_flags(flags) {}

	private:
		MeshData* m_data;
		Flags m_flags;

		friend class MeshLoader;
		friend class MeshDataBuilder;
	};

	class ReadOnlyMeshStream
	{
	public:
		~ReadOnlyMeshStream() = default;



	private:
		ReadOnlyMeshStream(MeshData* data, u32 stream_index);

	private:
		MeshData* m_data;
		u32 m_streamIndex;
		gfx::VertexAttributeStream m_offsetAndStride;

		friend class ReadOnlyMeshData;
	};

#if APEX_TOOL
	class MeshDataBuilder
	{
	public:
		MeshDataBuilder() = default;
		ReadOnlyMeshData Build(gfx::VertexFormat vertex_format, u32 size, u8* vertices, u32 count, u32* indices) const;
	};
#endif

	class MeshLoader
	{
	public:
		APEX_EXPORT static ReadOnlyMeshData		APEX_API ReadFromFile	(const char* filename, MeshExporterFlags flags = MeshExporterFlagBits::None);
		APEX_EXPORT static size_t				APEX_API WriteToFile	(const ReadOnlyMeshData& ref, const char* filename, MeshExporterFlags flags = MeshExporterFlagBits::None);
	};

}
}
