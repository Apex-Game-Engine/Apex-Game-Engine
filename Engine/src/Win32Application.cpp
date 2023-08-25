#include "apex_pch.h"
#include "Apex/Win32Application.h"
#include "Graphics/Window/Win32Window.h"

#include <cassert>


#ifdef APEX_PLATFORM_WIN32
#include <ShellScalingApi.h>

namespace apex {

	Application* Application::s_pInstance = nullptr;

	Application* Application::Construct(void* hInstance, int nCmdShow, uint32 width, uint32 height, const char* name)
	{
		Win32Application* pInstance = new Win32Application(static_cast<HINSTANCE>(hInstance), nCmdShow, width, height, name);

		s_pInstance = pInstance;
		return s_pInstance;
	}

	Win32Application::Win32Application(HINSTANCE hInstance, int nCmdShow, uint32 width, uint32 height, const char* name)
	:m_running(true)
	{
		setupDPIAwareness();

		m_window = apex::make_unique<Win32Window>(hInstance, nCmdShow, Win32Application::ProcessWindowsEvents, width, height, name);
	}

	LRESULT Win32Application::ProcessWindowsEvents(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (s_pInstance)
		{
			return static_cast<Win32Application*>(s_pInstance)->processWindowsEvents(hWnd, uMsg, wParam, lParam);
		}
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
		case WM_CLOSE: // window close event
			{
				// TODO: Add quit event to event queue
				m_window->close();
				break;
			}
		case WM_DESTROY:
			{
				m_running = false;
				break;
			}
		case WM_PAINT:
			{
				m_window->draw();
				break;
			}
		case WM_DRAWITEM:
			{
				m_window->draw(reinterpret_cast<LPDRAWITEMSTRUCT>(lParam));
				break;
			}
		default:
			break;
		}

		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	void Win32Application::setupDPIAwareness()
	{
		typedef HRESULT *(__stdcall *SetProcessDpiAwarenessFunc)(PROCESS_DPI_AWARENESS);

		HMODULE shCore = LoadLibraryA("Shcore.dll");
		if (shCore)
		{
			SetProcessDpiAwarenessFunc setProcessDpiAwareness = (SetProcessDpiAwarenessFunc)GetProcAddress(shCore, "SetProcessDpiAwareness");

			if (nullptr != setProcessDpiAwareness)
			{
				setProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
			}

			FreeLibrary(shCore);
		}
	}

	void Win32Application::run()
	{
		while (m_running)
		{
			m_window->pollOSEvents();
		}
	}

	void Win32Application::exit()
	{
		m_running = false;
	}

	Window* Win32Application::getWindow()
	{
		return m_window.get();
	}
}

#endif
