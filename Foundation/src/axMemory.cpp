#define APEX_ENABLE_MEMORY_LITERALS
#include "Foundation/axMemory.h"
#include "Foundation/detail/axMemory-ext.h"
#include <cstdlib>

namespace apex {
namespace memory {

	using namespace literals;

	/*//////////////////////////////////////////////
	               Arena Allocator
	//////////////////////////////////////////////*/

#ifdef APEX_ENABLE_MEMORY_TRACKING
#	define ALLOCATOR_MEMORY_TRACKER_ADD_ALLOC_INFO(size) m_memoryTracker.addAllocationInfo(size)
#	define ALLOCATOR_MEMORY_TRACKER_ADD_FREE_INFO(size) m_memoryTracker.addFreeInfo(size)
#else
#	define ALLOCATOR_MEMORY_TRACKER_ADD_ALLOC_INFO(size)
#	define ALLOCATOR_MEMORY_TRACKER_ADD_FREE_INFO(size)
#endif

	// Member functions
	void ArenaAllocator::initialize(void* p_begin, size_t size)
	{
		m_pBase = p_begin;
		m_capacity = size;
		m_offset = 0;
	}

	void* ArenaAllocator::allocate(size_t size)
	{
		axAssertMsg(m_pBase != nullptr, "Allocator not initialized!");
		axAssertMsg(m_capacity - m_offset > size, "Allocator overflow!");

		void* top = &static_cast<u8*>(m_pBase)[m_offset];

		m_offset += size;

		ALLOCATOR_MEMORY_TRACKER_ADD_ALLOC_INFO(size);

		return top;
	}

	void* ArenaAllocator::allocate(size_t size, size_t align)
	{
		const size_t actualAllocSize = size + align;

		axAssertMsg(m_pBase != nullptr, "Allocator not initialized!");
		axAssertMsg(m_capacity - m_offset > actualAllocSize, "Allocator overflow!");

		void* top = &static_cast<u8*>(m_pBase)[m_offset];
		top = detail::shift_and_align_pointer(top, align);

		m_offset += actualAllocSize;

		ALLOCATOR_MEMORY_TRACKER_ADD_ALLOC_INFO(actualAllocSize);

		return top;
	}

	void ArenaAllocator::free(void* p_ptr)
	{
		axCheckMsg(false, "Do not use free with Arena Allocator. "
			"This allocator type is meant to be used as scratch memory "
			"and should be reset rather than freed per allocation.");
		return;

		axAssert(p_ptr != nullptr);

		const i64 ptrOffset = static_cast<u8*>(p_ptr) - static_cast<u8*>(m_pBase);

		axAssertMsg(ptrOffset >= 0 && static_cast<u64>(ptrOffset) < m_offset, "Invalid pointer! Pointer not from allocator memory!");

		ALLOCATOR_MEMORY_TRACKER_ADD_FREE_INFO(m_offset - ptrOffset);

		if (m_pBase)
		{
			m_offset = ptrOffset;
		}

	}

	void ArenaAllocator::reset()
	{
		ALLOCATOR_MEMORY_TRACKER_ADD_FREE_INFO(m_offset);

		m_offset = 0;
	}

	size_t ArenaAllocator::getTotalCapacity()
	{
		return m_capacity;
	}

	size_t ArenaAllocator::getCurrentUsage()
	{
		return m_offset;
	}

	// Global functions
	void initialize(ArenaAllocator *p_allocator, void* p_begin, size_t size)
	{
		p_allocator->initialize(p_begin, size);
	}

	void reset(ArenaAllocator *p_allocator)
	{
		p_allocator->reset();
	}

	void* allocate(ArenaAllocator *p_allocator, size_t size)
	{
		return p_allocator->allocate(size);
	}
	
	void* allocate(ArenaAllocator *p_allocator, size_t size, size_t align)
	{
		return p_allocator->allocate(size, align);
	}

	void free(ArenaAllocator *p_allocator, void* p_ptr)
	{
		p_allocator->free(p_ptr);
	}


	/*//////////////////////////////////////////////
	               Stack Allocator
	//////////////////////////////////////////////*/

	// Global functions
	void initialize(StackAllocator* p_allocator, void* p_begin, size_t size)
	{
		p_allocator->m_pBase = p_begin;
		p_allocator->m_capacity = size;
		p_allocator->m_offset = 0;
	}

	void* allocate(StackAllocator* p_allocator, size_t size)
	{
		axAssertMsg(p_allocator->m_pBase != nullptr, "Stack allocator not initialized!");
		axAssertMsg(p_allocator->m_capacity - p_allocator->m_offset > size, "Stack allocator overflow!");

		void* top = &static_cast<u8*>(p_allocator->m_pBase)[p_allocator->m_offset];

		p_allocator->m_offset += size;

		return top;
	}

	void* allocate(StackAllocator* p_allocator, size_t size, size_t align)
	{
		return nullptr;
	}

	void free(StackAllocator* p_allocator, void* p_ptr)
	{
		axAssert(p_ptr != nullptr);
	}

	void reset(StackAllocator* p_allocator)
	{
		p_allocator->m_offset = 0;
	}

	// Member functions
	void StackAllocator::initialize(void* p_begin, size_t size)
	{
		memory::initialize(this, p_begin, size);
	}

	void* StackAllocator::allocate(size_t size)
	{
		return memory::allocate(this, size);
	}

	void* StackAllocator::allocate(size_t size, size_t align)
	{
		return memory::allocate(this, size, align);
	}

	void StackAllocator::free(void* p_ptr)
	{
		memory::free(this, p_ptr);
	}

	void StackAllocator::reset()
	{
		memory::reset(this);
	}


	// Pool Allocator

