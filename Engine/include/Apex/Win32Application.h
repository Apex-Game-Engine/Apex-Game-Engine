#pragma once
#include "Application.h"
#include "Core/Types.h"
#include "Memory/UniquePtr.h"

#ifdef APEX_PLATFORM_WIN32
#include "Graphics/Window/Win32Window.h"

namespace apex {

	struct Win32Application : public Application
	{
		Win32Application(HINSTANCE hInstance, int nCmdShow, uint32 width, uint32 height, const char* name);

		static LRESULT CALLBACK ProcessWindowsEvents(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		LRESULT CALLBACK processWindowsEvents(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		void setupDPIAwareness();
		
		void run() override;
		void exit() override;
		Window* getWindow() override;

		UniquePtr<Win32Window> m_window;
		bool m_running;
	};

}

#endif