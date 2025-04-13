#pragma once

#ifdef APEX_EXPORTS
#	ifdef APEX_PLATFORM_WIN32
#		define APEX_API __declspec(dllexport)
#	else
#		warn "This platform does not support dll exports"
#		define APEX_API 
#	endif
#else
#	ifdef APEX_PLATFORM_WIN32
#		define APEX_API __declspec(dllimport)
#	else
#		warn "This platform does not support dll imports"
#		define APEX_API 
#	endif
#endif
