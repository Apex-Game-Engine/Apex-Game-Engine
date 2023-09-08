#pragma once
#include "Window.h"
#include "Core/Types.h"

#ifdef APEX_PLATFORM_WIN32

#include <windows.h>

namespace apex {

	struct Win32Window : public Window
	{
		static const char WNDCLASS_NAME[];

		Win32Window(HINSTANCE hInstance, WNDPROC lpfnWndProc, uint32 width, uint32 height, const char* name);
		~Win32Window() override;

		void show(int nCmdShow) override;
		void draw() override;
		void close() override;
		void pollOSEvents() override;
		void waitForOSEvent() override;
		void* getOsWindowHandle() override;
		void getFramebufferSize(int& width, int& height) override;
		
		void draw(LPDRAWITEMSTRUCT lpDrawItemStruct);
		HWND getWin32WindowHandle();

		HWND m_hWnd { nullptr };
		bool m_isOpen { false };
	};

}

#endif
