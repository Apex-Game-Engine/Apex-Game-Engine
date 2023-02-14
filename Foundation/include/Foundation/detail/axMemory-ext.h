#pragma once
#include "Foundation/axAsserts.h"
#include "Foundation/axMemory.h"

namespace apex {
namespace memory {

	struct MemoryManagerImpl
	{
		std::vector<ArenaAllocator> m_arenaAllocators;
		std::vector<PoolAllocator> m_poolAllocators;

		u8 *m_pBase {};
		size_t m_capacity {};

		static MemoryManagerImpl* getInstance();
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

		using pool_size = u64;
		using elem_size = u64;

		using namespace apex::memory::literals;

		constexpr std::pair<elem_size, pool_size> s_poolAllocatorSizeMap[] = {
			{ 32, 65536 },     // 32 B    x 65536 = 2 MiB
			{ 64, 32768 },     // 64 B    x 32768 = 2 MiB
			{ 64_KiB, 512 },   // 64 KiB  x 512   = 32 MiB
			{ 512_KiB, 256 },  // 512 KiB x 256   = 128 MiB
			{ 4_MiB, 128 },    // 4 MiB   x 128   = 512 MiB
			{ 8_MiB,  64 },    // 8 MiB   x  64   = 512 MiB
			{ 16_MiB, 32 },    // 16 MiB  x  32   = 512 MiB
			{ 32_MiB, 32 },    // 32 MiB  x  32   = 1024 MiB
		};

		constexpr size_t s_poolAllocatorTotalSize { 2724_MiB };

	}

}
}
