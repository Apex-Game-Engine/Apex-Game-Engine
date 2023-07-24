#pragma once
#include "Core/Types.h"
#include "IMemoryTracker.h"
#include "Core/Logging.h"

namespace apex {
namespace memory {

	class PoolAllocator : public IMemoryTracker
	{
	public:
		PoolAllocator() = default;
		PoolAllocator(void* p_begin, size_t size, uint32 block_size);
		~PoolAllocator() = default;

		[[nodiscard]] void* allocate(size_t size);
		[[nodiscard]] void* allocate(size_t size, size_t align);
		void free(void *ptr);

		void initialize(void* p_begin, size_t size, uint32 block_size);
		void reset(); // Potentially EXPENSIVE operation !

		[[nodiscard]] uint32 getTotalBlocks() const { return m_numTotalBlocks; }
		[[nodiscard]] uint32 getBlockSize() const { return m_blockSize; }
		[[nodiscard]] uint32 getFreeBlocks() const { return m_numFreeBlocks; }

		bool containsPointer(void* mem) const { return mem >= m_pBase && mem <= static_cast<uint8*>(m_pBase) + m_numTotalBlocks; }
		
		[[nodiscard]] size_t getTotalCapacity() const override { return static_cast<size_t>(m_blockSize) * m_numTotalBlocks; }
		[[nodiscard]] size_t getCurrentUsage() const override { return static_cast<size_t>(m_blockSize) * (m_numTotalBlocks - m_numFreeBlocks); }

	private:
		void *m_pBase { nullptr };
		void *m_allocPtr {};
		uint32 m_blockSize {};
		uint32 m_numTotalBlocks {};
		uint32 m_numFreeBlocks {};

#ifdef APEX_ENABLE_TESTS
		friend class PoolAllocatorTest;
#endif
	};

	class AlignedPoolAllocator
	{
	};

}
}
