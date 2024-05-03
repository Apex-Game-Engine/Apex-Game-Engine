#pragma once
#include <mutex>

#include "Core/Platform.h"

namespace apex {
namespace concurrency {

	template <typename T>
    concept lockable = requires(T t)
    {
	    { t.lock() } -> std::same_as<void>;
		{ t.unlock() } -> std::same_as<void>;
	};

	template <typename T>
	concept try_lockable = lockable && requires(T t)
	{
		{ t.try_lock() } -> std::same_as<bool>;
	};

	namespace detail
	{
		template <typename Lock0_t, typename Lock1_t>
		int _lock(Lock0_t& lock0, Lock1_t& lock1)
		{
			lock0.lock();

			if (!lock1.try_lock())
			{
				lock0.unlock();
				_THREAD_PAUSE();
				return true; // try again
			}

			return false; // lock acquired, stop trying
		}
	}

	// Null lock for placeholder in case of single threaded use
	struct NullLock
	{
		void lock() {}
		bool try_lock() { return true; }
		void unlock() {}
	};

	// SpinLock implementation as described by Jason Gregory in his book
	// Game Engine Architecture 3rd ed. pgs. 319-320
	struct SpinLock
	{
		void lock();
		bool try_lock();
		void unlock();

	private:
		std::atomic_flag m_flag{};
	};

	// ReentrantLock32 implementation as described by Jason Gregory in his book
	// Game Engine Architecture 3rd ed. pgs. 322-323
	struct ReentrantLock32
	{
		void lock();
		bool try_lock();
		void unlock();

	private:
		std::atomic<size_t> m_atomic{0};
		uint32_t m_refCount{0};
	};

	// Reader-Writer lock implementation as described and left as exercise by Jason Gregory
	// in his book Game Engine Architecture 3rd ed. pgs. 324-325
	struct ReaderWriterLock32
	{
		void acquireRead();
		bool tryAcquireRead();
		void releaseRead();

		void lock();
		bool try_lock();
		void unlock();

	private:
		std::atomic<uint32_t> m_readerCount{0};
		static constexpr uint32_t WRITE_MODE = 0x80000000ui32;
	};

	struct RWLock32ReadOnlyWrapper
	{
		explicit RWLock32ReadOnlyWrapper(ReaderWriterLock32& rw_lock) : m_rwLock(rw_lock) {}

		void lock()
		{
			m_rwLock.acquireRead();
		}

		bool try_lock()
		{
			return m_rwLock.tryAcquireRead();
		}

		void unlock()
		{
			m_rwLock.releaseRead();
		}

	private:
		ReaderWriterLock32& m_rwLock;
	};

	template <typename... LockN_t>
	struct LockGuard
	{
		explicit LockGuard(LockN_t&... locks) : m_locks(locks...)
		{
			while (detail::_lock(locks...)) { /* keep retrying */ }
		}

		~LockGuard()
		{
			std::apply([](LockN_t&... locks) { (..., locks.unlock()); }, m_locks);
		}
		
		LockGuard(const LockGuard&) = delete;
		LockGuard& operator = (const LockGuard&) = delete;

		LockGuard(LockGuard&&) = default;
		LockGuard& operator = (LockGuard&&) = default;

	private:
		std::tuple<LockN_t&...> m_locks;
		
	};

	template <typename Lock_t>
	struct LockGuard<Lock_t>
	{
		explicit LockGuard(Lock_t& lock) : m_lock(lock)
		{
			m_lock.lock();
		}

		~LockGuard() noexcept
		{
			m_lock.unlock();
		}

		LockGuard(const LockGuard&) = delete;
		LockGuard& operator = (const LockGuard&) = delete;

		LockGuard(LockGuard&&) = default;
		LockGuard& operator = (LockGuard&&) = default;

	private:
		Lock_t& m_lock;
	};

	template <>
	struct LockGuard<>; // Empty parameter pack for LockGuard will result in compilation error


	//template <typename Lock0_t, typename Lock1_t>
	//struct LockGuard<Lock0_t, Lock1_t>
	//{
	//	explicit LockGuard(Lock0_t& lock0, Lock1_t lock1) : m_lock0(lock)
	//	{
	//		m_lock.lock();
	//	}

	//	~LockGuard() noexcept
	//	{
	//		m_lock.unlock();
	//	}

	//	LockGuard(const LockGuard&) = delete;
	//	LockGuard& operator = (const LockGuard&) = delete;

	//	LockGuard(LockGuard&&) = default;
	//	LockGuard& operator = (LockGuard&&) = default;

	//private:
	//	Lock0_t& m_lock0;
	//	Lock1_t& m_lock1;
	//};

}
	namespace cncy = concurrency;
}

#define ReaderOnlyLock_withlineno(var, lineno) apex::concurrency::ReaderWriterLock32 underlyingRWLock ## lineno; \
	apex::concurrency::RWLock32ReadOnlyWrapper var { underlyingRWLock ## lineno }
#define ReaderOnlyLock_withlineno1(var, lineno) ReaderOnlyLock_withlineno(var, lineno)
#define ReaderOnlyLock(var) ReaderOnlyLock_withlineno1(var, __LINE__)