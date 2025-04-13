#pragma once
#include "Application.h"
#include "Game.h"
#include "InputManager.h"
#include "Core/Types.h"
#include "Graphics/ForwardRenderer.h"
#include "Graphics/Vulkan/VulkanContext.h"
#include "Memory/UniquePtr.h"

#ifdef APEX_PLATFORM_WIN32
#include "Apex/Win32Window.h"

namespace apex {

	struct Win32Application : public Application
	{
		Win32Application(u32 width, u32 height, const char* name);
		static LRESULT CALLBACK ProcessWindowsEvents(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		LRESULT CALLBACK processWindowsEvents(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		void setupDPIAwareness();
		
		void initialize() override;
		void run() override;
		void exit() override;
		void shutdown() override;

		Window* getWindow() override;
		ApplicationState getState() override;
		InputManager* getInputManager() override;
		gfx::ForwardRenderer* getRenderer() override;

		// Application
		HINSTANCE m_hInstance;
		UniquePtr<Win32Window> m_window;
		bool m_running;
		ApplicationState m_applicationState;
		
		InputManager m_inputManager;

		// Graphics
		vk::VulkanContext m_vulkanContext;
		gfx::ForwardRenderer m_forwardRenderer;

		//Game
		UniquePtr<Game> m_game;
	};

}

#endif