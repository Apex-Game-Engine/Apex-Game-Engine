#include "Memory/SimpleHeapAllocator.h"

#include "Core/Types.h"

namespace apex::memory {
namespace block {

	static constexpr uint32_t SIZE_MASK = ~0x7; // 0xFFFFFFF8

	uint32 get_size(BlockHeader * bp)
	{
		return *bp & SIZE_MASK;
	}

	// here the size is the size of the block + the size of the header + the size of the footer
	void set_header(BlockHeader * bp, uint32 size, uint32 allocated)
	{
		*bp = (size & SIZE_MASK) | allocated;
	}

	uint32 get_allocated(BlockHeader * bp)
	{
		return *bp & 0x1;
	}

	BlockHeader * get_footer(BlockHeader * bp)
	{
		return reinterpret_cast<uint32*>(reinterpret_cast<uint8*>(bp) + get_size(bp) - sizeof(BlockHeader));
	}

	void set_footer(BlockHeader * bp)
	{
		*get_footer(bp) = *bp;
	}

	void set_footer(BlockHeader * bp, uint32 size, uint32 allocated)
	{
		*get_footer(bp) = (size & SIZE_MASK) | allocated;
	}

	void* get_payload(BlockHeader * bp)
	{
		return bp + 1;
	}

	auto get_next(BlockHeader* bp) -> BlockHeader*
	{
		return reinterpret_cast<BlockHeader*>(reinterpret_cast<uint8*>(bp) + get_size(bp));
	}

	auto get_prev(BlockHeader* bp) -> BlockHeader*
	{
		return reinterpret_cast<BlockHeader*>(reinterpret_cast<uint8*>(bp) - get_size(reinterpret_cast<BlockHeader*>(reinterpret_cast<uint8*>(bp) - sizeof(BlockHeader))));
	}
}

}
