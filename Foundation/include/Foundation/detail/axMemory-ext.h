#pragma once
#include "Foundation/axAsserts.h"

namespace apex {
namespace memory {

	struct ArenaAllocator
	{
		void *m_pBase { nullptr };
		size_t m_offset {};
		size_t m_capacity {};

#if 0
		/** Initializes a buffer for the stack allocator to allocate memory from
		 *
		 *  Overloads:
		 * - initialize(capacity) : malloc's a buffer of size `capacity`
		 * - initialize(begin, end) : assigns a pre-allocated buffer with `begin` and `end` pointers
		 * - initialize(begin, size) : assigns a pre-allocated buffer of `size` with `begin` pointer
		 */
		void initialize(size_t capacity);
		void initialize(void* begin, void* end);
		void initialize(void* begin, size_t size);


		/** Frees the allocated buffer
		 *
		 *  # MUST NOT be called for externally allocated buffer memory!
		 */
		void destroy();

		/** Resets the stack pointer to clear all the allocated memory
		 *
		 *  # Does NOT free the memory!
		 */
		void reset();

		/** Allocates managed memory to caller
		 *
		 *  For better performance always use the aligned allocation method
		 */
		void* allocate(size_t size) override;
		void* allocate(size_t size, size_t align) override;
		void free(void *ptr) override;
#endif
	};

	struct StackAllocator
	{
		void *m_pBase { nullptr };
		size_t m_offset {};
		size_t m_capacity {};
	};

	struct PoolAllocator
	{
		void *m_pBase { nullptr };
		u32 m_totalPoolCapacity {};
		u32 m_elementSize {};
		u32 m_poolSize {};
		u32 m_numElements {};
		void *m_freeListHead {};

	};

	struct MemoryManager
	{
		std::vector<ArenaAllocator> m_arenaAllocators;
		std::vector<PoolAllocator> m_poolAllocators;

		u8 *m_pBase {};
		size_t m_capacity {};
	};

	namespace detail
	{
		inline u64 align_address(u64 addr, u64 align)
		{
			const u64 mask = align - 1;
			axAssertMsg((align & mask) == 0, "Alignment MUST be a power of 2!");
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
			u64 shift = pAlignedPtr - raw_ptr;
			axAssert(shift > 0 && shift <= constants::u8_MAX);

			// Store the shift
			pAlignedPtr[-1] = static_cast<u8>(shift & 0xff);

			return pAlignedPtr;
		}

		inline void *unshift_pointer(void *aligned_ptr)
		{
			u8 *pAlignedPtr = static_cast<u8*>(aligned_ptr);

			u64 shift = pAlignedPtr[-1];
			if (shift == 0)
				shift = constants::u8_MAX;

			u8 *pRawPtr = pAlignedPtr - shift;

			return pRawPtr;
		}

	}

}
}
