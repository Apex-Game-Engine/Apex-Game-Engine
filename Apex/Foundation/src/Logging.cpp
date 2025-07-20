#include "Core/Logging.h"
#include "Core/Types.h"
#include "Core/Console.h"

#ifdef _WIN32
#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace apex {
namespace logging {

	namespace detail
	{
		constexpr const char* LOG_MSG_ANSI_FORMAT = "%s[%s::(%s):%d] <%s> :: %s\n\033[0m";
		constexpr const char* LOG_MSG_FORMAT = "[%s::(%s):%d] <%s> :: %s\n";
		char MSG_BUF[2048] {};

		const char* LOG_LEVEL_STR[static_cast<u64>(LogLevel::_MAX_ENUM_)] =
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
			(void)snprintf(MSG_BUF, sizeof(MSG_BUF), LOG_MSG_FORMAT,
				log_msg.filename, log_msg.funcsig, log_msg.lineno,
				LOG_LEVEL_STR[static_cast<u64>(log_msg.level)],
				log_msg.msg
			);

			return MSG_BUF;
		}

		const char* get_ansi_color_for_level(LogLevel level)
		{
			switch (level)
			{
			case LogLevel::Trace: return "\033[0m";
			case LogLevel::Debug: return "\033[97m";
			case LogLevel::Info: return "\033[92m";
			case LogLevel::Warn: return "\033[93m";
			case LogLevel::Error: return "\033[41m\033[97m";
			case LogLevel::Critical: return "\033[101m\033[30m";
			default: break;
			}
			return "";
		}

		const char* format_log_message_ansi(const LogMsg& log_msg)
		{
			(void)snprintf(MSG_BUF, sizeof(MSG_BUF), LOG_MSG_ANSI_FORMAT, get_ansi_color_for_level(log_msg.level),
				log_msg.filename, log_msg.funcsig, log_msg.lineno,
				LOG_LEVEL_STR[static_cast<u64>(log_msg.level)],
				log_msg.msg
			);

			return MSG_BUF;
		}

		Logger s_logger;
		ConsoleSink_st s_stdoutSink;
	}


	void Logger::log(LogLevel level, const char* file, const char* funcsig, u32 lineno, const char* msg) const
	{
		LogMsg logMsg { file, funcsig, msg, lineno, level };
		log(logMsg);
	}

	void Logger::log(const LogMsg& log_msg) const
	{
		for (size_t i = 0; i < m_sinkCount; i++)
		{
			m_sinks[i]->log(log_msg);
		}
	}

	void Logger::AttachConsole(Console* console)
	{
		detail::s_stdoutSink.console = console;
	}

	void Logger::addSink(ISink* sink)
	{
		if (m_sinkCount == std::size(m_sinks))
			KILL();

		m_sinks[m_sinkCount++] = sink;
	}

	void Logger::removeSink(ISink* sink)
	{
		for (size_t i = 0; i < m_sinkCount; i++)
		{
			if (m_sinks[i] == sink)
			{
				m_sinks[i] = m_sinks[m_sinkCount-1];
				m_sinks[m_sinkCount-1] = nullptr;
				m_sinkCount--;
				break;
			}
		}
	}

	void Logger::Init()
	{
		detail::s_logger.addSink(&detail::s_stdoutSink);
	}

	Logger& Logger::get()
	{
		return detail::s_logger;
	}

	void Logger::log(ISink* sink, LogLevel level, const char* file, const char* funcsig, u32 lineno, const char* msg)
	{
	}

	void Logger::log(ISink* sink, const LogMsg& log_msg)
	{
	}

	void ConsoleSink::log(const LogMsg& log_msg)
	{
		if (!console)
			return;

		auto msgStr = detail::format_log_message_ansi(log_msg);
		if (log_msg.level < LogLevel::Error)
		{
			console->Write(msgStr);
		}
		else
		{
			console->Error(msgStr);
		}
	}

	bool DisplayErrorMessageBox(const LogMsg& log_msg)
	{
	#ifdef _WIN32
		int res = MessageBoxA(
			NULL,
			log_msg.formatted,
			detail::LOG_LEVEL_STR[(int)log_msg.level],
			MB_ABORTRETRYIGNORE | MB_ICONERROR);
		if (res == IDABORT)
			exit(EXIT_FAILURE);
		if (res == IDRETRY)
			return false;
		if (res == IDIGNORE)
			return true;
	#endif
		return false;
	}

	LogMsg::LogMsg(const char* filepath, const char* funcsig, const char* msg, u32 lineno, LogLevel level)
	: filepath(filepath)
	, funcsig(funcsig)
	, msg(msg)
	, lineno(lineno)
	, level(level)
	, filename(nullptr)
	, formatted(nullptr)
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

		filename = filepath + i + 1;

		formatted = detail::format_log_msg(*this);
	}
}
}
