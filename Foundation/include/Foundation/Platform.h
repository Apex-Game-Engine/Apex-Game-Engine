#pragma once

#define _THREAD_PAUSE() _mm_pause()

#if defined(_MSC_VER)
#	define DEBUG_BREAK() __debugbreak()
#elif defined(__clang__) 
#	if defined(__has_builtin(__builtin_debugtrap))
#		define DEBUG_BREAK() __builtin_debugtrap()
#	elif defined(__has_builtin(__builtin_trap))
#		define DEBUG_BREAK() __builtin_trap()
#	endif
#elif defined(__GNUC__)
#	define DEBUG_BREAK() __builtin_trap()
#endif
#ifndef DEBUG_BREAK
#include <csignal>
#	if defined(SIGTRAP)
#		define DEBUG_BREAK() raise(SIGTRAP)
#	else
#		define DEBUG_BREAK() raise(SIGABRT)
#	endif
#endif
