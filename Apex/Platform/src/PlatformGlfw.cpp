#include "Platform/PlatformManager.h"
#include "Core/Asserts.h"

#if APEX_PLATFORM_USE_GLFW

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace apex {

	struct PlatformContext
	{
		GLFWwindow* window;
	};

	static PlatformContext g_context;

	void PlatformManager::Init(const PlatformWindowCreateParams& window_params)
	{
		if (!axVerify(glfwInit()))
		{
			exit(1);
		}

		glfwSetErrorCallback([](int error, const char* description)
		{
			axErrorFmt("GLFW Error[{}]: {}", error, description);
		});

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		GLFWwindow* window = glfwCreateWindow(window_params.width, window_params.height, window_params.title.c_str(), nullptr, nullptr);
		if (!axVerifyFmt(window, "Window creation failed!"))
		{
			exit(1);
		}

		glfwShowWindow(window);

		g_context.window = window;
	}

	void PlatformManager::Shutdown()
	{
		glfwDestroyWindow(g_context.window);
		glfwTerminate();
	}

	void PlatformManager::PollEvents()
	{
		glfwPollEvents();
	}

	bool PlatformManager::ShouldWindowClose()
	{
		return glfwWindowShouldClose(g_context.window);
	}
}

#endif // APEX_PLATFORM_USE_GLFW
