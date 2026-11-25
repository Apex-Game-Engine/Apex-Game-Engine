#include "Core/Asserts.h"
#include "Platform/InputManager.h"
#include "Platform/PlatformManager.h"
#include "Platform/Timer.h"
#include "String/AxString.h"

#if APEX_PLATFORM_WIN32

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <xinput.h>
#include <ShellScalingApi.h>
#include <commdlg.h>

namespace apex::plat {

	#define WNDCLASSNAME STR(apex::plat::PlatformWindowImpl)

	// xinput.dll
	typedef DWORD (WINAPI *PFN_XInputGetCapabilities)(DWORD,DWORD,XINPUT_CAPABILITIES*);
	typedef DWORD (WINAPI *PFN_XInputGetState)(DWORD,XINPUT_STATE*);
	typedef DWORD (WINAPI *PFN_XInputSetState)(DWORD,XINPUT_VIBRATION*);
	#define XInputGetCapabilities g_context.xinput.GetCapabilities
	#define XInputGetState g_context.xinput.GetState
	#define XInputSetState g_context.xinput.SetState

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

	class PlatformWindowImpl
	{
	public:
		HWND window;
		bool shouldQuit;
		u32 width, height; 
		u32 lastPosX, lastPosY;
		static LRESULT WindowProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam);
	};

	struct PlatformContext
	{
		HINSTANCE			instance;
		PlatformWindowImpl	mainWindow;
		PlatformWindow      mainWindowWrapper;
		InputManager		inputManager;
		LARGE_INTEGER		timerFrequency;
		PFN_WindowProc		userWindowProc;
		AxString			workingDirectory;
		AxString			executablePath;

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
			for (auto xinput_dll : xinput_dlls)
			{
				g_context.xinput.module = LoadLibraryA(xinput_dll);
				if (g_context.xinput.module)
				{
					g_context.xinput.GetCapabilities = (PFN_XInputGetCapabilities)GetProcAddress(g_context.xinput.module, "XInputGetCapabilities");
					g_context.xinput.GetState = (PFN_XInputGetState)GetProcAddress(g_context.xinput.module, "XInputGetState");
					g_context.xinput.SetState = (PFN_XInputSetState)GetProcAddress(g_context.xinput.module, "XInputSetState");
					break;
				}
			}
			axCriticalAssertFmt(g_context.xinput.module, "Failed to load xinput.dll");
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
			FreeLibrary(g_context.kernel32.module);

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

	static void FixupPlatformPath(char *path, u32 len)
	{
		for (u32 i = 0; i < len; i++)
		{
			char c = path[i];
			path[i] = c == '\\' ? '/' : c;
		}
	}

	LRESULT PlatformWindowImpl::WindowProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		PlatformWindowImpl* window = (PlatformWindowImpl*)GetPropA(wnd, "APEX");
		if (!window)
		{
			return DefWindowProc(wnd, msg, wParam, lParam);
		}

		if (g_context.userWindowProc && g_context.userWindowProc(wnd, msg, wParam, lParam))
			return true;

		switch (msg)
		{
		case WM_CLOSE:
			{
				RequestCloseWindow(*window);
				return 0;
			}
		case WM_SIZE:
			{
				if (wParam == SIZE_MINIMIZED)
				{
					window->width = 0;
					window->height = 0;
				}
				else
				{
					RECT rect;
					GetClientRect(window->window, &rect);
					window->width = rect.right - rect.left;
					window->height = rect.bottom - rect.top;
				}
				break;
			}
		case WM_KEYDOWN:
		case WM_KEYUP:
			{
				KeyCode key = static_cast<KeyCode>(wParam);
				g_context.inputManager.m_keyboard.keys[(u32)key] = ButtonState::Pressed;
			}
			// TODO: Mouse events
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

		RECT rc;
		GetClientRect(m_impl->window, &rc);
		m_impl->width = rc.right - rc.left;
		m_impl->height = rc.bottom - rc.top;
		m_impl->lastPosX = rc.left;
		m_impl->lastPosY = rc.top;
	}

	bool PlatformWindow::IsMinimized() const
	{
		return m_impl->width == 0 || m_impl->height == 0;
	}

	Dim2D PlatformWindow::GetDimensions() const
	{
		return { m_impl->width, m_impl->height };
	}

	void PlatformWindow::GetDimensions(u32* width, u32* height) const
	{
		*width = m_impl->width;
		*height = m_impl->height;
	}

	Dim2D PlatformWindow::GetPosition() const
	{
		return { m_impl->lastPosX, m_impl->lastPosY };
	}

	void PlatformWindow::GetPosition(u32* posX, u32* posY) const
	{
		*posX = m_impl->lastPosX;
		*posY = m_impl->lastPosY;
	}

	PlatformHandle PlatformWindow::GetOsHandle() const
	{
		return m_impl->window;
	}

	PlatformHandle PlatformWindow::GetOsApplicationHandle() const
	{
		return g_context.instance;
	}

	bool PlatformWindow::ShouldQuit() const
	{
		return m_impl->shouldQuit;
	}

	bool PlatformManager::Init(const PlatformManagerInitParams& params)
	{
		// ZeroMemory(&g_context, sizeof(PlatformContext));

		if (!LoadLibraries())
		{
			return false;
		}

		char pathBuf[MAX_PATH];
		DWORD pathLen = GetModuleFileNameA(NULL, pathBuf, std::size(pathBuf));
		if (axVerify(pathLen != 0))
		{
			FixupPlatformPath(pathBuf, pathLen);
			g_context.executablePath = AxString{ pathBuf, pathLen };
		}

		pathLen = GetCurrentDirectoryA(std::size(pathBuf), pathBuf);
		if (axVerify(pathLen != 0))
		{
			FixupPlatformPath(pathBuf, pathLen);
			g_context.workingDirectory = AxString{ pathBuf, pathLen };
		}

		WNDCLASS wc{};
		wc.lpfnWndProc = PlatformWindowImpl::WindowProc;
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
		g_context.executablePath.reset();
		g_context.workingDirectory.reset();
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

	PFN_WindowProc PlatformManager::SetUserWindowProc(PFN_WindowProc proc)
	{
		const auto prevProc = g_context.userWindowProc;
		g_context.userWindowProc = proc;
		return prevProc;
	}

	AxStringView PlatformManager::GetExecutablePath()
	{
		return g_context.executablePath;
	}

	AxStringView PlatformManager::GetWorkingDirectory()
	{
		return g_context.workingDirectory;
	}

	AxString PlatformManager::GetEnvVar(const char* name)
	{
		AxString env;
		DWORD sz = GetEnvironmentVariableA(name, NULL, 0);
		env.resize(sz-1);
		GetEnvironmentVariableA(name, env.data(), env.size() + 1);
		return env;
	}

	bool PlatformManager::ShowOpenFileDialog(char* const out_filepath, u32 buflen)
	{
		char filepath[MAX_PATH] = {0};

		OPENFILENAME ofn;
		ZeroMemory(&ofn, sizeof(ofn));

		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = g_context.mainWindow.window;
		ofn.lpstrFile = filepath;
		ofn.nMaxFile = MAX_PATH;
		ofn.lpstrFilter = "Apex Mesh Files (*.axmesh)\0*.axmesh\0All Files (*.*)\0*.*\0";
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

		if (GetOpenFileName(&ofn))
		{
			AxStringView(filepath).copy(out_filepath, buflen);
			return true;
		}
		return false;
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

	void PlatformManager::WaitForEvents()
	{
		MSG msg;
		if (GetMessage(&msg, nullptr, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			RequestCloseWindow(g_context.mainWindow);
		}
	}

	void Timer::Start()
	{
		LARGE_INTEGER liStartTime;
		QueryPerformanceCounter(&liStartTime);
		m_startTime = liStartTime.QuadPart;
	}

	void Timer::End()
	{
		LARGE_INTEGER liEndTime;
		QueryPerformanceCounter(&liEndTime);
		m_endTime = liEndTime.QuadPart;
	}

	void Timer::Restart()
	{
		m_startTime = m_endTime;
	}

	u64 Timer::GetElapsedMicroseconds() const
	{
		u64 elapsedMicroseconds = (m_endTime - m_startTime);
		elapsedMicroseconds *= 1000000;
		elapsedMicroseconds /= g_context.timerFrequency.QuadPart;
		return elapsedMicroseconds;
	}

	f32 Timer::GetElapsedSeconds() const
	{
		return static_cast<float>(GetElapsedMicroseconds()) / 1e6f;
	}
}

#endif // APEX_PLATFORM_WIN32
