#pragma once

#include "Factory.h"
#include "GraphicsContext.h"
#include "Containers/AxArray.h"

#if 0

namespace apex {
namespace gfx {
	class Pipeline;

	class FrameData
	{
	public:
		FrameData(Device* device);
		~FrameData();
		NON_COPYABLE(FrameData);
		DEFAULT_MOVABLE(FrameData);

		void BeginFrame();
		void EndFrame();

		Image* GetBackBufferImage() const
		{
			return m_images[m_frameIndex];
		}

		CommandBuffer* GetCommandBuffer() const
		{
			return m_commandBuffers[m_frameIndex];
		}

		u32 GetFrameIndex() const
		{
			return m_frameIndex;
		}

		u32 GetFrameCount() const
		{
			return m_frameCount;
		}

#if APEX_DEBUG
		Image* GetBackBufferImage_Unsafe(u32 frame_index)
		{
			return m_images[frame_index];
		}

		CommandBuffer* GetCommandBuffer_Unsafe(u32 frame_index) const
		{
			return m_commandBuffers[frame_index];
		}
#endif // APEX_DEBUG

	private:
		AxStaticArray<Image*, MAX_FRAMES_IN_FLIGHT> m_images;
		AxStaticArray<CommandBuffer*, MAX_FRAMES_IN_FLIGHT> m_commandBuffers;
		const Image* m_currentSwapchainImage;
		u64 m_frameIndex;
		u64 m_frameCount;
		bool m_presentSubmitted;
	};		

	class GBuffer
	{
	public:
		enum Attachment
		{
			eAlbedo,
			eNormal,
			eEmissive,
			eDepth,
			eTotalCount,
			eColorCount = eDepth,
		};

		constexpr static Format GetFormat(Attachment attachment)
		{
			switch (attachment)
			{
			case eAlbedo:	return Format::B8G8R8A8_SRGB;
			case eNormal:	return Format::R16G16B16A16_SNORM;
			case eEmissive:	return Format::B10G11R11_UFLOAT;
			case eDepth:	return Format::D32_SFLOAT;
			}
			return Format::UNDEFINED;
		}

		GBuffer() = default;
		~GBuffer() { Shutdown(); }

		NON_COPYABLE(GBuffer);
		DEFAULT_MOVABLE(GBuffer);

		void Init(Device* device, Dim2D dimensions);
		void Shutdown();

		void Begin(CommandBuffer* cmdbuf);
		void End(CommandBuffer* cmdbuf);

		Image* GetImage(Attachment attachment) const
		{
			return m_images[m_frameIndex * eTotalCount + attachment];
		}

		u32 GetFrameIndex() const
		{
			return m_frameIndex;
		}

		u32 GetFrameCount() const
		{
			return m_frameCount;
		}

#if APEX_DEBUG
		Image* GetImage_Unsafe(u32 frame_index, Attachment attachment) const
		{
			return m_images[frame_index * eTotalCount + attachment];
		}
#endif // APEX_DEBUG

	private:
		AxStaticArray<Image*, MAX_FRAMES_IN_FLIGHT * eTotalCount> m_images;
		u64 m_frameIndex;
		u64 m_frameCount;
		bool m_frameEnded;
	};

	class RenderPhase
	{
		enum Enum
		{
			eDepthPrePassPhase,
			eShadowMapPhase,
			eGBufferPhase,
			eOpaqueLightingPhase,
			eTransparentLightingPhase,
			ePostProcessPhase,

			MAX_COUNT
		};

		static void Init();
		static void Shutdown();

		static void CreatePipelines();
		static void CreateRenderTargets();

		static void BeginRenderPhase(RenderPhase::Enum phase);
		static void EndRenderPhase();
	};



	class Mesh
	{
	public:
		Mesh() = default;
		~Mesh() = default;

		NON_COPYABLE(Mesh);
		
		Mesh(Mesh&& other) noexcept
			: m_vertexFormat(other.m_vertexFormat), m_vertexBuffer(std::move(other.m_vertexBuffer)), m_indexBuffer(std::move(other.m_indexBuffer))
			, m_vertexCount(other.m_vertexCount), m_indexCount(other.m_indexCount)
		{
		}
		
		Mesh& operator=(Mesh&& other) noexcept
		{
			m_vertexFormat = other.m_vertexFormat;
			m_vertexBuffer = std::move(other.m_vertexBuffer);
			m_indexBuffer = std::move(other.m_indexBuffer);
			m_vertexCount = other.m_vertexCount;
			m_indexCount = other.m_indexCount;
			return *this;
		}

		const VertexFormat& GetVertexFormat() const { return m_vertexFormat; }
		const BufferRef& GetVertexBuffer() const { return m_vertexBuffer; }
		const BufferRef& GetIndexBuffer() const { return m_indexBuffer; }
		u32 GetVertexCount() const { return m_vertexCount; }
		u32 GetIndexCount() const { return m_indexCount; }

	private:
		Mesh(VertexFormat vertex_format, Buffer* vertex_buffer, Buffer* index_buffer, u32 vertex_count, u32 index_count)
			: m_vertexFormat(vertex_format), m_vertexBuffer(vertex_buffer), m_indexBuffer(index_buffer)
			, m_vertexCount(vertex_count), m_indexCount(index_count)
		{
		}
		
		VertexFormat m_vertexFormat;
		BufferRef m_vertexBuffer;
		BufferRef m_indexBuffer;
		u32 m_vertexCount{};
		u32 m_indexCount{};

		friend Mesh* CreateMesh(VertexFormat vertex_format, u32 vertex_count, void *pVertices, u32 index_count, u32 *pIndices);
	};

	Mesh* CreateMesh(VertexFormat vertex_format, u32 vertex_count, void *pVertices, u32 index_count, u32 *pIndices);

}
}

#endif