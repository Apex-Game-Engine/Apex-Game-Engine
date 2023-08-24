#pragma once
#include "Console.h"
#include "Apex/Application.h"

#include "Core/Logging.h"

#include "Graphics/Vulkan/VulkanContext.h"
#include "Memory/MemoryManager.h"

#ifdef APEX_PLATFORM_WIN32
	#include "Apex/Win32Application.h"
	#include "GlobalDefines.h"

	int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pCmdLine, int nCmdShow)
	{
		apex::logging::Logger::initialize();
		apex::memory::MemoryManager::initialize({ .frameArenaSize = 0, .numFramesInFlight = 3 });

		apex::Application *app = apex::Application::Construct(hInstance, nCmdShow, 1366u, 768u, APEX_DEFAULT_APPNAME);

	#if defined(APEX_CONFIG_DEBUG) || defined(APEX_CONFIG_DEVELOPMENT)
		apex::Console console(APEX_DEFAULT_APPNAME);
		console.connect();
	#endif

		// TODO: move this to a GameContext struct
		apex::vk::VulkanContext vulkanContext;
		vulkanContext.initialize(APEX_DEFAULT_APPNAME, app->getWindow(), true);

		app->getWindow()->show(nCmdShow);
		app->run();

		vulkanContext.shutdown();

		apex::memory::MemoryManager::shutdown();

		return 0;
	}
#else
	int main()
	{
	}
#endif
