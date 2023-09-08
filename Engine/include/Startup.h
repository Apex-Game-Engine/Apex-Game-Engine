#pragma once
#include "Console.h"
#include "Apex/Application.h"

#include "Core/Logging.h"
#include "Graphics/ForwardRenderer.h"

#include "Graphics/Vulkan/VulkanContext.h"
#include "Memory/MemoryManager.h"

#ifdef APEX_PLATFORM_WIN32
	#include "Apex/Win32Application.h"
	#include "GlobalDefines.h"

	int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pCmdLine, int nCmdShow)
	{
		apex::logging::Logger::initialize();
		apex::memory::MemoryManager::initialize({ .frameArenaSize = 0, .numFramesInFlight = 3 });

		apex::Application *app = apex::Application::Construct(1366u, 768u, APEX_DEFAULT_APPNAME);

	#if defined(APEX_CONFIG_DEBUG) || defined(APEX_CONFIG_DEVELOPMENT)
		apex::Console console(APEX_DEFAULT_APPNAME);
		console.connect();
	#endif

		app->getWindow()->show(nCmdShow);
		app->initialize();
		app->run();

		app->shutdown();
		apex::memory::MemoryManager::shutdown();

	#if defined(APEX_CONFIG_DEBUG) || defined(APEX_CONFIG_DEVELOPMENT)
		system("pause");
	#endif

		return 0;
	}
#else
	int main()
	{
	}
#endif
