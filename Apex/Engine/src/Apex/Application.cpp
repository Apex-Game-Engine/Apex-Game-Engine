#include "apex_pch.h"
#include "Apex/Application.h"

#include "Apex/InputManager.h"

namespace apex {

	Application* Application::s_pInstance = nullptr;

	void Application::setKeyState(KeyCode key, InputState state)
	{
		getInputManager()->setKeyState(key, state);
	}

	void Application::setMouseButtonState(MouseButton button, InputState state)
	{
		getInputManager()->setMouseButtonState(button, state);
	}

	void Application::setMousePosition(math::Vector2 const& normalized_mouse_pos)
	{
		getInputManager()->setMousePosition(normalized_mouse_pos);
	}

	void Application::setMouseWheelDeltaV(f32 value)
	{
		getInputManager()->setMouseWheelDeltaV(value);
	}

	void Application::setMouseWheelDeltaH(f32 value)
	{
		getInputManager()->setMouseWheelDeltaH(value);
	}
}
