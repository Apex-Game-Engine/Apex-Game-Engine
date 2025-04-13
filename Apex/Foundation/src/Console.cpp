#include "Core/Console.h"

#if APEX_PLATFORM_WIN32
#	include <windows.h>
#endif

namespace apex {

	Console::Console(const char* title)
	{
	#if APEX_PLATFORM_WIN32
		AllocConsole();
		AttachConsole(GetCurrentProcessId());
		SetConsoleTitle(TEXT(title));

		m_hConsoleIn = GetStdHandle(-10);
		m_hConsoleOut = GetStdHandle(-11);
		m_hConsoleErr = GetStdHandle(-12);

		DWORD consoleMode;
		GetConsoleMode(m_hConsoleOut, &consoleMode);
		SetConsoleMode(m_hConsoleOut, consoleMode | ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING );

		GetConsoleMode(m_hConsoleErr, &consoleMode);
		SetConsoleMode(m_hConsoleErr, consoleMode | ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING );
	#else
	#	error "Console not supported on this platform"
	#endif
	}

	Console::~Console()
	{
	#if APEX_PLATFORM_WIN32
		FreeConsole();
	#endif
	}

	void Console::Write(const char* msg)
	{
	#if APEX_PLATFORM_WIN32
		DWORD charsWritten;
		WriteConsole(m_hConsoleOut, msg, strlen(msg), &charsWritten, nullptr);
	#endif
	}

	void Console::Error(const char* msg)
	{
	#if APEX_PLATFORM_WIN32
		DWORD charsWritten;
		WriteConsole(m_hConsoleErr, msg, strlen(msg), &charsWritten, nullptr);
	#endif
	}
}
