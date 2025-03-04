#include "Graphics/GraphicsContext.h"

#include "Graphics/Vulkan/VulkanContext.h"
#include "Memory/MemoryManager.h"

namespace apex::gfx {

	Context Context::CreateContext(ContextApi api)
	{
		ContextBase* instance = nullptr;
		switch (api)
		{
		case ContextApi::None:
			// m_instance = new NullContext;
			break;
		case ContextApi::Vulkan:
		{
			instance = apex_new(VulkanContext);
			break;
		}
		case ContextApi::D3D12:
			//m_instance = new D3D12Context;
			break;
		case ContextApi::WebGpu:
			//m_instance = new WebGpuContext;
			break;
		}
		axStrongAssertFmt(instance, "Failed to create a graphics context! Check if the requested API is available on the devices.");
		return { instance };
	}

}