	void PoolAllocator::initialize(void* p_begin, size_t size)
	{
		m_pBase = p_begin;
		m_numTotalBlocks = static_cast<u32>(size / static_cast<size_t>(m_blockSize));
	}

	void* PoolAllocator::allocate(size_t size)
	{
		axAssertMsg(m_pBase != nullptr, "Pool allocator not initialized!");
		axAssertMsg(m_blockSize > size, "Allocation size cannot be larger than pool element size!");

		u8* ptr;

		if (m_freeListHead == 0)
		{
			size_t nextBlockIdx = m_numTotalBlocks - m_numFreeBlocks;
			ptr = static_cast<u8*>(m_pBase) + static_cast<u64>(nextBlockIdx * m_blockSize);
			++m_numFreeBlocks;
		}
		else
		{
			u64* curHead = static_cast<u64*>(m_freeListHead);
			u64* nextHead = reinterpret_cast<u64*>(*curHead);
			m_freeListHead = nextHead;

			ptr = reinterpret_cast<u8*>(curHead);
		}

		return ptr;
	}

	void* PoolAllocator::allocate(size_t size, size_t align)
	{
		axAssertMsg(align <= 16, "");

		return allocate(size);
	}

	void PoolAllocator::free(void* p_ptr)
	{
		axAssert(p_ptr != nullptr);

		const i64 ptrOffset = static_cast<u8*>(p_ptr) - static_cast<u8*>(m_pBase);
		axAssertMsg(ptrOffset >= 0 && static_cast<u64>(ptrOffset) < static_cast<u64>(m_numTotalBlocks) * m_blockSize, "Invalid pointer! Please provide pointer from the allocator!");

		u64* curHead = static_cast<u64*>(m_freeListHead);
		u64* nextHead = static_cast<u64*>(p_ptr);
		*nextHead = reinterpret_cast<u64>(curHead);
		m_freeListHead = nextHead;

		--m_numFreeBlocks;
	}

	void PoolAllocator::reset()
	{
		m_freeListHead = nullptr;
	}

	size_t PoolAllocator::getTotalCapacity()
	{
		return m_blockSize * m_numTotalBlocks;
	}

	size_t PoolAllocator::getCurrentUsage()
	{
		return m_blockSize * (m_numTotalBlocks - m_numFreeBlocks);
	}


	// Memory Manager

	namespace
	{
		MemoryManagerImpl s_pMemoryManagerImpl;
	}

	MemoryManagerImpl* MemoryManagerImpl::getInstance()
	{
		return &s_pMemoryManagerImpl;
	}

	void MemoryManager::initialize(MemoryManagerDesc desc)
	{
		axAssert(desc.arenaManagedMemory > static_cast<size_t>(desc.frameAllocatorCapacity) * static_cast<size_t>(desc.numFramesInFlight));
		axAssertMsg((desc.frameAllocatorCapacity & (desc.frameAllocatorCapacity - 1)) == 0, "Frame allocator capacity must be power of 2!");

		s_pMemoryManagerImpl.m_capacity = detail::s_poolAllocatorTotalSize + desc.arenaManagedMemory;
		s_pMemoryManagerImpl.m_pBase = static_cast<u8*>(malloc(s_pMemoryManagerImpl.m_capacity));
		s_pMemoryManagerImpl.m_arenaAllocators.resize(desc.numFramesInFlight);

		u8* pMemItr = s_pMemoryManagerImpl.m_pBase; // malloc'd memory is 16 byte aligned
		
		s_pMemoryManagerImpl.m_poolAllocators.resize(std::size(detail::s_poolAllocatorSizeMap));
		for (size_t i = 0; i < std::size(detail::s_poolAllocatorSizeMap); i++)
		{
			auto& [elemSize, poolSize] = detail::s_poolAllocatorSizeMap[i];
			PoolAllocator& poolAllocator = s_pMemoryManagerImpl.m_poolAllocators[i];
			poolAllocator.m_blockSize = static_cast<u32>(elemSize);
			poolAllocator.m_numTotalBlocks = static_cast<u32>(poolSize);
			poolAllocator.m_pBase = pMemItr;

			pMemItr += elemSize * poolSize;
		}

		for (u32 i = 0; i < desc.numFramesInFlight; i++)
		{
			void* pFrameBase = detail::align_ptr(pMemItr, alignof(size_t));
			s_pMemoryManagerImpl.m_arenaAllocators[i].initialize(pFrameBase, desc.frameAllocatorCapacity);

			pMemItr = static_cast<u8*>(pFrameBase) + desc.frameAllocatorCapacity;
		}
	}

	void MemoryManager::shutdown()
	{
		::free(s_pMemoryManagerImpl.m_pBase);
		s_pMemoryManagerImpl.m_capacity = 0;

		for (ArenaAllocator& frameAllocator : s_pMemoryManagerImpl.m_arenaAllocators)
		{
			reset(&frameAllocator);
			frameAllocator.m_pBase = nullptr;
			frameAllocator.m_capacity = 0;
		}
	}

	/*void* allocateMemory(MemoryManager& memory_manager, AllocationType alloc_type, size_t size)
	{
		switch (alloc_type)
		{
		case AllocationType::ARENA:
			memory_manager.m_arenaAllocators
		}
	}*/



	void MemoryTracking::addAllocationInfo(size_t size)
	{
		m_currentUsage += size;
		++m_numAllocationsInFrame;
		++m_numAllocations;
		m_maxUsageInFrame = max(m_maxUsageInFrame, m_currentUsage);
	}

	void MemoryTracking::addFreeInfo(size_t size)
	{
		m_currentUsage -= size;
		--m_numFreesInFrame;
		--m_numFrees;
	}

	void MemoryTracking::beginNewFrame()
	{
		m_numAllocationsInFrame = 0;
		m_numFreesInFrame = 0;
	}

}
}
