﻿#pragma once
#include "IMemoryTracker.h"

namespace apex {
namespace mem {

	class ArenaAllocator
	{
	public:
		void initialize(void* p_begin, size_t size);
		[[nodiscard]] void* allocate(size_t size);
		[[nodiscard]] void* allocate(size_t size, size_t align);
		// void free(void *p_ptr);
		void reset();


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
