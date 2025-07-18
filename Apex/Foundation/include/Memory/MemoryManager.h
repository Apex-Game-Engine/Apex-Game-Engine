#pragma once

#include "Core/Types.h"

namespace apex {
namespace mem {
	enum Tag { ManagedTag };

	class MemoryManagerImpl;
	class PoolAllocator;

	constexpr size_t g_allocatorDefaultCapacity = 1024ui64 * 1024ui64 * 128ui64; // 128 MiB

	enum class AllocationType : u8
	{
		eGlobal, // Global Multi-pool Allocator
		eArena, // Arena Allocator
		eFrame, // Per-frame Stack Allocator
		eLastFrame, // Previous frame's Stack Allocator

		COUNT
	};

	enum class MemoryTag : u8
	{
		eGame,
		eRender,
		eGpu,

		COUNT
	};

	struct MemoryManagerDesc
	{
		u32 frameArenaSize;
		u32 numFramesInFlight;
		// More to come ...
	};
	
	class MemoryManager
	{
	public:
		static void initialize(MemoryManagerDesc desc);
		static void shutdown();

		[[nodiscard]] static void* allocate(size_t size);
		[[nodiscard]] static void* allocate(size_t* size);
		static void free(void* mem);

		static void* getScratchMemory(size_t size);

		static bool checkManaged(void* mem);
		static bool canFree(void* ptr);

		[[nodiscard]] static size_t getTotalCapacity();
		[[nodiscard]] static size_t getAllocatedSize();
		[[nodiscard]] static size_t getFreeSize();

		[[nodiscard]] static size_t getNumMemoryAllocationsInFrame();
		[[nodiscard]] static size_t getNumMemoryFreesInFrame();
		[[nodiscard]] static size_t getMaxUsageInFrame();

		[[nodiscard]] static size_t getNumMemoryAllocations();
		[[nodiscard]] static size_t getNumMemoryFrees();
		[[nodiscard]] static size_t getMaxUsage();

		[[nodiscard]] static float getAverageUsage();

#if defined(APEX_CONFIG_DEBUG)
		static MemoryManagerImpl& getImplInstance();
#endif

	};

	namespace literals {
		constexpr size_t operator""_KiB(size_t x) { return x << 10; }
		constexpr size_t operator""_MiB(size_t x) { return x << 20; }
	}

	struct GlobalMemoryOperators
	{
		static void* OperatorNew(size_t);
		static void OperatorDelete(void* ptr) noexcept;
	};

}
}

void* operator new(size_t size);
void* operator new[](size_t size);

void* operator new(size_t size, apex::mem::Tag tag);
void* operator new[](size_t size, apex::mem::Tag tag);

void* operator new(size_t size, apex::mem::Tag tag, const char* func, const char* file, uint32_t line);
void* operator new[](size_t size, apex::mem::Tag tag, const char* func, const char* file, uint32_t line);

void* operator new(size_t size, apex::mem::Tag tag, const char* type, const char* func, const char* file, uint32_t line);
void* operator new[](size_t size, apex::mem::Tag tag, const char* type, const char* func, const char* file, uint32_t line);

void* operator new(size_t size, const char* func, const char* file, uint32_t line);
void* operator new[](size_t size, const char* func, const char* file, uint32_t line);

void operator delete(void* ptr) noexcept;
void operator delete[](void* ptr) noexcept;

#define APEX_TRACK_ALLOCATIONS 1

#define apex_alloc(SIZE)	(apex::mem::MemoryManager::allocate(SIZE))
#define apex_free(PTR)		(apex::mem::MemoryManager::free(PTR))

#if APEX_TRACK_ALLOCATIONS
#	define apex_new			new (apex::mem::ManagedTag, __FUNCTION__, __FILE__, __LINE__)
#else
#	define apex_new			new (apex::mem::ManagedTag)
#endif
