#pragma once
#include "Core/Types.h"
#include "IMemoryTracker.h"
#include "PoolAllocator.h"
#include "Core/Asserts.h"
#include "Core/Logging.h"

namespace apex {
namespace mem {

	class PoolAllocator
	{
	public:
		PoolAllocator() = default;
		PoolAllocator(void* p_begin, size_t size, u32 block_size);
		~PoolAllocator() = default;

		PoolAllocator(const PoolAllocator& a) = delete;
		PoolAllocator& operator=(const PoolAllocator& a) = delete;

		PoolAllocator(PoolAllocator&& a) = default;
		PoolAllocator& operator=(PoolAllocator&& a) = default;

		void initialize(void* p_begin, size_t size, u32 block_size);
		void reset(); // Potentially EXPENSIVE operation !
		void shutdown();

		[[nodiscard]] void* allocate(size_t size);
		[[nodiscard]] void* allocate(size_t size, size_t align);
		void free(void *ptr);

		[[nodiscard]] u32 getTotalBlocks() const { return m_numTotalBlocks; }
		[[nodiscard]] u32 getBlockSize() const { return m_blockSize; }
		[[nodiscard]] u32 getFreeBlocks() const { return m_numFreeBlocks; }

		bool isFree(void* ptr) const;
		bool containsPointer(void* ptr) const;
		bool checkManaged(void* ptr) const;

	protected:
		void* getBase() const { return m_basePtr; }

	private:
		void *m_basePtr {};
		void *m_allocPtr {};
		u32 m_blockSize {};
		u32 m_numTotalBlocks {};
		u32 m_numFreeBlocks {};

		friend class MemoryManagerImpl;

#ifdef APEX_ENABLE_TESTS
		friend class PoolAllocatorTest;
#endif
	};

}
}
