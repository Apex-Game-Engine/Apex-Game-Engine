#include "Core/Asserts.h"
#include "Platform/InputManager.h"
#include "Platform/PlatformManager.h"
#include "Platform/Timer.h"

#if APEX_PLATFORM_WIN32

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <xinput.h>
#include <ShellScalingApi.h>

namespace apex::plat {

	#define WNDCLASSNAME STR(apex::plat::PlatformWindowImpl)

	// xinput.dll
	typedef DWORD (WINAPI *PFN_XInputGetCapabilities)(DWORD,DWORD,XINPUT_CAPABILITIES*);
	typedef DWORD (WINAPI *PFN_XInputGetState)(DWORD,XINPUT_STATE*);
	typedef DWORD (WINAPI *PFN_XInputSetState)(DWORD,XINPUT_VIBRATION*);
	#define XInputGetCapabilities g_context.xinput.GetCapabilities
	#define XInputGetState g_context.xinput.GetState

	// kernel32.dll
	// CreateWaitableTimerEx

	// shcore.dll
	typedef DWORD (WINAPI *PFN_SetProcessDpiAwareness)(PROCESS_DPI_AWARENESS);
	typedef DWORD (WINAPI *PFN_GetDpiForMonitor)(HMONITOR,MONITOR_DPI_TYPE,UINT*,UINT*);
	#define SetProcessDpiAwareness g_context.shcore.SetProcessDpiAwareness_
	#define GetDpiForMonitor g_context.shcore.GetDpiForMonitor_

	// ntdll.dll
	typedef DWORD (WINAPI *PFN_RtlCaptureStackBackTrace)(DWORD,DWORD,PVOID*,PDWORD);
	#define RtlCaptureStackBackTrace g_context.ntdll.RtlCaptureStackBackTrace_

	struct PlatformWindowImpl
	{
		HWND window;
		bool shouldQuit;
		u32 width, height; 
		u32 lastPosX, lastPosY;
	};

	struct PlatformContext
	{
		HINSTANCE			instance;
		PlatformWindowImpl	mainWindow;
		PlatformWindow      mainWindowWrapper;
		InputManager		inputManager;
		LARGE_INTEGER		timerFrequency;

		struct {
			HMODULE							module;
			PFN_XInputGetCapabilities		GetCapabilities;
			PFN_XInputGetState				GetState;
			PFN_XInputSetState				SetState;
		} xinput;

		struct {
			HMODULE							module;
		} kernel32;

		struct {
			HMODULE							module;
		} user32;

		struct {
			HMODULE							module;
			PFN_SetProcessDpiAwareness		SetProcessDpiAwareness_;
			PFN_GetDpiForMonitor			GetDpiForMonitor_;
		} shcore;

		struct {
			HMODULE							module;
			PFN_RtlCaptureStackBackTrace	RtlCaptureStackBackTrace_;
		} ntdll;
	};

	static PlatformContext g_context;

	static bool LoadLibraries()
	{
		if (!axVerifyFmt(GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
							GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
							(LPCSTR)&g_context,
							&g_context.instance), "Failed to retrieve application module handle"))
		{
			return false;
		}

		g_context.kernel32.module = LoadLibraryA("kernel32.dll");
		if (!axVerifyFmt(g_context.kernel32.module, "Failed to load kernel32.dll"))
		{
			return false;
		}

		g_context.user32.module = LoadLibraryA("user32.dll");
		if (!axVerifyFmt(g_context.user32.module, "Failed to load user32.dll"))
		{
			return false;
		}

		{
			const char* xinput_dlls[] = {
				"xinput1_4.dll",
				"xinput1_3.dll",
				"xinput1_2.dll",
				"xinput1_1.dll",
			};
			for (int i = 0; i <  std::size(xinput_dlls); i++)
			{
				g_context.xinput.module = LoadLibraryA(xinput_dlls[i]);
				if (g_context.xinput.module)
				{
					g_context.xinput.GetCapabilities = (PFN_XInputGetCapabilities)GetProcAddress(g_context.xinput.module, "XInputGetCapabilities");
					g_context.xinput.GetState = (PFN_XInputGetState)GetProcAddress(g_context.xinput.module, "XInputGetState");
					g_context.xinput.SetState = (PFN_XInputSetState)GetProcAddress(g_context.xinput.module, "XInputSetState");
					break;
				}
			}
			axStrongAssertFmt(g_context.xinput.module, "Failed to load xinput.dll");
		}

