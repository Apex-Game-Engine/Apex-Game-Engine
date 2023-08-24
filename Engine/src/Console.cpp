#include "apex_pch.h"
#include "Console.h"

#ifdef APEX_PLATFORM_WIN32
#	include <windows.h>
#endif

namespace apex {

	Console::Console(const char* title)
	{
	#ifdef APEX_PLATFORM_WIN32
		AllocConsole();
		AttachConsole(GetCurrentProcessId());
		SetConsoleTitle(TEXT(title));
	#else
	#	error "Console not supported on this platform"
	#endif
	}

	void Console::connect()
	{
		FILE* stream;
		(void)freopen_s(&stream, "CONIN$", "r", stdin);
		(void)freopen_s(&stream, "CONOUT$", "w+", stdout);
		(void)freopen_s(&stream, "CONOUT$", "w+", stderr);
	}
}
