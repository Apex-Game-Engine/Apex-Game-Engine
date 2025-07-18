#pragma once

#include "Containers/AxArray.h"
#include "Core/Macros.h"
#include "Core/Types.h"

namespace apex {
namespace asset {

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

	struct MeshData;

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

	private:
		constexpr MeshRef(MeshData* data) : m_data(data) {}

	private:
		MeshData* m_data;

		friend class MeshLoader;
	};

	using PFN_MeshLoaderUserAllocate = void*(*)(size_t);
	using PFN_MeshLoaderUserFree = void(*)(void*);

	class MeshLoader
	{
	public:
		static void SetUserMemoryCallbacks(PFN_MeshLoaderUserAllocate allocate_fn, PFN_MeshLoaderUserFree free_fn);

		constexpr MeshLoader(MeshLoaderFlags flags) : m_flags(flags) {}

		MeshRef ReadFromFile(const char* filename);
		size_t WriteToFile(const MeshRef& ref, const char* filename);

	private:
		MeshLoaderFlags m_flags;
	};

}
}
