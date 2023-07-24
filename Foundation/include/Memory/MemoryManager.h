#pragma once

#include "Core/Types.h"

namespace apex {
	struct AxHandle;

namespace memory {
	struct MemoryManagerImpl;
	struct PoolAllocator;

	constexpr size_t g_allocatorDefaultCapacity = 1024ui64 * 1024ui64 * 128ui64; // 128 MiB

	enum class AllocationType : uint8
	{
		eGlobal, // Global Multi-pool Allocator
		eArena, // Arena Allocator
		eFrame, // Per-frame Stack Allocator
		eLastFrame, // Previous frame's Stack Allocator

		COUNT
	};

	enum class MemoryTag : uint8
	{
		eGame,
		eRender,
		eGpu,

		COUNT
	};

	struct MemoryManagerDesc
	{
		uint32 frameArenaSize;
		uint32 numFramesInFlight;
		// More to come ...
	};
	
	class MemoryManager
	{
	public:
		static void initialize(MemoryManagerDesc desc);
		static void shutdown();

		static AxHandle allocate(size_t size);
		static void free(void* mem);

		static void* getScratchMemory(size_t size);

		static bool checkManaged(void* mem);

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

}
}
