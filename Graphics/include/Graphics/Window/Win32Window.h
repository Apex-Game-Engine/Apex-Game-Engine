#pragma once
#include "Window.h"
#include "Core/Types.h"

#ifdef APEX_PLATFORM_WIN32

#include <windows.h>

namespace apex {

	struct Win32Window : public Window
	{
		static const char WNDCLASS_NAME[];
		Win32Window(HINSTANCE hInstance, int nCmdShow, WNDPROC lpfnWndProc, uint32 width, uint32 height, const char* name);

		void construct(void* hInstance) override { construct(static_cast<HINSTANCE>(hInstance)); }
		void show(int nCmdShow) override;
		void draw() override;
		void pollOSEvents() override;
		void waitForOSEvent() override;
		void* getOsWindowHandle() override;

		void onCommand(WPARAM wParam, LPARAM lParam);
		void draw(LPDRAWITEMSTRUCT lpDrawItemStruct);
		void construct(HINSTANCE hInstance);

		HWND m_hWnd { nullptr };
	};

}

#endif
