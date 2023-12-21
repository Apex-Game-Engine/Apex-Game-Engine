#include "apex_pch.h"
#include "Apex/Win32Application.h"
#include "Graphics/Window/Win32Window.h"

#include <cassert>


#ifdef APEX_PLATFORM_WIN32
#include <ShellScalingApi.h>

namespace apex {

	Application* Application::s_pInstance = nullptr;

	Application* Application::Construct(uint32 width, uint32 height, const char* name, UniquePtr<Game>&& pGame)
	{
		Win32Application* pInstance = new Win32Application(width, height, name);
		pInstance->m_game = std::forward<UniquePtr<Game>&&>(pGame);

		s_pInstance = pInstance;
		return s_pInstance;
	}

	Win32Application::Win32Application(uint32 width, uint32 height, const char* name)
	: m_hInstance(GetModuleHandle(NULL))
	, m_running(false)
	, m_applicationState(ApplicationState::eStopped)
	{
		setupDPIAwareness();
		m_window = apex::make_unique<Win32Window>(m_hInstance, Win32Application::ProcessWindowsEvents, width, height, name);

		m_running = true;
		m_applicationState = ApplicationState::eRunning;
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
		case WM_SIZE:
			{
				UINT width = LOWORD(lParam);
				UINT height = HIWORD(lParam);

				if (0 == width || 0 == height)
				{
					m_applicationState = ApplicationState::ePaused;
					break;
				}
				else
				{
					m_applicationState = ApplicationState::eRunning;
				}

				// TODO: Add to event queue. Make vulkanContext and forwardRenderer listen to the event
				if (m_vulkanContext.isInitialized())
				{
					m_vulkanContext.onWindowResize(width, height);
					m_forwardRenderer.onWindowResize(width, height);
				}
				break;
			}
		/*case WM_PAINT:
			{
				m_window->draw();
				break;
			}*/
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

	void Win32Application::initialize()
	{
		m_vulkanContext.initialize(APEX_DEFAULT_APPNAME, m_window.get(), true);
		m_forwardRenderer.initialize(m_vulkanContext);

		m_game->initialize();
	}

	void Win32Application::run()
	{
		while (m_running)
		{
			m_window->pollOSEvents();

			if (!m_running || m_applicationState != ApplicationState::eRunning)
				continue;

			m_game->run();
			m_forwardRenderer.onUpdate(16666.666f);
		}
	}

	void Win32Application::exit()
	{
		m_running = false;
	}

	void Win32Application::shutdown()
	{
		m_forwardRenderer.shutdown();
		m_game->stop();
		m_vulkanContext.shutdown();
	}

	Window* Win32Application::getWindow()
	{
		return m_window.get();
	}

	ApplicationState Win32Application::getState()
	{
		return m_applicationState;
	}

	gfx::ForwardRenderer* Win32Application::getRenderer()
	{
		return &m_forwardRenderer;
	}
}

#endif
