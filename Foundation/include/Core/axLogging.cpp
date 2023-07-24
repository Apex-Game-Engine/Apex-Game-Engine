#include "axLogging.h"

apex::logging::LogMsg::LogMsg(const char* filepath, const char* funcsig, const char* msg, uint32 lineno, LogLevel level)
: filepath(filepath)
, funcsig(funcsig)
, msg(msg)
, lineno(lineno)
, level(level)
{
	int filepathlen = strlen(filepath);
	int i;
	for (i = filepathlen - 1; i >= 0; i--)
	{
		if (filepath[i] == '\\')
		{
			break;
		}
	}

	filename = filepath + i;
}
