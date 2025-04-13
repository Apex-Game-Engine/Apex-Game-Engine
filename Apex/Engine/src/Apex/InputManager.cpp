#include "apex_pch.h"
#include "Apex/InputManager.h"

namespace apex {

	InputManager* InputManager::s_pInstance = nullptr;

	static const char* InputStateStrings[] = {
		"Released",
		"Pressed",
	};

	void InputManager::initialize()
	{
		axAssertFmt(s_pInstance == nullptr, "InputManager already initialized!");
		s_pInstance = this;
	}

	InputState InputManager::getKeyState(KeyCode key) const
	{
		return m_keyStates[static_cast<size_t>(key)];
	}

	InputState InputManager::getMouseButtonState(MouseButton button) const
	{
		return m_mouseButtonStates[static_cast<size_t>(button)];
	}

	math::Vector2 InputManager::getMousePosition() const
	{
		return m_mousePosition;
	}

	math::Vector2 InputManager::getMouseDelta() const
	{
		return m_mouseDelta;
	}

	void InputManager::setKeyState(KeyCode key, InputState state)
	{
		m_keyStates[static_cast<size_t>(key)] = state;
	}

	void InputManager::setMouseButtonState(MouseButton button, InputState state)
	{
		m_mouseButtonStates[static_cast<size_t>(button)] = state;
	}

	void InputManager::setMousePosition(math::Vector2 const& normalized_mouse_pos)
	{
		m_mouseDelta = normalized_mouse_pos - m_mousePosition;
		m_mousePosition = normalized_mouse_pos;
	}

	void InputManager::setMouseWheelDeltaV(f32 delta)
	{
		m_mouseWheelDelta.y = delta;
	}

	void InputManager::setMouseWheelDeltaH(f32 value)
	{
		m_mouseWheelDelta.x = value;
	}
}
