﻿#include "Graphics/Window/Win32Window.h"

#include "Core/Asserts.h"

#ifdef APEX_PLATFORM_WIN32

namespace apex {

	const char Win32Window::WNDCLASS_NAME[] = "apex::Win32Window";

	Win32Window::Win32Window(HINSTANCE hInstance, int nCmdShow, WNDPROC lpfnWndProc, uint32 width, uint32 height, const char* name)
	{
		static bool isWndClassRegistered = false;
		if (!isWndClassRegistered)
		{
			WNDCLASS wc{};
			wc.lpfnWndProc   = lpfnWndProc;
			wc.lpszClassName = Win32Window::WNDCLASS_NAME;
			wc.hInstance     = static_cast<HINSTANCE>(hInstance);

			RegisterClass(&wc);

			isWndClassRegistered = true;
		}

		m_hWnd = CreateWindowEx(
			0,                               // Optional window styles
			Win32Window::WNDCLASS_NAME,      // Window class
			name,                            // Window text
			WS_OVERLAPPEDWINDOW,             // Window style

			CW_USEDEFAULT, CW_USEDEFAULT,    // Position
			width, height,                   // Size
			
			NULL,      // Parent window
			NULL,      // Menu
			hInstance, 
			NULL 
		);

		axAssert(m_hWnd != nullptr);
	}

	void Win32Window::show(int nCmdShow)
	{
		ShowWindow(m_hWnd, nCmdShow);
	}

	void Win32Window::draw()
	{
	}

	void Win32Window::draw(LPDRAWITEMSTRUCT lpDrawItemStruct)
	{

	}

	void Win32Window::pollOSEvents()
	{
		MSG msg{nullptr};
		while (PeekMessage(&msg, m_hWnd, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
	    }
	}

	void Win32Window::waitForOSEvent()
	{
		MSG msg{nullptr};
		if (GetMessage(&msg, NULL, 0, 0) > 0)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	void* Win32Window::getOsWindowHandle()
	{
		return m_hWnd;
	}

	void Win32Window::onCommand(WPARAM wParam, LPARAM lParam)
	{
	}
}

#endif
