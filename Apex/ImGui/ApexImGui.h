#pragma once
#include "Graphics/GraphicsContext.h"
#include "Platform/PlatformManager.h"

namespace apex::gfx
{
	class CommandBuffer;
	class Context;
}

namespace apex {

	struct AxImGui
	{
		static bool Init(plat::PlatformWindow& window, gfx::Context& ctx);
		static void Shutdown();
		static void BeginFrame();
		static void EndFrame();
		static void Render(gfx::CommandBuffer* command_buffer);
	};

}
