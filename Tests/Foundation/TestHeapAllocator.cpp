#include <gtest/gtest.h>

#include "Memory/SimpleHeapAllocator.h"


namespace apex::memory {

	TEST(HeapAllocatorTest, TestBlockBitfield)
	{
		uint32 block = 0x00000007;
		uint32 size = block::get_size(&block);
		uint32 allocated = block::get_allocated(&block);
		EXPECT_EQ(size, 0x00000000);
		EXPECT_EQ(allocated, 0x00000001);

		block = 0x00000008;
		size = block::get_size(&block);
		allocated = block::get_allocated(&block);
		EXPECT_EQ(size, 0x00000008);
		EXPECT_EQ(allocated, 0x00000000);

		block = 0x0000000F;
		size = block::get_size(&block);
		allocated = block::get_allocated(&block);
		EXPECT_EQ(size, 0x00000008);
		EXPECT_EQ(allocated, 0x00000001);
	}

	TEST(HeapAllocatorTest, TestBlockHeaderFooter)
	{
		union
		{
			uint32 block[4];
			uint8 bytes[16];
		};
		block::set_header(block, 0x10, true);
		block::set_footer(block);

		EXPECT_EQ(block[0], 0x00000011);
		EXPECT_EQ(block[3], 0x00000011);
	}


}
