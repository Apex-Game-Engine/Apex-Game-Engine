#pragma once

#include "Core/Asserts.h"
#include "ArenaAllocator.h"
#include "PoolAllocator.h"

#pragma warning(disable: 4530) // C++ exception handler used, but unwind semantics are not enabled. Specify /EHsc

#include <vector>
#include <optional>

namespace apex {
namespace mem {
	
	class MemoryManagerImpl
	{
	public:
		std::vector<ArenaAllocator> m_arenaAllocators;
		std::vector<PoolAllocator> m_poolAllocators;

		void setUpMemoryPools();
		void setUpMemoryArenas(u32 numFramesInFlight, u32 frameArenaSize);

		std::pair<u32, void*> allocateOnMemoryPool(size_t allocSize);
		void freeFromMemoryPool(void* mem);
		void freeFromMemoryPool(u32 poolIdx, void* mem);

		PoolAllocator& getMemoryPoolForSize(size_t allocSize);
		PoolAllocator& getMemoryPoolFromPointer(void* mem);

		bool checkManaged(void* mem) const;
		bool canFree(void* mem);

		size_t getAllocatedSizeInPools() const;

	private:
		u8 *m_pBase {};
		size_t m_capacity {};

		size_t m_arenaMemorySize;
		size_t m_poolMemorySize;

		friend class MemoryManager;
	};

	namespace detail
	{
		inline u64 align_address(u64 addr, u64 align)
		{
			const u64 mask = align - 1;
			axAssertFmt((align & mask) == 0, "Alignment MUST be a power of 2!");
			return (addr + mask) & ~mask;
		}

		inline void *align_ptr(void *ptr, u64 align)
		{
			const u64 addr = reinterpret_cast<u64>(ptr);
			const u64 alignedAddr = align_address(addr, align);
			return reinterpret_cast<void*>(alignedAddr);
		}

		inline void *shift_and_align_pointer(void *raw_ptr, u64 align)
		{
			u8 *pAlignedPtr = static_cast<u8*>(align_ptr(raw_ptr, align));
			if (pAlignedPtr == raw_ptr)
				pAlignedPtr += align;

			// Find the shift in alignment
			u64 shift = reinterpret_cast<u64>(pAlignedPtr) - reinterpret_cast<u64>(raw_ptr);
			axAssert(shift > 0 && shift <= Constants::u8_MAX);

			// Store the shift
			pAlignedPtr[-1] = static_cast<u8>(shift & 0xff);

			return pAlignedPtr;
		}

		inline void *unshift_pointer(void *aligned_ptr)
		{
			u8 *pAlignedPtr = static_cast<u8*>(aligned_ptr);

			u64 shift = pAlignedPtr[-1];
			if (shift == 0)
				shift = Constants::u8_MAX;

			u8 *pRawPtr = pAlignedPtr - shift;

			return pRawPtr;
		}

	}

}
}
