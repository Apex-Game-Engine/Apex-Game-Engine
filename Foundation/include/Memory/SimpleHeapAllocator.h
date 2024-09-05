#pragma once
#include "Core/Types.h"

namespace apex {
namespace memory {
	
	using BlockHeader = uint32;

namespace block {
    /**
     * Retrieves the size of a block.
     *
     * @param bp The pointer to the block header.
     * @return The size of the block.
     */
    auto get_size(BlockHeader* bp) -> uint32;

    /**
     * Sets the header of a block.
     *
     * @param bp The pointer to the block header.
     * @param size The size of the block (header + payload + footer).
     * @param allocated The allocation status of the block.
     */
    void set_header(BlockHeader* bp, uint32 size, uint32 allocated);

    /**
     * Retrieves the allocation status of a block.
     *
     * @param bp The pointer to the block header.
     * @return The allocation status of the block.
     */
    auto get_allocated(BlockHeader* bp) -> uint32;

    /**
     * Retrieves the footer of a block.
     *
     * @param bp The pointer to the block header.
     * @return The pointer to the block footer.
     */
    auto get_footer(BlockHeader* bp) -> BlockHeader*;

    /**
     * Sets the footer of a block.
     *
     * @param bp The pointer to the block header.
     */
    void set_footer(BlockHeader* bp);

	/**
	 * Sets the footer of a block.
	 *
	 * @param bp The pointer to the block header.
	 * @param size The size of the block (header + payload + footer).
	 * @param allocated The allocation status of the block.
	 */
	void set_footer(BlockHeader* bp, uint32 size, uint32 allocated);

    /**
     * Retrieves the payload of a block.
     *
     * @param bp The pointer to the block header.
     * @return The pointer to the block payload.
     */
    auto get_payload(BlockHeader* bp) -> void*;

	/**
	 * Retrieves the next block in the heap.
	 *
	 * @param bp The pointer to the block header.
	 * @return The pointer to the next block header.
	 */
	auto get_next(BlockHeader* bp) -> BlockHeader*;

	/**
	 * Retrieves the previous block in the heap.
	 *
	 * @param bp The pointer to the block header.
	 * @return The pointer to the previous block header.
	 */
	auto get_prev(BlockHeader* bp) -> BlockHeader*;
}

	class SimpleHeapAllocator
	{
	public:
		static void* allocate(size_t size);
		static void free(void* ptr);
	private:
		
	};

}
}
