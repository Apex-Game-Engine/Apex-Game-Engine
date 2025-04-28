#pragma once
#include "gfx_config.h"
#include "Core/Types.h"

namespace apex::gfx
{
	class CommandBuffer;
}

namespace apex::math
{
	struct Quat;
	struct Vector3;
}

namespace apex {
namespace gfx {
	class Buffer;
	class Device;
	class GraphicsPipeline;

	struct DebugRendererCreateDesc
	{
		u32 maxVertexCount;
	};

	class DebugRenderer
	{
	public:
		static DebugRenderer* Create(Device* device, DebugRendererCreateDesc const& desc);

		~DebugRenderer();

		void NewFrame();
		void Draw(CommandBuffer* cmd) const;

		void Line(math::Vector3 const& v0, math::Vector3 const& v1);
		void Cube(math::Vector3 const& pos, math::Vector3 const& size);

	protected:
		DebugRenderer(Device* device, GraphicsPipeline* pipeline)
		: m_device(device), m_pipeline(pipeline)
		{
		}

	private:
		Device*							m_device {};
		GraphicsPipeline*				m_pipeline {};
		Buffer*							m_vertexBuffers [MAX_FRAMES_IN_FLIGHT] {};
		Buffer*							m_indexBuffers [MAX_FRAMES_IN_FLIGHT] {};
		u32								m_maxVertexCount {};
		u32								m_maxIndexCount {};
		u32								m_vertexCount {};
		u32								m_indexCount {};
		u32								m_frameIndex {};
	};

}
}
