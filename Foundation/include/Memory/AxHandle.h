#pragma once
#include "Core/Logging.h"
#include "Core/Types.h"

namespace apex {
	namespace memory
	{
		class MemoryManagerTest;
	}

	class AxHandle
	{
	public:
		AxHandle() = default;
		AxHandle(size_t size);
		~AxHandle() = default;

		AxHandle(AxHandle&& other) noexcept
		: m_cachedPtr(other.m_cachedPtr)
		, m_memoryPoolIdx(other.m_memoryPoolIdx)
		{}

		AxHandle operator=(AxHandle&& other) noexcept
		{
			m_cachedPtr = other.m_cachedPtr;
			m_memoryPoolIdx = other.m_memoryPoolIdx;
		}

		bool isValid() { return m_cachedPtr != nullptr; }
		size_t getBlockSize() const;

		template <typename T>
		T* getAs()
		{
			return static_cast<T*>(m_cachedPtr);
		}

		AxHandle(AxHandle const&) = delete;
		AxHandle& operator=(AxHandle const&) = delete;

	protected:
		void free();
		void allocate(size_t size);
		friend class AxManagedClass;

	private:
		void*  m_cachedPtr { nullptr };
		uint32 m_memoryPoolIdx {};
		// uint32 m_memoryBlockIdx {};

		friend class memory::MemoryManagerTest;
	};

	template <typename T>
	AxHandle make_handle()
	{
		return AxHandle(sizeof(T));
	}

	template <typename T> requires (std::is_array_v<T> && std::extent_v<T> == 0)
	AxHandle make_handle(const size_t size)
	{
		if constexpr (std::is_trivially_destructible_v<T>)
			return AxHandle(sizeof(std::remove_extent_t<T>) * size);
		else
			return AxHandle(sizeof(size_t) + sizeof(std::remove_extent_t<T>) * size);
	}
}
