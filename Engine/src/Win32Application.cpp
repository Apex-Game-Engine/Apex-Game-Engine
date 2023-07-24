#include "apex_pch.h"
#include "Apex/Win32Application.h"

#include <cassert>


#ifdef APEX_PLATFORM_WIN32
#include "Graphics/Window/Win32Window.h"

namespace apex {

	Application* Application::s_pInstance = nullptr;

	Application* Application::Construct(void* hInstance, int nCmdShow, uint32 width, uint32 height, const char* name)
	{
		Win32Application* pInstance = new Win32Application(static_cast<HINSTANCE>(hInstance), nCmdShow, width, height, name);

		s_pInstance = pInstance;
		return s_pInstance;
	}

	Win32Application::Win32Application(HINSTANCE hInstance, int nCmdShow, uint32 width, uint32 height, const char* name)
	: m_window(hInstance, nCmdShow, Win32Application::ProcessWindowsEvents, width, height, name)
	, m_running(true)
	{
	}

	LRESULT Win32Application::ProcessWindowsEvents(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (s_pInstance)
			return static_cast<Win32Application*>(s_pInstance)->processWindowsEvents(hWnd, uMsg, wParam, lParam);
		else
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	LRESULT Win32Application::processWindowsEvents(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_CREATE:
			{
				break;
			}
		case WM_DESTROY:
			{
				PostQuitMessage(0);                                                                 

				// TODO: Add quit event to event queue
				exit();
				return 0;
			}
		case WM_PAINT:
			{
				m_window.draw();
				return 0;
			}
		case WM_DRAWITEM:
			{
				m_window.draw(reinterpret_cast<LPDRAWITEMSTRUCT>(lParam));
				break;
			}
		case WM_COMMAND:
			{
				if (m_window.m_hWnd == hWnd)
					m_window.onCommand(wParam, lParam);
				break;
			}
		default:
			break;
		}

		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	void Win32Application::run()
	{
		while (m_running)
		{
			m_window.pollOSEvents();
		}
	}

	void Win32Application::exit()
	{
		m_running = false;
	}

	Window* Win32Application::getWindow()
	{
		return &m_window;
	}
}

#endif
