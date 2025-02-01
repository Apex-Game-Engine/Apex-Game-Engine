﻿#pragma once
#include "Containers/AxArray.h"
#include "Math/Vector2.h"

namespace apex {

	enum class InputState : u8
	{
		Released,
		Pressed,
	};

	enum class KeyCode : u32
	{
		KeyUnknown = 0,
		KeySpace = 32,
		KeyApostrophe = 39,  /* ' */
		KeyComma = 44,       /* , */
		KeyMinus = 45,       /* - */
		KeyPeriod = 46,      /* . */
		KeySlash = 47,       /* / */
		Key0 = 48,
        Key1 = 49,
		Key2 = 50,
		Key3 = 51,
		Key4 = 52,
        Key5 = 53,
        Key6 = 54,
		Key7 = 55,
		Key8 = 56,
		Key9 = 57,
		KeySemicolon = 59,   /* ; */
		KeyEqual = 61,       /* = */
		KeyA = 65,
		KeyB = 66,
        KeyC = 67,
		KeyD = 68,
		KeyE = 69,
		KeyF = 70,
		KeyG = 71,
		KeyH = 72,
		KeyI = 73,
		KeyJ = 74,
        KeyK = 75,
		KeyL = 76,
		KeyM = 77,
		KeyN = 78,
		KeyO = 79,
        KeyP = 80,
		KeyQ = 81,
		KeyR = 82,
		KeyS = 83,
		KeyT = 84,
        KeyU = 85,
        KeyV = 86,
		KeyW = 87,
		KeyX = 88,
		KeyY = 89,
		KeyZ = 90,
		KeyLeftBracket = 91,  /* [ */
		KeyBackslash = 92,    /* \ */
        KeyRightBracket = 93, /* ] */
		KeyGraveAccent = 96,  /* ` */
		KeyEscape = 256,
		KeyEnter = 257,
		KeyTab = 258,
		KeyBackspace = 259,
		KeyInsert = 260,
		KeyDelete = 261,
		KeyRight = 262,
		KeyLeft = 263,
		KeyDown = 264,
		KeyUp = 265,
		KeyPageUp = 266,
		KeyPageDown = 267,
		KeyHome = 268,
		KeyEnd = 269,
		KeyCapsLock = 280,
		KeyScrollLock = 281,
		KeyNumLock = 282,
		KeyPrintScreen = 283,
		KeyPause = 284,
		KeyF1 = 290,
		KeyF2 = 291,
		KeyF3 = 292,
		KeyF4 = 293,
		KeyF5 = 294,
		KeyF6 = 295,
		KeyF7 = 296,
		KeyF8 = 297,
		KeyF9 = 298,
		KeyF10 = 299,
		KeyF11 = 300,
		KeyF12 = 301,
		KeyF13 = 302,
        KeyF14 = 303,
		KeyF15 = 304,
		KeyF16 = 305,
		KeyF17 = 306,
		KeyF18 = 307,
		KeyF19 = 308,
		KeyF20 = 309,
		KeyF21 = 310,
		KeyF22 = 311,
		KeyF23 = 312,
		KeyF24 = 313,
		KeyNumpad0 = 320,
		KeyNumpad1 = 321,
		KeyNumpad2 = 322,
		KeyNumpad3 = 323,
		KeyNumpad4 = 324,
		KeyNumpad5 = 325,
		KeyNumpad6 = 326,
		KeyNumpad7 = 327,
		KeyNumpad8 = 328,
		KeyNumpad9 = 329,
		KeyNumpadDecimal = 330,
		KeyNumpadDivide = 331,
		KeyNumpadMultiply = 332,
		KeyNumpadSubtract = 333,
		KeyNumpadAdd = 334,
		KeyNumpadEnter = 335,
		KeyNumpadEqual = 336,
		KeyLeftShift = 340,
		KeyLeftControl = 341,
		KeyLeftAlt = 342,
		KeyLeftSuper = 343,
		KeyRightShift = 344,
		KeyRightControl = 345,
		KeyRightAlt = 346,
		KeyRightSuper = 347,
		KeyMenu = 348,

		COUNT,
	};

	enum class KeyModifiers : u32
	{
		ModShift = 0x0001,
		ModControl = 0x0002,
		ModAlt = 0x0004,
		ModSuper = 0x0008,
		ModCapsLock = 0x0010,
		ModNumLock = 0x0020
	};

	enum class MouseButton : u8
	{
		MouseBtnUnknown = 0,
		MouseBtn1 = 1,
		MouseBtn2 = 2,
		MouseBtn3 = 3,
		MouseBtn4 = 4,
		MouseBtn5 = 5,
		MouseBtn6 = 6,
		MouseBtn7 = 7,
		MouseBtn8 = 8,

		COUNT,

		MouseBtnLeft = MouseBtn1,
		MouseBtnRight = MouseBtn2,
		MouseBtnMiddle = MouseBtn3,
	};


	class APEX_API InputManager
	{
	public:
		InputManager() = default;

		void initialize();

		static InputManager* Instance() { return s_pInstance; }

		InputState getKeyState(KeyCode key) const;
		InputState getMouseButtonState(MouseButton button) const;
		math::Vector2 getMousePosition() const;
		math::Vector2 getMouseDelta() const;

	protected:
		inline void setKeyState(KeyCode key, InputState state);
		inline void setMouseButtonState(MouseButton button, InputState state);
		inline void setMousePosition(math::Vector2 const& normalized_mouse_pos);
		inline void setMouseWheelDeltaV(f32 delta);
		inline void setMouseWheelDeltaH(f32 value);

	private:
		static InputManager *s_pInstance; // pimpl idiom

		AxStaticArray<InputState, static_cast<size_t>(KeyCode::COUNT)> m_keyStates;
		AxStaticArray<InputState, static_cast<size_t>(MouseButton::COUNT)> m_mouseButtonStates;
		math::Vector2 m_mousePosition;
		math::Vector2 m_mouseDelta;
		math::Vector2 m_mouseWheelDelta;

		friend class Application;
	};

}

