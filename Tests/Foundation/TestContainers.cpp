#include <gtest/gtest.h>
#include "Containers/Containers.h"

TEST(TestContainers, TestFreeListInitialize)
{
	std::vector<uint64_t> aCounters(16);
	/*apex::containers::FreeList<uint64_t> flCounters{ aCounters.data(), aCounters.size() };

	for (uint64_t i = 0; i < 15; i++)
	{
		EXPECT_EQ(aCounters[i], reinterpret_cast<uintptr_t>(&aCounters[i+1]));
	}*/

}


TEST(TestContainers, TestArray)
{
	
}

