﻿#pragma once

#include "Core/Types.h"
#include "Concurrency/Concurrency.h"
#include "Formatting.h"

namespace apex
{
	class Console;
}

namespace apex::concurrency
{
	struct NullLock;
}

namespace apex {
namespace logging {

	enum class LogLevel : apex::u8
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
		u32 lineno;
		LogLevel level;
		const char *filename;
		const char* formatted;

		LogMsg(const char *filepath, const char *funcsig, const char *msg, u32 lineno, LogLevel level);
	};

	struct ISink
	{
		virtual ~ISink() = default;
		virtual void log(const LogMsg& log_msg) = 0;
	};

	struct Logger
	{
		void log(LogLevel level, const char *file, const char *funcsig, u32 lineno, const char *msg) const;
		void log(const LogMsg& log_msg) const;

		void addSink(ISink* sink);
		void removeSink(ISink* sink);

		static void AttachConsole(Console* console);
		static void Init();
		static Logger& get();

		static void log(ISink* sink, LogLevel level, const char *file, const char *funcsig, u32 lineno, const char *msg);
		static void log(ISink* sink, const LogMsg& log_msg);

		ISink* m_sinks[8];
		size_t m_sinkCount {0};
	};

	struct ConsoleSink : public ISink
	{
		void log(const LogMsg& log_msg) override;

		Console* console;
	};
	using ConsoleSink_st = ConsoleSink;

	template <typename Lock_t>
	struct ConsoleSink_mt final : public ConsoleSink
	{
		void log(const LogMsg& log_msg) override
		{
			concurrency::LockGuard lock{ m_lock };
			ConsoleSink::log(log_msg);
		}

	private:
		Lock_t m_lock;
	};


	struct IRingBufferSink : public ISink
	{
		void log(const LogMsg& log_msg) override;
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

	bool DisplayErrorMessageBox(const LogMsg& log_msg);

	inline bool LogVerifyFailedError(const char *file, const char *funcsig, apex::u32 lineno, const char *msg)
	{
		Logger::get().log(LogLevel::Error, file, funcsig, lineno, msg);
		return DisplayErrorMessageBox(LogMsg(file, funcsig, msg, lineno, LogLevel::Error));
	}

}
}

#define axLogLevel(level, file, funcsig, lineno, msg)          apex::logging::Logger::get().log(level, file, funcsig, lineno, msg)
#define axLogLevelFmt(level, file, funcsig, lineno, _fmt, ...) axLogLevel(level, file, funcsig, lineno, apex::format(_fmt, ##__VA_ARGS__).data())

#define axLog(msg)      axLogLevel(apex::logging::LogLevel::Trace, __FILE__, __FUNCTION__, __LINE__, msg)
#define axDebug(msg)    axLogLevel(apex::logging::LogLevel::Debug, __FILE__, __FUNCTION__, __LINE__, msg)
#define axInfo(msg)     axLogLevel(apex::logging::LogLevel::Info, __FILE__, __FUNCTION__, __LINE__, msg)
#define axWarn(msg)     axLogLevel(apex::logging::LogLevel::Warn, __FILE__, __FUNCTION__, __LINE__, msg)
#define axError(msg)    axLogLevel(apex::logging::LogLevel::Error, __FILE__, __FUNCTION__, __LINE__, msg)
#define axCritical(msg) (axLogLevel(apex::logging::LogLevel::Critical, __FILE__, __FUNCTION__, __LINE__, msg), DEBUG_BREAK())

#define axLogFmt(_fmt, ...)      axLogLevelFmt(apex::logging::LogLevel::Trace, __FILE__, __FUNCTION__, __LINE__, _fmt, ##__VA_ARGS__)
#define axDebugFmt(_fmt, ...)    axLogLevelFmt(apex::logging::LogLevel::Debug, __FILE__, __FUNCTION__, __LINE__, _fmt, ##__VA_ARGS__)
#define axInfoFmt(_fmt, ...)     axLogLevelFmt(apex::logging::LogLevel::Info, __FILE__, __FUNCTION__, __LINE__, _fmt, ##__VA_ARGS__)
#define axWarnFmt(_fmt, ...)     axLogLevelFmt(apex::logging::LogLevel::Warn, __FILE__, __FUNCTION__, __LINE__, _fmt, ##__VA_ARGS__)
#define axErrorFmt(_fmt, ...)    (axLogLevelFmt(apex::logging::LogLevel::Error, __FILE__, __FUNCTION__, __LINE__, _fmt, ##__VA_ARGS__), DEBUG_BREAK())
#define axCriticalFmt(_fmt, ...) (axLogLevelFmt(apex::logging::LogLevel::Critical, __FILE__, __FUNCTION__, __LINE__, _fmt, ##__VA_ARGS__), DEBUG_BREAK())

#define axVerifyFailedError(msg)          (apex::logging::LogVerifyFailedError(__FILE__, __FUNCTION__, __LINE__, msg))
#define axVerifyFailedErrorFmt(_fmt, ...) (apex::logging::LogVerifyFailedError(__FILE__, __FUNCTION__, __LINE__, apex::format(_fmt, ##__VA_ARGS__).data()))
