#include "Foundation/axConcurrency.h"
#include "Foundation/axAsserts.h"

namespace apex {
namespace concurrency {

	void SpinLock::lock()
	{
		// Keep spinning until successfully acquired the lock
		while (!try_lock())
		{
			// Reduce power consumption
			// Uses _mm_pause on Intel x86 CPUs. This allows the other hyperthread
			// to take precedence in the current core's pipeline and reduces the
			// memory bashing at the atomic's location.
			_THREAD_PAUSE();
		}
	}

	bool SpinLock::try_lock()
	{
		// Use a read-acquire fence here to ensure that no other read instructions 
		// to the atomic in this thread are shifted before the atomic read, and the
		// writes to the atomic by all the other threads are visible to this thread.
		// Ensures that all subsequent reads by this thread will be valid.
		bool alreadyLocked = m_flag.test_and_set(std::memory_order_acquire);

		return !alreadyLocked;
	}

	void SpinLock::unlock()
	{
		// Use a write-release fence here to ensure that no other write instructions
		// to the atomic in this thread are shifted after the atomic write, and all
		// the writes to the atomic by this thread are visible to all other threads.
		// Ensures that all prior writes by this thread will be valid/fully committed.
		m_flag.clear(std::memory_order_release);
	}

	void ReentrantLock32::lock()
	{
		size_t tid = std::hash<std::thread::id>{}(std::this_thread::get_id());

		// Check if the current thread holds the lock.
		// Relaxed semantics can be used here as we are not acquiring the lock here
		if (m_atomic.load(std::memory_order_relaxed) != tid)
		{
			// Keep spinning until the lock is acquired
			size_t unlockValue = 0;
			while (!m_atomic.compare_exchange_weak(
				             unlockValue,
				             tid,
				             std::memory_order_relaxed, // Acquire fence below handles memory ordering!
				             std::memory_order_relaxed))
			{
				unlockValue = 0;
				_THREAD_PAUSE();
			}
		}
		// Increment counter to aloow verifying that lock and unlock are called in pairs
		++m_refCount;

		// Use an acquire fence to ensure that all the subsequent reads by this thread will be valid
		std::atomic_thread_fence(std::memory_order_acquire);
	}

	bool ReentrantLock32::try_lock()
	{
		size_t tid = std::hash<std::thread::id>{}(std::this_thread::get_id());

		bool acquired = false;

		if (m_atomic.load(std::memory_order_relaxed) == tid)
		{
			acquired = true;
		}
		else
		{
			size_t unlockValue = 0;
			acquired = m_atomic.compare_exchange_strong(
					            unlockValue,
					            tid,
					            std::memory_order_relaxed,
					            std::memory_order_relaxed);
		}

		if (acquired)
		{
			++m_refCount;
			std::atomic_thread_fence(std::memory_order_acquire);
		}
		return acquired;
	}

	void ReentrantLock32::unlock()
	{
		// Use a release fence to ensure that all prior writes by this thread will be valid/fully committed.
		std::atomic_thread_fence(std::memory_order_release);

		size_t tid = std::hash<std::thread::id>{}(std::this_thread::get_id());

		size_t actual = m_atomic.load(std::memory_order_relaxed);
		axAssertMsg(actual == tid, "Attempting to release ReentrantLock32 not help by current thread!");

		--m_refCount;
		if (m_refCount == 0)
		{
			// Since ref count is zero we are the final and only owner of the lock,
			// so we can safely release the lock.
			m_atomic.store(0, std::memory_order_relaxed);
		}
	}

	void ReaderWriterLock32::acquireRead()
	{
		uint32_t expected = m_readerCount.load(std::memory_order_relaxed);
		while (expected == WRITE_MODE)
		{
			expected = m_readerCount.load(std::memory_order_relaxed);
		}

		uint32_t desired = expected + 1;

		while (!m_readerCount.compare_exchange_weak(
			                  expected,
			                  desired,
			                  std::memory_order_relaxed,
			                  std::memory_order_relaxed))
		{
			desired = expected + 1;
			_THREAD_PAUSE();
		}

		std::atomic_thread_fence(std::memory_order_acquire);
	}

	bool ReaderWriterLock32::tryAcquireRead()
	{
		bool acquired = false;

		uint32_t expected = m_readerCount.load(std::memory_order_relaxed);
		if (expected == WRITE_MODE)
		{
			return false;
		}

		uint32_t desired = expected + 1;

		acquired = m_readerCount.compare_exchange_weak(
			                     expected,
			                     desired,
			                     std::memory_order_relaxed,
			                     std::memory_order_relaxed);

		if (acquired)
		{
			std::atomic_thread_fence(std::memory_order_acquire);
		}

		return acquired;
	}

	void ReaderWriterLock32::releaseRead()
	{

	}

	void ReaderWriterLock32::lock()
	{
		uint32_t expected = m_readerCount.load(std::memory_order_relaxed);
		while (expected != 0)
		{
			expected = m_readerCount.load(std::memory_order_relaxed);
		}

		uint32_t desired = WRITE_MODE;
		while (!m_readerCount.compare_exchange_weak(
			                  expected,
			                  desired,
			                  std::memory_order_acquire, // Using stricter memory ordering here since multiple writers may
			                                             // attempt to write the same value which can lead to ABA problem
			                  std::memory_order_relaxed))
		{
			expected = 0;
			_THREAD_PAUSE();
		}

		std::atomic_thread_fence(std::memory_order_acquire);
	}

	bool ReaderWriterLock32::try_lock()
	{
		bool acquired = false;

		uint32_t expected = m_readerCount.load(std::memory_order_relaxed);
		if (expected != 0)
		{
			return false;
		}

		uint32_t desired = WRITE_MODE;
		acquired = m_readerCount.compare_exchange_strong(
			                     expected,
			                     desired,
			                     std::memory_order_acquire, // Using stricter memory ordering here since multiple writers may
			                                                // attempt to write the same value which can lead to ABA problem
			                     std::memory_order_relaxed);

		if (acquired)
		{
			std::atomic_thread_fence(std::memory_order_acquire);
		}

		return acquired;
	}

	void ReaderWriterLock32::unlock()
	{

	}

}
}
