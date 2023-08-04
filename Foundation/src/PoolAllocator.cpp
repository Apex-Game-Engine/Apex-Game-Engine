#include "Memory/PoolAllocator.h"

#include "Core/Asserts.h"

namespace apex::memory {

	namespace {
		struct Block
		{
			Block *pNext;
		};
	}

	PoolAllocator::PoolAllocator(void* p_begin, size_t size, uint32 block_size)
	{
		initialize(p_begin, size, block_size);
	}

	void PoolAllocator::initialize(void* p_begin, size_t size, uint32 block_size)
	{
		axAssert(size > block_size);

		m_pBase = p_begin;
		m_allocPtr = m_pBase;
		m_blockSize = block_size;
		m_numTotalBlocks = static_cast<uint32>(size / static_cast<size_t>(m_blockSize));
		m_numFreeBlocks = m_numTotalBlocks;

		reset();
	}

	void* PoolAllocator::allocate(size_t size)
	{
		axAssertMsg(m_pBase != nullptr, "Pool allocator not initialized!");
		axAssertMsg(size <= m_blockSize, "Allocation size cannot be larger than pool element size!");

		if (m_numFreeBlocks == 0)
			return nullptr;

		// return the current block pointed to by allocPtr
		void* ret = m_allocPtr;
		--m_numFreeBlocks;

		// update allocPtr to next block
		Block *blockPtr = static_cast<Block*>(m_allocPtr);
		if (blockPtr->pNext == nullptr)
		{
			m_allocPtr = static_cast<uint8*>(m_allocPtr) + m_blockSize;
		}
		else
		{
			m_allocPtr = blockPtr->pNext;
		}

		return ret;
	}

	void* PoolAllocator::allocate(size_t size, size_t align)
	{
		axAssertMsg(false, "Not implemented yet!");
		axAssertMsg(align <= 16, "");

		return allocate(size);
	}

	void PoolAllocator::free(void* ptr)
	{
		axAssertMsg(containsPointer(ptr), "Input memory is NOT managed by this Pool!");

		Block *blockPtr = static_cast<Block*>(ptr);
		blockPtr->pNext = static_cast<Block*>(m_allocPtr);
		m_allocPtr = ptr;
		++m_numFreeBlocks;
	}

	void PoolAllocator::reset()
	{
		memset(m_pBase, 0, static_cast<size_t>(m_numTotalBlocks) * m_blockSize);
		m_allocPtr = m_pBase;
		m_numFreeBlocks = m_numTotalBlocks;
	}

}
