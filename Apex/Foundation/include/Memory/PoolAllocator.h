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

		bool containsPointer(void* mem) const { return mem >= m_basePtr && mem < static_cast<u8*>(m_basePtr) + m_numTotalBlocks * m_blockSize; }
		bool checkManaged(void* mem) const { return containsPointer(mem) && (reinterpret_cast<intptr_t>(mem) - reinterpret_cast<intptr_t>(m_basePtr)) % m_blockSize == 0; }

	protected:
		void* GetBasePointer() const { return m_basePtr; }

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
