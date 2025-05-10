#pragma once
#include "Core/Types.h"

namespace apex {
namespace plat {

	class InputManager;

	typedef void* PlatformHandle;

	struct PlatformWindowCreateParams
	{
		u32 width;
		u32 height;
		const char* title;
		// fullscreen, decoration, callbacks
	};

	struct PlatformWindowImpl;

	class PlatformWindow
	{
	public:
		constexpr PlatformWindow() = default;

		void Show() const;

		void Minimize();
		void Maximize();

		void GetSize(u32* width, u32* height) const;
		void GetPosition(u32* posX, u32* posY) const;

		PlatformHandle GetOsHandle() const;
		PlatformHandle GetOsApplicationHandle() const;
		bool ShouldQuit() const;

	protected:
		constexpr void SetImpl(PlatformWindowImpl* impl) { m_impl = impl; }

	private:
		PlatformWindowImpl* m_impl;

		friend class PlatformManager;
	};

	struct PlatformManagerInitParams
	{
		PlatformWindowCreateParams windowParams;
	};

#if APEX_PLATFORM_WIN32
	typedef s64 (*PFN_WindowProc)(void* wnd, u32 msg, u64 wParam, s64 lParam);
#endif

	class PlatformManager
	{
	public:
		static bool Init(const PlatformManagerInitParams& params);
		static void Shutdown();

		static InputManager& GetInputManager();
		static PlatformWindow& GetMainWindow();
	#ifdef APEX_PLATFORM_WIN32
		static PFN_WindowProc SetUserWindowProc(PFN_WindowProc proc);
	#endif

		static void PollEvents();
	};

}
}
