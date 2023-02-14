#pragma once

#include "axTypes.h"
#include "BasicDefines.h"

namespace apex {
namespace memory {

	constexpr size_t g_allocatorDefaultCapacity = 1024ui64 * 1024ui64 * 128ui64; // 128 MiB

	struct MemoryTracking
	{
		size_t m_currentUsage;
		// Last frame statistics
		size_t m_numAllocationsInFrame{};
		size_t m_numFreesInFrame{};
		size_t m_maxUsageInFrame{};
		// Global statistics
		size_t m_numAllocations{};
		size_t m_numFrees{};
		size_t m_maxUsage{};
		// Calculated statistics
		float m_averageUsage{};

		void addAllocationInfo(size_t size);
		void addFreeInfo(size_t size);
		void beginNewFrame();

		[[nodiscard]] size_t getNumMemoryAllocationsInFrame() const { return m_numAllocationsInFrame; }
		[[nodiscard]] size_t getNumMemoryFreesInFrame() const { return m_numFreesInFrame; }
		[[nodiscard]] size_t getMaxUsageInFrame() const { return m_maxUsageInFrame; }

		[[nodiscard]] size_t getNumMemoryAllocations() const { return m_numAllocations; }
		[[nodiscard]] size_t getNumMemoryFrees() const { return m_numFrees; }
		[[nodiscard]] size_t getMaxUsage() const { return m_maxUsage; }

		[[nodiscard]] float  getAverageUsage() const { return m_averageUsage; }
	};

	/** Base allocator class
	 *
	 *	Adds memory alloc/free semantics to a managed memory space
	 */
	struct IAllocator
	{
		virtual ~IAllocator() = default;

		// Initializes a buffer for the stack allocator to allocate memory from
		virtual void initialize(void *p_begin, size_t size) = 0;
		// Allocates managed memory to caller
		[[nodiscard]] virtual void *allocate(size_t size) = 0;
		// Allocates managed memory to caller with required alignment
		[[nodiscard]] virtual void *allocate(size_t size, size_t align) = 0;
		// Frees the memory of a particular element
		virtual void free(void *ptr) = 0;
		// Clears all the allocated memory. Does NOT free the memory!
		virtual void reset() = 0;

		// Returns total memory capacity assigned to the allocator
		[[nodiscard]] virtual size_t getTotalCapacity() = 0;
		[[nodiscard]] virtual size_t getCurrentUsage() = 0;

#ifdef APEX_ENABLE_MEMORY_TRACKING
		[[nodiscard]] MemoryTracking getMemoryTrackingInfo();
		MemoryTracking m_memoryTracker;
#else
		[[nodiscard]] MemoryTracking getMemoryTrackingInfo() { return {}; }
#endif

	};

	struct ArenaAllocator : public IAllocator
	{
		void *m_pBase { nullptr };
		size_t m_offset {};
		size_t m_capacity {};

		void initialize(void* p_begin, size_t size) override;
		[[nodiscard]] void* allocate(size_t size) override;
		[[nodiscard]] void* allocate(size_t size, size_t align) override;
		void free(void *p_ptr) override;
		void reset() override;

#ifdef APEX_ENABLE_MEMORY_TRACKING
		[[nodiscard]] size_t getTotalCapacity() override;
		[[nodiscard]] size_t getCurrentUsage() override;
#endif
	};

	struct StackAllocator : public IAllocator
	{
		void *m_pBase { nullptr };
		size_t m_offset {};
		size_t m_capacity {};

		void initialize(void* p_begin, size_t size) override;
		[[nodiscard]] void* allocate(size_t size) override;
		[[nodiscard]] void* allocate(size_t size, size_t align) override;
		void free(void *p_ptr) override;
		void reset() override;

#ifdef APEX_ENABLE_MEMORY_TRACKING
		[[nodiscard]] size_t getTotalCapacity() override;
		[[nodiscard]] size_t getCurrentUsage() override;
#endif
	};

	struct PoolAllocator : public IAllocator
	{
		void *m_pBase { nullptr };
		void *m_freeListHead {};
		u32 m_blockSize {};
		u32 m_numTotalBlocks {};
		u32 m_numFreeBlocks {};

		void initialize(void* p_begin, size_t size) override;
		[[nodiscard]] void* allocate(size_t size) override;
		[[nodiscard]] void* allocate(size_t size, size_t align) override;
		void free(void *p_ptr) override;
		void reset() override;

#ifdef APEX_ENABLE_MEMORY_TRACKING
		[[nodiscard]] size_t getTotalCapacity() override;
		[[nodiscard]] size_t getCurrentUsage() override;
#endif
	};

	enum class AllocationType : u8
	{
		ARENA, // Arena Allocator
		FRAME, // per-frame Stack Allocator
		LAST_FRAME, // previous frame's Stack Allocator
		GLOBAL, // global Multi-pool Allocator
		CONTAINER, // container-specific Pool Allocator

		MAX_ENUM_VALUE
	};

	struct MemoryManagerDesc
	{
		u64 arenaManagedMemory;
		u32 frameAllocatorCapacity;
		u32 numFramesInFlight;
		// More to come ...
	};

	struct MemoryManager
	{
		static void initialize(MemoryManagerDesc desc);
		static void shutdown();

		static IAllocator* getAllocator(AllocationType alloc_type);

		static PoolAllocator* getMemoryPool(size_t block_size, size_t );

		[[nodiscard]] static size_t getNumMemoryAllocationsInFrame();
		[[nodiscard]] static size_t getNumMemoryFreesInFrame();
		[[nodiscard]] static size_t getMaxUsageInFrame();

		[[nodiscard]] static size_t getNumMemoryAllocations();
		[[nodiscard]] static size_t getNumMemoryFrees();
		[[nodiscard]] static size_t getMaxUsage();

		[[nodiscard]] static float  getAverageUsage();
	};

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
