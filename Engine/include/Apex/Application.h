#pragma once

#include "Api.h"
#include "Game.h"
#include "InputManager.h"
#include "Memory/UniquePtr.h"

namespace apex {
	enum class InputState : uint8;
	enum class KeyCode : uint32;

	namespace gfx {
		class ForwardRenderer;
	}

	struct Window;

	enum class ApplicationState
	{
		eStopped,
		eRunning,
		ePaused
	};

	struct APEX_API Application
	{
		virtual ~Application() = default;

		static Application* Construct(uint32 width, uint32 height, const char* name, UniquePtr<Game>&& pGame);

		static Application* Instance()
		{
			return s_pInstance;
		}

		virtual void initialize() = 0;
		virtual void run() = 0;
		virtual void exit() = 0;
		virtual void shutdown() = 0;

		virtual Window* getWindow() = 0;
		virtual ApplicationState getState() = 0;
		virtual InputManager* getInputManager() = 0;
		virtual gfx::ForwardRenderer* getRenderer() = 0;

		static Application *s_pInstance;

	protected:
		void setKeyState(KeyCode key, InputState state);
		void setMouseButtonState(MouseButton button, InputState state);
		void setMousePosition(math::Vector2 const& normalized_mouse_pos);
		void setMouseWheelDeltaV(float32 value);
		void setMouseWheelDeltaH(float32 value);
	};

}
