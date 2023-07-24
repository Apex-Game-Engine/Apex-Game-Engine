#include "Memory/StackAllocator.h"

#include "Core/Asserts.h"

namespace apex::memory {

	StackAllocator::StackAllocator(void* p_begin, size_t size)
	: m_pBase(p_begin)
	, m_capacity(size)
	{
	}

	void StackAllocator::initialize(void* p_begin, size_t size)
	{
		m_pBase = p_begin;
		m_capacity = size;
		m_offset = 0;
	}

	void* StackAllocator::allocate(size_t size)
	{
		axAssertMsg(m_pBase != nullptr, "Stack allocator not initialized!");
		axAssertMsg(m_capacity - m_offset > size, "Stack allocator overflow!");

		void* top = &static_cast<uint8*>(m_pBase)[m_offset];

		m_offset += size;

		return top;
	}

	void* StackAllocator::allocate(size_t size, size_t align)
	{
		return nullptr;
	}

	void StackAllocator::free(void* p_ptr)
	{
		axAssert(p_ptr != nullptr);
		
	}

	void StackAllocator::reset()
	{
		m_offset = 0;
	}

	size_t StackAllocator::getTotalCapacity() const
	{
		// TODO
		return 0;
	}

	size_t StackAllocator::getCurrentUsage() const
	{
		// TODO
		return 0;
	}
}
