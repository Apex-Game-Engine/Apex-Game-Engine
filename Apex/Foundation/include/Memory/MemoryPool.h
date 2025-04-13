#pragma once
#include <utility>

#include "Core/Types.h"
#include "MemoryManager.h"

#pragma warning(disable: 4267) // size_t to uint32_t conversion

namespace apex {
namespace mem {

	using literals::operator""_KiB;
	using literals::operator""_MiB;

	using pool_size = u32;
	using elem_size = u32;

	// TODO: Consider splitting this into multiple pool arenas { Game, Render, GPU }
	static constexpr std::pair<elem_size, pool_size> g_memoryPoolSizes[] = {
			// { 8, 131072 },     // 8 B     x 131072 = 1 MiB
			// { 16, 65536 },     // 16 B    x 65536  = 1 MiB
			{ 32, 65536 },     // 32 B    x 65536 = 2 MiB
			{ 48, 65536 },     // 48 B    x 65536 = 3 MiB
			{ 64, 32768 },     // 64 B    x 32768 = 2 MiB
			{ 80, 16384 },     // 80 B    x 16384 = 1.25 MiB
			{ 128, 4096 },     // 128 B   x 4096  = 0.5 MiB
			{ 160, 4096 },     // 160 B   x 4096  = 0.625 MiB
			{ 256, 4096 },     // 256 B   x 4096  = 1 MiB
			{ 320, 4096 },     // 320 B   x 4096  = 1.25 MiB
			{ 512, 4096 },     // 512 B   x 4096  = 4 MiB
			{ 1024, 4096 },    // 1 KiB   x 4096  = 4 MiB
			{ 2048, 4096 },    // 2 KiB   x 4096  = 8 MiB
			{ 4096, 4096 },    // 4 KiB   x 4096  = 16 MiB
			{ 8192, 2048 },    // 8 KiB   x 2048  = 16 MiB
			{ 16_KiB, 2048 },  // 16 KiB  x 2048  = 32 MiB
			{ 32_KiB, 1024 },  // 32 KiB  x 1024  = 32 MiB
			{ 64_KiB, 512 },   // 64 KiB  x 512   = 32 MiB
			{ 128_KiB, 256 },  // 128 KiB x 256   = 32 MiB
			{ 512_KiB, 256 },  // 512 KiB x 256   = 128 MiB
			{ 1_MiB, 128 },    // 1 MiB   x 128   = 128 MiB
			{ 2_MiB, 128 },    // 1 MiB   x 128   = 256 MiB
			{ 4_MiB, 128 },    // 4 MiB   x 128   = 512 MiB
			{ 8_MiB,  64 },    // 8 MiB   x  64   = 512 MiB
			{ 16_MiB, 32 },    // 16 MiB  x  32   = 512 MiB
			{ 32_MiB, 32 },    // 32 MiB  x  32   = 1024 MiB
			{ 128_MiB, 2 }	  // 128 MiB x  2    = 256 MiB
		};

}
}


#pragma warning(default: 4267) // size_t to uint32_t conversion