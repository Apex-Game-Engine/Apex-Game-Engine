#include <windows.h>

#include "Core/Logging.h"
#include "Core/Types.h"


namespace apex {
namespace logging {

	namespace detail
	{
		constexpr const char* LOG_MSG_FORMAT = "[%s::(%s):%d] <%s> :: %s\n";
		char MSG_BUF[1024] {};

		const char* LOG_LEVEL_STR[static_cast<uint64>(LogLevel::_MAX_ENUM_)] =
		{
			"Trace",
			"Debug",
			"Info",
			"Warn",
			"Error",
			"Critical",
		};

		const char* format_log_msg(const LogMsg& log_msg)
		{
			(void)sprintf_s(MSG_BUF, LOG_MSG_FORMAT,
				log_msg.filename, log_msg.funcsig, log_msg.lineno,
				LOG_LEVEL_STR[static_cast<uint64>(log_msg.level)],
				log_msg.msg
			);

			return MSG_BUF;
		}

		Logger s_logger;
		ConsoleSink_st s_stdoutSink;
	}


	void Logger::log(LogLevel level, const char* file, const char* funcsig, uint32 lineno, const char* msg) const
	{
		LogMsg logMsg { file, funcsig, msg, lineno, level };
		log(logMsg);
	}

	void Logger::log(const LogMsg& log_msg) const
	{
		for (ISink* sink : m_sinks)
		{
			sink->log(log_msg);
		}
	}

	void Logger::initialize()
	{
		detail::s_logger.m_sinks.push_back(&detail::s_stdoutSink);
	}

	Logger& Logger::get()
	{
		return detail::s_logger;
	}

	void Logger::log(ISink* sink, LogLevel level, const char* file, const char* funcsig, uint32 lineno, const char* msg)
	{
	}

	void Logger::log(ISink* sink, const LogMsg& log_msg)
	{
	}

	void IConsoleSink::log(const LogMsg& log_msg)
	{
		auto msgStr = detail::format_log_msg(log_msg);
#if defined(APEX_CONFIG_DEBUG) || defined(APEX_CONFIG_DEBUGGAME) || defined(APEX_CONFIG_DEVELOPMENT)
		if (log_msg.level == LogLevel::Trace)
		{
			OutputDebugString(msgStr);
			return;
		}
#endif
		if (log_msg.level < LogLevel::Error)
		{
			(void)fprintf(stdout, "%s", msgStr);
		}
		else
		{
			(void)fprintf(stderr, "%s", msgStr);
		}
	}

	LogMsg::LogMsg(const char* filepath, const char* funcsig, const char* msg, uint32 lineno, LogLevel level)
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
}
}
