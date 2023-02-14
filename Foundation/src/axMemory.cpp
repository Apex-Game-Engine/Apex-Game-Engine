#define APEX_ENABLE_MEMORY_LITERALS
#include "Foundation/axMemory.h"
#include "Foundation/detail/axMemory-ext.h"
#include <cstdlib>

namespace apex {
namespace memory {

	using namespace literals;

	// Arena Allocator

	void initialize(ArenaAllocator *p_allocator, void* p_begin, size_t size)
	{
		p_allocator->m_pBase = p_begin;
		p_allocator->m_capacity = size;
		p_allocator->m_offset = 0;
	}

	void reset(ArenaAllocator *p_allocator)
	{
		p_allocator->m_offset = 0;
	}

	void* allocate(ArenaAllocator *p_allocator, size_t size)
	{
		axAssertMsg(p_allocator->m_pBase != nullptr, "Allocator not initialized!");
		axAssertMsg(p_allocator->m_capacity - p_allocator->m_offset > size, "Stack allocator overflow!");

		void* top = &static_cast<u8*>(p_allocator->m_pBase)[p_allocator->m_offset];

		p_allocator->m_offset += size;

		return top;
	}
	
	void* allocate(ArenaAllocator *p_allocator, size_t size, size_t align)
	{
		const size_t actualAllocSize = size + align;

		axAssertMsg(p_allocator->m_pBase != nullptr, "Allocator not initialized!");
		axAssertMsg(p_allocator->m_capacity - p_allocator->m_offset > actualAllocSize, "Stack allocator overflow!");

		void* top = &static_cast<u8*>(p_allocator->m_pBase)[p_allocator->m_offset];
		top = detail::shift_and_align_pointer(top, align);

		p_allocator->m_offset += actualAllocSize;

		return top;
	}

	void free(ArenaAllocator *p_allocator, void* p_ptr)
	{
		axAssert(p_ptr != nullptr);

		const i64 ptrOffset = static_cast<u8*>(p_ptr) - static_cast<u8*>(p_allocator->m_pBase);

		axAssertMsg(ptrOffset >= 0 && static_cast<u64>(ptrOffset) < p_allocator->m_offset, "Invalid pointer! Please provide pointer from the stack!");

		if (p_allocator->m_pBase)
		{
			p_allocator->m_offset = ptrOffset;
		}
	}


	// Stack Allocator

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
	}

	void reset(StackAllocator* p_allocator)
	{
		p_allocator->m_offset = 0;
	}


	// Pool Allocator

	void initialize(PoolAllocator* p_allocator, void* p_begin, size_t size)
	{
		p_allocator->m_pBase = p_begin;
		p_allocator->m_totalPoolCapacity = static_cast<u32>(size / static_cast<size_t>(p_allocator->m_elementSize));
	}

	void* allocate(PoolAllocator* p_allocator, size_t size)
	{
		axAssertMsg(p_allocator->m_pBase != nullptr, "Pool allocator not initialized!");
		axAssertMsg(p_allocator->m_elementSize > size, "Allocation size cannot be larger that Pool allocator's element size!");

		u8* ptr;

		if (p_allocator->m_freeListHead == 0)
		{
			ptr = static_cast<u8*>(p_allocator->m_pBase) + static_cast<u64>(p_allocator->m_poolSize * p_allocator->m_elementSize);
			++p_allocator->m_poolSize;
			++p_allocator->m_numElements;
		}
		else
		{
			u64* curHead = static_cast<u64*>(p_allocator->m_freeListHead);
			u64* nextHead = reinterpret_cast<u64*>(*curHead);
			p_allocator->m_freeListHead = nextHead;

			ptr = reinterpret_cast<u8*>(curHead);
		}

		return ptr;
	}

	void* allocate(PoolAllocator* p_allocator, size_t size, size_t align)
	{
		return nullptr;
	}

