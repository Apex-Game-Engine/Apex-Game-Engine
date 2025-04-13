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

	class PlatformManager
	{
	public:
		static bool Init(const PlatformManagerInitParams& params);
		static void Shutdown();

		static InputManager& GetInputManager();
		static PlatformWindow& GetMainWindow();

		static void PollEvents();
	};

}
}
