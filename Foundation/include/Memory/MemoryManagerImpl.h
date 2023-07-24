#pragma once

#include "Core/Asserts.h"
#include "ArenaAllocator.h"
#include "PoolAllocator.h"

#include <vector>
#include <optional>

namespace apex {
namespace memory {
	
	struct MemoryManagerImpl
	{
		std::vector<ArenaAllocator> m_arenaAllocators;
		std::vector<PoolAllocator> m_poolAllocators;

		uint8 *m_pBase {};
		size_t m_capacity {};

		size_t m_arenaMemorySize;
		size_t m_poolMemorySize;

		void setUpMemoryPools();
		void setUpMemoryArenas(uint32 numFramesInFlight, uint32 frameArenaSize);

		std::pair<uint32, void*> allocateOnMemoryPool(size_t allocSize);
		void freeFromMemoryPool(void* mem);
		void freeFromMemoryPool(uint32 poolIdx, void* mem);

		PoolAllocator& getMemoryPoolForSize(size_t allocSize);
		PoolAllocator& getMemoryPoolFromPointer(void* mem);

		bool checkManaged(void* mem);

		size_t getAllocatedSizeInPools() const;
	};

	namespace detail
	{
		inline uint64 align_address(uint64 addr, uint64 align)
		{
			const uint64 mask = align - 1;
			axAssertMsg((align & mask) == 0, "Alignment MUST be a power of 2!");
			return (addr + mask) & ~mask;
		}

		inline void *align_ptr(void *ptr, uint64 align)
		{
			const uint64 addr = reinterpret_cast<uint64>(ptr);
			const uint64 alignedAddr = align_address(addr, align);
			return reinterpret_cast<void*>(alignedAddr);
		}

		inline void *shift_and_align_pointer(void *raw_ptr, uint64 align)
		{
			uint8 *pAlignedPtr = static_cast<uint8*>(align_ptr(raw_ptr, align));
			if (pAlignedPtr == raw_ptr)
				pAlignedPtr += align;

			// Find the shift in alignment
			uint64 shift = pAlignedPtr - raw_ptr;
			axAssert(shift > 0 && shift <= constants::uint8_MAX);

			// Store the shift
			pAlignedPtr[-1] = static_cast<uint8>(shift & 0xff);

			return pAlignedPtr;
		}

		inline void *unshift_pointer(void *aligned_ptr)
		{
			uint8 *pAlignedPtr = static_cast<uint8*>(aligned_ptr);

			uint64 shift = pAlignedPtr[-1];
			if (shift == 0)
				shift = constants::uint8_MAX;

			uint8 *pRawPtr = pAlignedPtr - shift;

			return pRawPtr;
		}

	}

}
}
