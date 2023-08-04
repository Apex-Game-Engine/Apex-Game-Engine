#pragma once
#include "Apex/Application.h"

#include "Core/Logging.h"

#include "Graphics/Vulkan/VulkanContext.h"

#ifdef APEX_PLATFORM_WIN32
	#include "Apex/Win32Application.h"
	#include "GlobalDefines.h"

	int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pCmdLine, int nCmdShow)
	{
		apex::logging::Logger::initialize();

		apex::Application *app = apex::Application::Construct(hInstance, nCmdShow, 1366u, 768u, APEX_DEFAULT_APPNAME);

		// TODO: move this to a GameContext struct
		apex::gfx::VulkanContext vulkanContext;
		vulkanContext.initialize(APEX_DEFAULT_APPNAME, app->getWindow(), true);

		app->getWindow()->construct(hInstance);
		app->getWindow()->show(nCmdShow);
		app->run();

		vulkanContext.shutdown();

		return 0;
	}
#else
	int main()
	{
	}
#endif
