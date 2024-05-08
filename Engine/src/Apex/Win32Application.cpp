#include "apex_pch.h"
#include "Apex/Win32Application.h"
#include "Apex/Win32Window.h"

#include <cassert>

#include "Apex/InputManager.h"
#include "Apex/PlatformInput.h"


#ifdef APEX_PLATFORM_WIN32
#include <ShellScalingApi.h>
#include <windowsx.h>

namespace apex {

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
		/*case WM_DRAWITEM:
			{
				m_window->draw(reinterpret_cast<LPDRAWITEMSTRUCT>(lParam));
				break;
			}*/
		case WM_MOUSEMOVE:
			{
				POINT pt;
				pt.x = GET_X_LPARAM(lParam);
				pt.y = GET_Y_LPARAM(lParam);

				int width, height;
				m_window->getFramebufferSize(width, height);
				const math::Vector2 mousePos = { static_cast<float32>(pt.x), static_cast<float32>(pt.y) };
				const math::Vector2 windowSize = { static_cast<float32>(width), static_cast<float32>(height) };
				const math::Vector2 normalizedMousePos = (2.f * mousePos - windowSize) / windowSize;

				setMousePosition(normalizedMousePos);

				break;
			}
		case WM_LBUTTONDOWN:
			{
				setMouseButtonState(MouseButton::MouseBtnLeft, InputState::Pressed);
				break;
			}
		case WM_LBUTTONUP:
			{
				setMouseButtonState(MouseButton::MouseBtnLeft, InputState::Released);
				break;
			}
		case WM_RBUTTONDOWN:
			{
				setMouseButtonState(MouseButton::MouseBtnRight, InputState::Pressed);
				break;
			}
		case WM_RBUTTONUP:
			{
				setMouseButtonState(MouseButton::MouseBtnRight, InputState::Released);
				break;
			}
		case WM_MBUTTONDOWN:
			{
				setMouseButtonState(MouseButton::MouseBtnMiddle, InputState::Pressed);
				break;
			}
		case WM_MBUTTONUP:
			{
				setMouseButtonState(MouseButton::MouseBtnMiddle, InputState::Released);
				break;
			}
		case WM_MOUSEWHEEL:
			{
				WORD delta = GET_WHEEL_DELTA_WPARAM(wParam);
				setMouseWheelDeltaV(delta);
				break;
			}
		case WM_MOUSEHWHEEL:
			{
				WORD delta = GET_WHEEL_DELTA_WPARAM(wParam);
				setMouseWheelDeltaV(delta);
				break;
			}
		case WM_KEYUP:
		case WM_KEYDOWN:
			{
				// eventQueue.push(Event(EventType::eKeyPressed, wParam));
				WORD key = LOWORD(wParam);
				WORD keyFlags = HIWORD(lParam);
				WORD scanCode = LOBYTE(keyFlags);
				BOOL isExtended = (keyFlags & KF_EXTENDED) == KF_EXTENDED;

				if (isExtended)
					scanCode = MAKEWORD(scanCode, 0xE0);

				BOOL wasKeyDown = (keyFlags & KF_REPEAT) == KF_REPEAT;
				WORD repeatCount = LOWORD(lParam);

				BOOL isKeyReleased = (keyFlags & KF_UP) == KF_UP;

				switch (key)
				{
				case VK_SHIFT:
				case VK_CONTROL:
				case VK_MENU:
					key = LOWORD(MapVirtualKeyW(scanCode, MAPVK_VSC_TO_VK_EX));
				}

				setKeyState(translateKeyCode(key), isKeyReleased ? InputState::Released : InputState::Pressed);
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
		m_inputManager.initialize();
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

			m_game->update(16666.666f);
			m_forwardRenderer.onUpdate(16666.666f);
		}
	}

	void Win32Application::exit()
	{
		m_running = false;
	}

	void Win32Application::shutdown()
	{
		m_forwardRenderer.stop();
		m_game->stop();
		m_forwardRenderer.shutdown();
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

	InputManager* Win32Application::getInputManager()
	{
		return &m_inputManager;
	}

	gfx::ForwardRenderer* Win32Application::getRenderer()
	{
		return &m_forwardRenderer;
	}
}

#endif
