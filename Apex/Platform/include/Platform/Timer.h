#pragma once
#include "Core/Types.h"

namespace apex {
namespace plat {

	class PlatformTimer
	{
	public:
		PlatformTimer();
		~PlatformTimer();

		void Start();
		void End();
		void Restart();
		u64 GetElapsedMicroseconds() const;
		f32 GetElapsedSeconds() const;

	private:
		u64 m_startTime;
		u64 m_endTime;
	};

	class ScopedTimer
	{
	public:
		explicit ScopedTimer(PlatformTimer& timer)
			: m_timer(&timer)
		{
			m_timer->Start();
		}

		~ScopedTimer()
		{
			if (m_timer)
				m_timer->End();
		}

		ScopedTimer(const ScopedTimer&) = delete;
		ScopedTimer& operator=(const ScopedTimer&) = delete;

		ScopedTimer(ScopedTimer&& other) noexcept
			: m_timer(other.m_timer) 
		{
			other.m_timer = nullptr;
		}

		ScopedTimer& operator=(ScopedTimer&& other) noexcept
		{
			m_timer = other.m_timer;
			other.m_timer = nullptr;
			return *this;
		}

	private:
		PlatformTimer* m_timer;
	};
		
}
}
