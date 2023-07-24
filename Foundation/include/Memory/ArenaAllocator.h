#pragma once
#include "IMemoryTracker.h"

namespace apex {
namespace memory {

	class ArenaAllocator : public IMemoryTracker
	{
	public:
		void initialize(void* p_begin, size_t size);
		[[nodiscard]] void* allocate(size_t size);
		[[nodiscard]] void* allocate(size_t size, size_t align);
		// void free(void *p_ptr);
		void reset();

#ifdef APEX_ENABLE_MEMORY_TRACKING
		[[nodiscard]] size_t getTotalCapacity() const override { return m_capacity; }
		[[nodiscard]] size_t getCurrentUsage() const override { return m_offset; }
#endif

	private:
		void *m_pBase { nullptr };
		size_t m_offset {};
		size_t m_capacity {};

#ifdef APEX_ENABLE_TESTS
		friend class ArenaAllocatorTest;
#endif
	};

}
}
