#pragma once

#include "axTypes.h"
#include "detail/axMemory-ext.h"

namespace apex {
namespace memory {

	constexpr size_t g_allocatorDefaultCapacity = 1024ui64 * 1024ui64 * 128ui64; // 128 MiB

	/*struct IAllocator
	{
		virtual ~IAllocator() = default;

		virtual void *allocate(size_t size) = 0;
		virtual void *allocate(size_t size, size_t align) = 0;
		virtual void free(void *ptr) = 0;
	};*/

#define APEX_DECLARE_ALLOCATOR(Allocator) \
	void  initialize(Allocator *p_allocator, void *p_begin, size_t size); \
	void *allocate(Allocator *p_allocator, size_t size); \
	void *allocate(Allocator *p_allocator, size_t size, size_t align); \
	void  free(Allocator *p_allocator, void* p_ptr); \
	void  reset(Allocator *p_allocator);

	struct ArenaAllocator;
	APEX_DECLARE_ALLOCATOR(ArenaAllocator)
	struct StackAllocator;
	APEX_DECLARE_ALLOCATOR(StackAllocator)
	struct PoolAllocator;
	APEX_DECLARE_ALLOCATOR(PoolAllocator)

	enum class AllocationType : u8
	{
		ARENA, // Arena Allocator
		FRAME, // per-frame Stack Allocator
		LAST_FRAME, // previous frame's Stack Allocator
		GLOBAL, // global Multi-pool Allocator
		CONTAINER, // container-specific Pool Allocator

		MAX_ENUM_VALUE
	};

	struct MemoryManager;

	struct MemoryManagerDesc
	{
		u64 arenaManagedMemory;
		u32 frameAllocatorCapacity;
		u32 numFramesInFlight;
		// More to come ...
	};

	void initializeMemoryManager(MemoryManager *p_memory_manager, MemoryManagerDesc desc);
	void destroyMemoryManager(MemoryManager *p_memory_manager);
	void* allocateMemory(MemoryManager& memory_manager, AllocationType alloc_type, size_t size);

	struct Handle
	{
		void *m_pPtr { nullptr };
		AllocationType m_eAllocType;
	};

#ifdef APEX_ENABLE_MEMORY_LITERALS

	namespace literals
	{
		constexpr size_t operator""_KiB(size_t x)
		{
			return x * 1024;
		}

		constexpr size_t operator""_MiB(size_t x)
		{
			return x * 1024 * 1024;
		}
	}

#endif

}
}
