#pragma once
#include "Core/Types.h"

namespace apex {

	struct AxHandle
	{
		AxHandle() = default;
		AxHandle(size_t size);
		~AxHandle() = default;

		void*  m_cachedPtr { nullptr };
		uint32 m_memoryPoolIdx {};
		// uint32 m_memoryBlockIdx {};

		void release();

		bool isValid() { return m_cachedPtr != nullptr; }
		size_t getBlockSize() const;

		template <typename T>
		T* getAs()
		{
			return static_cast<T*>(m_cachedPtr);
		}

	};

	template <typename T>
	AxHandle make_handle()
	{
		return AxHandle(sizeof(T));
	}

	template <typename T> requires (std::is_array_v<T> && std::extent_v<T> == 0)
	AxHandle make_handle(const size_t size)
	{
		return AxHandle(sizeof(std::remove_extent_t<T>) * size);
	}
}