	void free(PoolAllocator* p_allocator, void* p_ptr)
	{
		axAssert(p_ptr != nullptr);

		const i64 ptrOffset = static_cast<u8*>(p_ptr) - static_cast<u8*>(p_allocator->m_pBase);
		axAssertMsg(ptrOffset >= 0 && static_cast<u64>(ptrOffset) < p_allocator->m_poolSize, "Invalid pointer! Please provide pointer from the allocator!");

		u64* curHead = static_cast<u64*>(p_allocator->m_freeListHead);
		u64* nextHead = static_cast<u64*>(p_ptr);
		*nextHead = reinterpret_cast<u64>(curHead);
		p_allocator->m_freeListHead = nextHead;

		--p_allocator->m_numElements;
	}

	void reset(PoolAllocator* p_allocator)
	{
		p_allocator->m_poolSize = 0;
		p_allocator->m_freeListHead = nullptr;
	}

	// Memory Manager

	namespace
	{
		using pool_size = u64;
		using elem_size = u64;

		constexpr std::pair<elem_size, pool_size> s_poolAllocatorSizeMap[] = {
			{ 32, 65536 },     // 32 B    x 65536 = 2 MiB
			{ 64, 32768 },     // 64 B    x 32768 = 2 MiB
			{ 64_KiB, 512 },   // 64 KiB  x 512   = 32 MiB
			{ 512_KiB, 256 },  // 512 KiB x 256   = 128 MiB
			{ 4_MiB, 128 },    // 4 MiB   x 128   = 512 MiB
			{ 8_MiB,  64 },    // 8 MiB   x  64   = 512 MiB
			{ 16_MiB, 32 },    // 16 MiB  x  32   = 512 MiB
			{ 32_MiB, 32 },    // 32 MiB  x  32   = 1024 MiB
		};

		constexpr size_t s_poolAllocatorTotalSize { 2724_MiB };
	}

	void initializeMemoryManager(MemoryManager* p_memory_manager, MemoryManagerDesc desc)
	{
		axAssert(desc.arenaManagedMemory > static_cast<size_t>(desc.frameAllocatorCapacity) * static_cast<size_t>(desc.numFramesInFlight));
		axAssertMsg((desc.frameAllocatorCapacity & (desc.frameAllocatorCapacity - 1)) == 0, "Frame allocator capacity must be power of 2!");

		p_memory_manager->m_capacity = s_poolAllocatorTotalSize + desc.arenaManagedMemory;
		p_memory_manager->m_pBase = static_cast<u8*>(malloc(p_memory_manager->m_capacity));
		p_memory_manager->m_arenaAllocators.resize(desc.numFramesInFlight);

		u8* pMemItr = p_memory_manager->m_pBase; // malloc'd memory is 16 byte aligned
		
		p_memory_manager->m_poolAllocators.resize(std::size(s_poolAllocatorSizeMap));
		for (size_t i = 0; i < std::size(s_poolAllocatorSizeMap); i++)
		{
			auto& [elemSize, poolSize] = s_poolAllocatorSizeMap[i];
			PoolAllocator& poolAllocator = p_memory_manager->m_poolAllocators[i];
			poolAllocator.m_elementSize = static_cast<u32>(elemSize);
			poolAllocator.m_totalPoolCapacity = static_cast<u32>(poolSize);
			poolAllocator.m_pBase = pMemItr;

			pMemItr += elemSize * poolSize;
		}

		for (u32 i = 0; i < desc.numFramesInFlight; i++)
		{
			void* pFrameBase = detail::align_ptr(pMemItr, alignof(size_t));
			initialize(&p_memory_manager->m_arenaAllocators[i], pFrameBase, desc.frameAllocatorCapacity);

			pMemItr = static_cast<u8*>(pFrameBase) + desc.frameAllocatorCapacity;
		}
	}

	void destroyMemoryManager(MemoryManager* p_memory_manager)
	{
		::free(p_memory_manager->m_pBase);
		p_memory_manager->m_capacity = 0;

		for (ArenaAllocator& frameAllocator : p_memory_manager->m_arenaAllocators)
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
}
}
