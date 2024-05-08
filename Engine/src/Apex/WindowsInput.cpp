#include "apex_pch.h"
#include "Apex/PlatformInput.h"

#ifdef APEX_PLATFORM_WIN32

#define NOMINMAX
#include <Windows.h>

namespace apex {

	KeyCode translateKeyCode(int32 key)
	{
		switch (key)
		{
		case VK_SPACE: return KeyCode::KeySpace;
		case VK_OEM_7: return KeyCode::KeyApostrophe;
		case VK_OEM_COMMA: return KeyCode::KeyComma;
		case VK_OEM_MINUS: return KeyCode::KeyMinus;
		case VK_OEM_PERIOD: return KeyCode::KeyPeriod;
		case VK_OEM_2: return KeyCode::KeySlash;
		case '0': return KeyCode::Key0;
        case '1': return KeyCode::Key1;
		case '2': return KeyCode::Key2;
		case '3': return KeyCode::Key3;
		case '4': return KeyCode::Key4;
        case '5': return KeyCode::Key5;
        case '6': return KeyCode::Key6;
		case '7': return KeyCode::Key7;
		case '8': return KeyCode::Key8;
		case '9': return KeyCode::Key9;
		case VK_OEM_1: return KeyCode::KeySemicolon;
		case 'A': return KeyCode::KeyA;
		case 'B': return KeyCode::KeyB;
        case 'C': return KeyCode::KeyC;
		case 'D': return KeyCode::KeyD;
		case 'E': return KeyCode::KeyE;
		case 'F': return KeyCode::KeyF;
		case 'G': return KeyCode::KeyG;
		case 'H': return KeyCode::KeyH;
		case 'I': return KeyCode::KeyI;
		case 'J': return KeyCode::KeyJ;
        case 'K': return KeyCode::KeyK;
		case 'L': return KeyCode::KeyL;
		case 'M': return KeyCode::KeyM;
		case 'N': return KeyCode::KeyN;
		case 'O': return KeyCode::KeyO;
        case 'P': return KeyCode::KeyP;
		case 'Q': return KeyCode::KeyQ;
		case 'R': return KeyCode::KeyR;
		case 'S': return KeyCode::KeyS;
		case 'T': return KeyCode::KeyT;
        case 'U': return KeyCode::KeyU;
        case 'V': return KeyCode::KeyV;
		case 'W': return KeyCode::KeyW;
		case 'X': return KeyCode::KeyX;
		case 'Y': return KeyCode::KeyY;
		case 'Z': return KeyCode::KeyZ;
		case VK_OEM_4: return KeyCode::KeyLeftBracket;
		case VK_OEM_5: return KeyCode::KeyBackslash;
        case VK_OEM_6: return KeyCode::KeyRightBracket;
		case VK_OEM_3: return KeyCode::KeyGraveAccent;
		case VK_ESCAPE: return KeyCode::KeyEscape;
		case VK_RETURN: return KeyCode::KeyEnter;
		case VK_TAB: return KeyCode::KeyTab;
		case VK_BACK: return KeyCode::KeyBackspace;
		case VK_INSERT: return KeyCode::KeyInsert;
		case VK_DELETE: return KeyCode::KeyDelete;
		case VK_RIGHT: return KeyCode::KeyRight;
		case VK_LEFT: return KeyCode::KeyLeft;
		case VK_DOWN: return KeyCode::KeyDown;
		case VK_UP: return KeyCode::KeyUp;
		case VK_PRIOR: return KeyCode::KeyPageUp;
		case VK_NEXT: return KeyCode::KeyPageDown;
		case VK_HOME: return KeyCode::KeyHome;
		case VK_END: return KeyCode::KeyEnd;
		case VK_CAPITAL: return KeyCode::KeyCapsLock;
		case VK_SCROLL: return KeyCode::KeyScrollLock;
		case VK_NUMLOCK: return KeyCode::KeyNumLock;
		case VK_SNAPSHOT: return KeyCode::KeyPrintScreen;
		case VK_PAUSE: return KeyCode::KeyPause;
		case VK_F1: return KeyCode::KeyF1;
		case VK_F2: return KeyCode::KeyF2;
		case VK_F3: return KeyCode::KeyF3;
		case VK_F4: return KeyCode::KeyF4;
		case VK_F5: return KeyCode::KeyF5;
		case VK_F6: return KeyCode::KeyF6;
		case VK_F7: return KeyCode::KeyF7;
		case VK_F8: return KeyCode::KeyF8;
		case VK_F9: return KeyCode::KeyF9;
		case VK_F10: return KeyCode::KeyF10;
		case VK_F11: return KeyCode::KeyF11;
		case VK_F12: return KeyCode::KeyF12;
		case VK_F13: return KeyCode::KeyF13;
        case VK_F14: return KeyCode::KeyF14;
		case VK_F15: return KeyCode::KeyF15;
		case VK_F16: return KeyCode::KeyF16;
		case VK_F17: return KeyCode::KeyF17;
		case VK_F18: return KeyCode::KeyF18;
		case VK_F19: return KeyCode::KeyF19;
		case VK_F20: return KeyCode::KeyF20;
		case VK_F21: return KeyCode::KeyF21;
		case VK_F22: return KeyCode::KeyF22;
		case VK_F23: return KeyCode::KeyF23;
		case VK_F24: return KeyCode::KeyF24;
		case VK_NUMPAD0: return KeyCode::KeyNumpad0;
		case VK_NUMPAD1: return KeyCode::KeyNumpad1;
		case VK_NUMPAD2: return KeyCode::KeyNumpad2;
		case VK_NUMPAD3: return KeyCode::KeyNumpad3;
		case VK_NUMPAD4: return KeyCode::KeyNumpad4;
		case VK_NUMPAD5: return KeyCode::KeyNumpad5;
		case VK_NUMPAD6: return KeyCode::KeyNumpad6;
		case VK_NUMPAD7: return KeyCode::KeyNumpad7;
		case VK_NUMPAD8: return KeyCode::KeyNumpad8;
		case VK_NUMPAD9: return KeyCode::KeyNumpad9;
		case VK_DECIMAL: return KeyCode::KeyNumpadDecimal;
		case VK_DIVIDE: return KeyCode::KeyNumpadDivide;
		case VK_MULTIPLY: return KeyCode::KeyNumpadMultiply;
		case VK_SUBTRACT: return KeyCode::KeyNumpadSubtract;
		case VK_ADD: return KeyCode::KeyNumpadAdd;
		case VK_LSHIFT: return KeyCode::KeyLeftShift;
		case VK_LCONTROL: return KeyCode::KeyLeftControl;
		case VK_LMENU: return KeyCode::KeyLeftAlt;
		case VK_LWIN: return KeyCode::KeyLeftSuper;
		case VK_RSHIFT: return KeyCode::KeyRightShift;
		case VK_RCONTROL: return KeyCode::KeyRightControl;
		case VK_RMENU: return KeyCode::KeyRightAlt;
		case VK_RWIN: return KeyCode::KeyRightSuper;
		}
		return KeyCode::KeyUnknown;
	}

	MouseButton translateMouseButton(int32 input)
	{
		switch (input)
		{
		case VK_LBUTTON: return MouseButton::MouseBtnLeft;
		case VK_RBUTTON: return MouseButton::MouseBtnRight;
		case VK_MBUTTON: return MouseButton::MouseBtnMiddle;
		case VK_XBUTTON1: return MouseButton::MouseBtn4;
		case VK_XBUTTON2: return MouseButton::MouseBtn5;
		}
		return MouseButton::MouseBtnUnknown;
	}
}

#endif
