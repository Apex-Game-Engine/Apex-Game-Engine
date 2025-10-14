#include "Core/Logging.h"
#include "Core/Types.h"
#include "Core/Console.h"

#ifdef _WIN32
#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>
#endif

namespace apex {
namespace logging {

	namespace detail
	{
		constexpr const char* LOG_MSG_ANSI_FORMAT = "{} [{}] \033[0m ({}:{}) <{}> :: {}\n";
		constexpr const char* LOG_MSG_FORMAT = "[{}] ({}:{}) <{}> :: {}\n";
		char MSG_BUF[2048] {};

		constexpr const char* get_log_level_str(LogLevel log_level)
		{
			switch (log_level) {
			case LogLevel::Trace:		return "Trace";
			case LogLevel::Debug:		return "Debug";
			case LogLevel::Info:		return "Info";
			case LogLevel::Warn:		return "Warn";
			case LogLevel::Error:		return "Error";
			case LogLevel::Critical:	return "Critical";
			default:					return "";
			}
		};

		const char* format_log_msg(const LogMsg& log_msg)
		{
			auto [pEnd, size] = fmt::format_to_n(MSG_BUF, sizeof(MSG_BUF),
				LOG_MSG_FORMAT,
				get_log_level_str(log_msg.level),
				log_msg.filename,
				log_msg.lineno,
				log_msg.funcsig,
				log_msg.msg);
			*pEnd = '\0';

			return MSG_BUF;
		}

		constexpr const char* get_ansi_color_for_level(LogLevel level)
		{
			switch (level)
			{
			case LogLevel::Trace: return "\033[0m";
			case LogLevel::Debug: return "\033[97m";
			case LogLevel::Info: return "\033[92m";
			case LogLevel::Warn: return "\033[93m";
			case LogLevel::Error: return "\033[41m\033[97m";
			case LogLevel::Critical: return "\033[101m\033[30m";
			default: return "";
			}
		}

		const char* format_log_message_ansi(const LogMsg& log_msg)
		{
			auto [pEnd, size] = fmt::format_to_n(MSG_BUF, sizeof(MSG_BUF),
				LOG_MSG_ANSI_FORMAT,
				get_ansi_color_for_level(log_msg.level),
				get_log_level_str(log_msg.level),
				log_msg.filename,
				log_msg.lineno,
				log_msg.funcsig,
				log_msg.msg);
			*pEnd = '\0';

			return MSG_BUF;
		}

		Logger s_logger;
		Console* s_console;
		DebugOutputSink s_debugSink;
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

	void Logger::Init(bool enable_console)
	{
		if (enable_console)
		{
			static char s_consoleBuffer[sizeof(Console)];
			detail::s_stdoutSink.console = new (s_consoleBuffer) Console("Apex Debug Console");
		}
		else
		{
			detail::s_logger.addSink(&detail::s_debugSink);
		}
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

	void DebugOutputSink::log(const LogMsg& log_msg)
	{
		OutputDebugStringA(log_msg.formatted);
		fmt::detail::print(stdout, { log_msg.formatted });
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
			detail::get_log_level_str(log_msg.level),
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