		g_context.shcore.module = LoadLibraryA("shcore.dll");
		if (axVerifyFmt(g_context.shcore.module, "Failed to load shcore.dll"))
		{
			g_context.shcore.SetProcessDpiAwareness_ = (PFN_SetProcessDpiAwareness)GetProcAddress(g_context.shcore.module, "SetProcessDpiAwareness");
			g_context.shcore.GetDpiForMonitor_ = (PFN_GetDpiForMonitor)GetProcAddress(g_context.shcore.module, "GetDpiForMonitor");
		}

		g_context.ntdll.module = LoadLibraryA("ntdll.dll");
		if (axVerifyFmt(g_context.ntdll.module, "Failed to load ntdll.dll"))
		{
			g_context.ntdll.RtlCaptureStackBackTrace_ = (PFN_RtlCaptureStackBackTrace)GetProcAddress(g_context.ntdll.module, "RtlCaptureStackBackTrace");
		}

		return true;
	}

	static void FreeLibraries()
	{
		if (g_context.xinput.module)
			FreeLibrary(g_context.xinput.module);

		if (g_context.kernel32.module)
			FreeLibrary(g_context.xinput.module);

		if (g_context.user32.module)
			FreeLibrary(g_context.user32.module);

		if (g_context.shcore.module)
			FreeLibrary(g_context.shcore.module);

		if (g_context.ntdll.module)
			FreeLibrary(g_context.ntdll.module);
	}

	static bool CreatePlatformWindow(PlatformWindowImpl& window, const PlatformWindowCreateParams& params)
	{
		HWND hwnd = CreateWindowEx(
			0,								// Optional window styles
			WNDCLASSNAME,					// Window class
			params.title,					// Window text
			WS_OVERLAPPEDWINDOW,			// Window style
			CW_USEDEFAULT, CW_USEDEFAULT,	// Position
			1366, 768,						// Size
			NULL,							// Parent window
			NULL,							// Menu
			g_context.instance,				// Application instance
			NULL							// optional param
		);

		SetPropA(hwnd, "APEX", &window);

		return window.window = hwnd;
	}

	static void RequestCloseWindow(PlatformWindowImpl& window)
	{
		window.shouldQuit = true;
	}

	static LRESULT WindowProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		PlatformWindowImpl* window = (PlatformWindowImpl*)GetPropA(wnd, "APEX");
		if (!window)
		{
			return DefWindowProc(wnd, msg, wParam, lParam);
		}

		switch (msg)
		{
		case WM_CLOSE:
			{
				RequestCloseWindow(*window);
				return 0;
			}
		case WM_SIZE:
			{
				
			}
			// TODO: Mouse events
			// TODO: Key events
		}

		return DefWindowProc(wnd, msg, wParam, lParam);
	}

	bool PollGamepadEvents(GamepadState& gamepad)
	{
		XINPUT_STATE state {};
		DWORD dwResult = XInputGetState(gamepad.index, &state);
		if (dwResult != ERROR_SUCCESS)
		{
			// TODO: Lost connection. Destroy gamepad state
			return false;
		}

		const WORD buttons[] = {
			XINPUT_GAMEPAD_A,
			XINPUT_GAMEPAD_B,
			XINPUT_GAMEPAD_X,
			XINPUT_GAMEPAD_Y,
			XINPUT_GAMEPAD_LEFT_SHOULDER,
			XINPUT_GAMEPAD_RIGHT_SHOULDER,
			XINPUT_GAMEPAD_LEFT_THUMB,
			XINPUT_GAMEPAD_RIGHT_THUMB,
			XINPUT_GAMEPAD_BACK,
			XINPUT_GAMEPAD_START,
			XINPUT_GAMEPAD_DPAD_UP,
			XINPUT_GAMEPAD_DPAD_DOWN,
			XINPUT_GAMEPAD_DPAD_LEFT,
			XINPUT_GAMEPAD_DPAD_RIGHT,
		};

		for (size_t i = 0; i < std::size(buttons); i++)
		{
			gamepad.buttons[i] = state.Gamepad.wButtons & buttons[i] ? ButtonState::Pressed : ButtonState::Released;
		}

		gamepad.leftStick.x = state.Gamepad.sThumbLX / 32767.f;
		gamepad.leftStick.y = state.Gamepad.sThumbLY / 32767.f;
		gamepad.rightStick.x = state.Gamepad.sThumbRX / 32767.f;
		gamepad.rightStick.y = state.Gamepad.sThumbRY / 32767.f;
		gamepad.leftTrigger = state.Gamepad.bLeftTrigger / 127.f;
		gamepad.rightTrigger = state.Gamepad.bRightTrigger / 127.f;

		return true;
	}

	void DetectGamepads()
	{
		for (DWORD index = 0; index < XUSER_MAX_COUNT; index++)
		{
			u32 j;
			for (j = 0; j < g_context.inputManager.GetGamepadsConnectedCount(); j++)
			{
				if (g_context.inputManager.GetGamepad(j).index == index)
				{
					break;
				}
			}
			if (j < g_context.inputManager.GetGamepadsConnectedCount()) // Gamepad is already added
			{
				continue;
			}

			XINPUT_CAPABILITIES xcap;
			if (ERROR_SUCCESS != XInputGetCapabilities(index, XINPUT_FLAG_GAMEPAD, &xcap)) // Gamepad is not connected
			{
				if (g_context.inputManager.GetGamepad(index).index != (u32)-1)
					g_context.inputManager.OnGamepadDisconnected(index);
				continue;
			}

			g_context.inputManager.OnGamepadConnected(index);
		}
	}

	void PlatformWindow::Show() const
	{
		ShowWindow(m_impl->window, SW_SHOW);
	}

	void PlatformWindow::GetSize(u32* width, u32* height) const
	{
		*width = m_impl->width;
		*height = m_impl->height;
	}

	void PlatformWindow::GetPosition(u32* posX, u32* posY) const
	{
		*posX = m_impl->lastPosX;
		*posY = m_impl->lastPosY;
	}

	PlatformHandle PlatformWindow::GetOsHandle() const { return m_impl->window; }

	PlatformHandle PlatformWindow::GetOsApplicationHandle() const
	{
		return g_context.instance;
	}

	bool PlatformWindow::ShouldQuit() const { return m_impl->shouldQuit; }

	bool PlatformManager::Init(const PlatformManagerInitParams& params)
	{
		// ZeroMemory(&g_context, sizeof(PlatformContext));

		if (!LoadLibraries())
		{
			return false;
		}

		WNDCLASS wc{};
		wc.lpfnWndProc = WindowProc;
		wc.lpszClassName = WNDCLASSNAME;
		wc.hInstance = g_context.instance;
		RegisterClass(&wc);

		if (!axVerifyFmt(CreatePlatformWindow(g_context.mainWindow, params.windowParams), "Failed to create main window"))
		{
			return false;
		}
		g_context.mainWindowWrapper.SetImpl(&g_context.mainWindow);

		DetectGamepads();

		QueryPerformanceFrequency(&g_context.timerFrequency);

		return true;
	}

	void PlatformManager::Shutdown()
	{
		CloseWindow(g_context.mainWindow.window);
		FreeLibraries();
	}

	InputManager& PlatformManager::GetInputManager()
	{
		return g_context.inputManager;
	}

	PlatformWindow& PlatformManager::GetMainWindow()
	{
		return g_context.mainWindowWrapper;
	}

	void PlatformManager::PollEvents()
	{
		MSG msg;
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				RequestCloseWindow(g_context.mainWindow);
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		for (u32 i = 0; i < InputManager::GAMEPAD_MAX_COUNT; i++)
		{
			if (g_context.inputManager.IsGamepadConnected(i))
				PollGamepadEvents(g_context.inputManager.m_gamepads[i]);
		}
	}

	PlatformTimer::PlatformTimer() = default;

	PlatformTimer::~PlatformTimer() = default;

	void PlatformTimer::Start()
	{
		LARGE_INTEGER liStartTime;
		QueryPerformanceCounter(&liStartTime);
		m_startTime = liStartTime.QuadPart;
	}

	void PlatformTimer::End()
	{
		LARGE_INTEGER liEndTime;
		QueryPerformanceCounter(&liEndTime);
		m_endTime = liEndTime.QuadPart;
	}

	void PlatformTimer::Restart()
	{
		m_startTime = m_endTime;
	}

	u64 PlatformTimer::GetElapsedMicroseconds() const
	{
		u64 elapsedMicroseconds = (m_endTime - m_startTime);
		elapsedMicroseconds *= 1000000;
		elapsedMicroseconds /= g_context.timerFrequency.QuadPart;
		return elapsedMicroseconds;
	}

	f32 PlatformTimer::GetElapsedSeconds() const
	{
		return static_cast<float>(GetElapsedMicroseconds()) / 1e6f;
	}
}

#endif // APEX_PLATFORM_WIN32
