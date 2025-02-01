#include "Memory/ArenaAllocator.h"

#include "Core/Asserts.h"
#include "Memory/MemoryManagerImpl.h"

namespace apex::memory {

	void ArenaAllocator::initialize(void* p_begin, size_t size)
	{
		m_pBase = p_begin;
		m_capacity = size;
		m_offset = 0;
	}

	void* ArenaAllocator::allocate(size_t size)
	{
		axAssertFmt(m_pBase != nullptr, "Allocator not initialized!");
		axAssertFmt(m_capacity - m_offset > size, "Allocator overflow!");

		void* top = &static_cast<uint8*>(m_pBase)[m_offset];

		m_offset += size;

		return top;
	}

	void* ArenaAllocator::allocate(size_t size, size_t align)
	{
		const size_t actualAllocSize = size + align;

		axAssertFmt(m_pBase != nullptr, "Allocator not initialized!");
		axAssertFmt(m_capacity - m_offset > actualAllocSize, "Allocator overflow!");

		void* top = &static_cast<uint8*>(m_pBase)[m_offset];
		top = detail::shift_and_align_pointer(top, align);

		m_offset += actualAllocSize;

		return top;
	}

	void ArenaAllocator::reset()
	{
		m_offset = 0;
	}

}
