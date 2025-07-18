#include "Memory/PoolAllocator.h"

#include "Core/Asserts.h"

namespace apex::mem {

	namespace {
		struct Block
		{
			Block *pNext;
		};
	}

	PoolAllocator::PoolAllocator(void* p_begin, size_t size, u32 block_size)
	{
		initialize(p_begin, size, block_size);
	}

	void PoolAllocator::initialize(void* p_begin, size_t size, u32 block_size)
	{
		axAssertFmt(p_begin != nullptr, "Invalid memory address!");
		axAssert(size > block_size);

		m_basePtr = p_begin;
		m_allocPtr = m_basePtr;
		m_blockSize = block_size;
		m_numTotalBlocks = static_cast<u32>(size / static_cast<size_t>(m_blockSize));
		m_numFreeBlocks = m_numTotalBlocks;

		reset();
	}

	void* PoolAllocator::allocate(size_t size)
	{
		axAssertFmt(m_basePtr != nullptr, "Pool allocator not initialized!");
		axAssertFmt(size <= m_blockSize, "Allocation size cannot be larger than pool element size!");

		if (m_numFreeBlocks == 0)
			return nullptr;

		// return the current block pointed to by allocPtr
		void* ret = m_allocPtr;
		--m_numFreeBlocks;

		// update allocPtr to next block
		Block *blockPtr = static_cast<Block*>(m_allocPtr);
		if (blockPtr->pNext == nullptr)
		{
			m_allocPtr = static_cast<u8*>(m_allocPtr) + m_blockSize;
		}
		else
		{
			m_allocPtr = blockPtr->pNext;
		}

		return ret;
	}

	void* PoolAllocator::allocate(size_t size, size_t align)
	{
		axAssertFmt(false, "Not implemented yet!");
		axAssertFmt(align <= 16, "");

		return allocate(size);
	}

	void PoolAllocator::free(void* ptr)
	{
		axAssertFmt(containsPointer(ptr), "Input memory is NOT managed by this Pool!");

		Block *blockPtr = static_cast<Block*>(ptr);
		blockPtr->pNext = static_cast<Block*>(m_allocPtr);
		m_allocPtr = ptr;
		++m_numFreeBlocks;
	}

	void PoolAllocator::reset()
	{
		memset(m_basePtr, 0, static_cast<size_t>(m_numTotalBlocks) * m_blockSize);
		m_allocPtr = m_basePtr;
		m_numFreeBlocks = m_numTotalBlocks;
	}

	void PoolAllocator::shutdown()
	{
		m_basePtr = nullptr;
		m_allocPtr = nullptr;
		m_blockSize = 0;
		m_numTotalBlocks = 0;
		m_numFreeBlocks = 0;
	}
}
