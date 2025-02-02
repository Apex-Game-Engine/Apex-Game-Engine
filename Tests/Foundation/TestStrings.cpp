#include <gtest/gtest.h>

#include "Containers/AxString.h"
#include "Memory/MemoryManager.h"

using namespace apex;

class AxStringTest: public testing::Test
{
public:
	void SetUp() override
	{
		mem::MemoryManager::initialize({ .frameArenaSize = 0, .numFramesInFlight = 0 });
	}

	void TearDown() override
	{
		mem::MemoryManager::shutdown();
	}
};

TEST_F(AxStringTest, TestSSO)
{
	ASSERT_EQ(mem::MemoryManager::getAllocatedSize(), 0);
	{
		AxString str;
		ASSERT_EQ(mem::MemoryManager::getAllocatedSize(), 0);
	}
	ASSERT_EQ(mem::MemoryManager::getAllocatedSize(), 0);

	{
		AxString str("Hello, Apex!");
		ASSERT_STREQ(str.c_str(), "Hello, Apex!");
		ASSERT_EQ(mem::MemoryManager::getAllocatedSize(), 0);
	}
	ASSERT_EQ(mem::MemoryManager::getAllocatedSize(), 0);
}

TEST_F(AxStringTest, TestNonSSO)
{
	ASSERT_EQ(mem::MemoryManager::getAllocatedSize(), 0);
	{
		AxString str(64);
		ASSERT_EQ(mem::MemoryManager::getAllocatedSize(), 64);
	}
	ASSERT_EQ(mem::MemoryManager::getAllocatedSize(), 0);

	{
		AxString str("Hello, Apex Game Engine!");
		ASSERT_STREQ(str.c_str(), "Hello, Apex Game Engine!");
		ASSERT_EQ(mem::MemoryManager::getAllocatedSize(), 32);
	}
	ASSERT_EQ(mem::MemoryManager::getAllocatedSize(), 0);
}


