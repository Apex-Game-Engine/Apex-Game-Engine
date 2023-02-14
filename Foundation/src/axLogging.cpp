#include "Foundation/axLogging.h"
#include "Foundation/axTypes.h"


namespace apex {
namespace logging {

	namespace detail
	{
		constexpr const char* LOG_MSG_FORMAT = "[%s::(%s):%d] <%s> :: %s";
		char MSG_BUF[1024] {};

		const char* LOG_LEVEL_STR[static_cast<u64>(LogLevel::_MAX_ENUM_)] =
		{
			"Verbose",
			"Debug",
			"Info",
			"Warn",
			"Error",
			"Critical",
		};

		const char* format_log_msg(const LogMsg& log_msg)
		{
			(void)sprintf_s(MSG_BUF, LOG_MSG_FORMAT,
				log_msg.file, log_msg.funcsig, log_msg.lineno,
				LOG_LEVEL_STR[static_cast<u64>(log_msg.level)],
				log_msg.msg
			);

			return MSG_BUF;
		}

		Logger* g_logger;
	}


	void Logger::log(LogLevel level, const char* file, const char* funcsig, u32 lineno, const char* msg) const
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
		detail::g_logger = new Logger;
	}

	Logger& Logger::get()
	{
		return *detail::g_logger;
	}

	void Logger::log(ISink* sink, LogLevel level, const char* file, const char* funcsig, u32 lineno, const char* msg)
	{
	}

	void Logger::log(ISink* sink, const LogMsg& log_msg)
	{
	}

	void IConsoleSink::log(const LogMsg& log_msg)
	{
		printf("%s\n", detail::format_log_msg(log_msg));
	}
}
}
