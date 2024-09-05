#pragma once

#include <vector>

#include "Core/Types.h"
#include "Concurrency/Concurrency.h"

namespace apex::concurrency
{
	struct NullLock;
}

namespace apex {
namespace logging {

	enum class LogLevel : apex::uint8
	{
		Trace    = 0,
		Debug    = 1,
		Info     = 2,
		Warn     = 3,
		Error    = 4,
		Critical = 5,

		_MAX_ENUM_
	};

	struct LogMsg
	{
		const char *filepath;
		const char *funcsig;
		const char *msg;
		uint32 lineno;
		LogLevel level;

		LogMsg(const char *filepath, const char *funcsig, const char *msg, uint32 lineno, LogLevel level);

		const char *filename;
	};

	struct ISink
	{
		virtual ~ISink() = default;
		virtual void log(const LogMsg& log_msg) = 0;
	};

	struct Logger
	{
		void log(LogLevel level, const char *file, const char *funcsig, uint32 lineno, const char *msg) const;
		void log(const LogMsg& log_msg) const;

		void addSink(ISink* sink);

		static void initialize();
		static Logger& get();

		static void log(ISink* sink, LogLevel level, const char *file, const char *funcsig, uint32 lineno, const char *msg);
		static void log(ISink* sink, const LogMsg& log_msg);

		ISink* m_sinks[8];
		size_t m_sinkCount {0};
	};

	struct IConsoleSink : public ISink
	{
		virtual void log(const LogMsg& log_msg) override;
	};
	using ConsoleSink_st = IConsoleSink;

	template <typename Lock_t>
	struct ConsoleSink final : public IConsoleSink
	{
		void log(const LogMsg& log_msg) override
		{
			concurrency::LockGuard lock{ m_lock };
			IConsoleSink::log(log_msg);
		}

	private:
		Lock_t m_lock;
	};


	struct IRingBufferSink : public ISink
	{
		virtual void log(const LogMsg& log_msg) override;
	};
	using RingBufferSink_st = IRingBufferSink;

	template <typename Lock_t>
	struct RingBufferSink final : public IRingBufferSink
	{
		void log(const LogMsg& log_msg) override
		{
			concurrency::LockGuard lock{ m_lock };
			IRingBufferSink::log(log_msg);
		}

		char *m_pBuffer;
		size_t m_capacity;

	private:
		Lock_t m_lock;
	};

}
}

#include "Formatting.h"

#define axLogLevel(level, file, funcsig, lineno, msg)          apex::logging::Logger::get().log(level, file, funcsig, lineno, msg)
#define axLogLevelFmt(level, file, funcsig, lineno, _fmt, ...) axLogLevel(level, file, funcsig, lineno, apex::format(_fmt, ##__VA_ARGS__).data())

#define axLog(msg)      axLogLevel(apex::logging::LogLevel::Trace, __FILE__, __FUNCTION__, __LINE__, msg)
#define axDebug(msg)    axLogLevel(apex::logging::LogLevel::Debug, __FILE__, __FUNCTION__, __LINE__, msg)
#define axInfo(msg)     axLogLevel(apex::logging::LogLevel::Info, __FILE__, __FUNCTION__, __LINE__, msg)
#define axWarn(msg)     axLogLevel(apex::logging::LogLevel::Warn, __FILE__, __FUNCTION__, __LINE__, msg)
#define axError(msg)    axLogLevel(apex::logging::LogLevel::Error, __FILE__, __FUNCTION__, __LINE__, msg)
#define axCritical(msg) axLogLevel(apex::logging::LogLevel::Critical, __FILE__, __FUNCTION__, __LINE__, msg)

#define axLogFmt(_fmt, ...)      axLogLevelFmt(apex::logging::LogLevel::Trace, __FILE__, __FUNCTION__, __LINE__, _fmt, ##__VA_ARGS__)
#define axDebugFmt(_fmt, ...)    axLogLevelFmt(apex::logging::LogLevel::Debug, __FILE__, __FUNCTION__, __LINE__, _fmt, ##__VA_ARGS__)
#define axInfoFmt(_fmt, ...)     axLogLevelFmt(apex::logging::LogLevel::Info, __FILE__, __FUNCTION__, __LINE__, _fmt, ##__VA_ARGS__)
#define axWarnFmt(_fmt, ...)     axLogLevelFmt(apex::logging::LogLevel::Warn, __FILE__, __FUNCTION__, __LINE__, _fmt, ##__VA_ARGS__)
#define axErrorFmt(_fmt, ...)    axLogLevelFmt(apex::logging::LogLevel::Error, __FILE__, __FUNCTION__, __LINE__, _fmt, ##__VA_ARGS__)
#define axCriticalFmt(_fmt, ...) axLogLevelFmt(apex::logging::LogLevel::Critical, __FILE__, __FUNCTION__, __LINE__, _fmt, ##__VA_ARGS__)
