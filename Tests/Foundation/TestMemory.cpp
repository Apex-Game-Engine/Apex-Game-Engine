#include <gtest/gtest.h>

#define APEX_ENABLE_MEMORY_LITERALS
#include "Foundation/axMemory.h"
#include "Foundation/detail/axMemory-ext.h"

TEST(TestArenaAllocator, TestConstructor)
{
	apex::memory::ArenaAllocator stackAllocator;
	ASSERT_EQ(stackAllocator.m_pBase, nullptr);
	ASSERT_EQ(stackAllocator.m_offset, 0);
	ASSERT_EQ(stackAllocator.m_capacity, 0);
}

TEST(TestArenaAllocator, TestInitialize)
{
	apex::memory::ArenaAllocator stackAllocator;

	constexpr size_t BUF_SIZE = 1024ui64 * 1024ui64 * 128ui64;
	std::vector<apex::u8> stackBuf(BUF_SIZE);

	initialize(&stackAllocator, stackBuf.data(), BUF_SIZE);
	ASSERT_EQ(stackAllocator.m_pBase, stackBuf.data());
	ASSERT_EQ(stackAllocator.m_offset, 0);
	ASSERT_EQ(stackAllocator.m_capacity, BUF_SIZE);
}

TEST(TestArenaAllocator, TestReset)
{
	apex::memory::ArenaAllocator stackAllocator;

	constexpr size_t BUF_SIZE = 1024ui64 * 1024ui64 * 128ui64;
	std::vector<apex::u8> stackBuf(BUF_SIZE);

	initialize(&stackAllocator, stackBuf.data(), BUF_SIZE);
	reset(&stackAllocator);
	ASSERT_EQ(stackAllocator.m_pBase, stackBuf.data());
	ASSERT_EQ(stackAllocator.m_offset, 0);
	ASSERT_EQ(stackAllocator.m_capacity, BUF_SIZE);
}

TEST(TestArenaAllocator, TestDestrutor)
{
	void *basePtr;

	constexpr size_t BUF_SIZE = 1024ui64 * 1024ui64 * 128ui64;
	std::vector<apex::u8> stackBuf(BUF_SIZE);


	{
		apex::memory::ArenaAllocator stackAllocator;
		initialize(&stackAllocator, stackBuf.data(), BUF_SIZE);
		basePtr = stackAllocator.m_pBase;
	}

	// This test should fail
	//int b = *static_cast<int*>(basePtr);
}

TEST(TestArenaAllocator, TestAllocate)
{
	apex::memory::ArenaAllocator stackAllocator;

	constexpr size_t BUF_SIZE = 1024ui64 * 1024ui64 * 128ui64;
	std::vector<apex::u8> stackBuf(BUF_SIZE);

	initialize(&stackAllocator, stackBuf.data(), BUF_SIZE);

	int *pInt = static_cast<int*>(allocate(&stackAllocator, sizeof(int)));
	int& a = *pInt;

	ASSERT_EQ(stackAllocator.m_offset, sizeof(int));
}

struct SomeClass
{
	int m_int;
	float m_float;
	double m_double;

	SomeClass() : m_int(1), m_float(1.234f), m_double(3.141592) {}
};

TEST(TestArenaAllocator, TestAllocateObject)
{
	apex::memory::ArenaAllocator stackAllocator;

	constexpr size_t BUF_SIZE = 1024ui64 * 1024ui64 * 128ui64;
	std::vector<apex::u8> stackBuf(BUF_SIZE);

	initialize(&stackAllocator, stackBuf.data(), BUF_SIZE);

	void *pInt = allocate(&stackAllocator, sizeof(SomeClass));
	SomeClass* a = new(pInt) SomeClass();

	ASSERT_EQ(stackAllocator.m_offset, sizeof(SomeClass));

	ASSERT_EQ(a->m_int, 1);
	ASSERT_EQ(a->m_float, 1.234f);
	ASSERT_EQ(a->m_double, 3.141592);
}

TEST(TestArenaAllocator, TestFree)
{
	apex::memory::ArenaAllocator stackAllocator;

	constexpr size_t BUF_SIZE = 1024ui64 * 1024ui64 * 128ui64;
	std::vector<apex::u8> stackBuf(BUF_SIZE);

	initialize(&stackAllocator, stackBuf.data(), BUF_SIZE);

	int *pInt = static_cast<int*>(allocate(&stackAllocator, sizeof(int)));

	free(&stackAllocator, pInt);

	ASSERT_EQ(stackAllocator.m_offset, 0);
}




TEST(TestMemoryManager, TestInitialize)
{
	using namespace apex::memory::literals;

	apex::memory::MemoryManager memoryManager;

	apex::memory::MemoryManagerDesc desc{};
	desc.arenaManagedMemory = 1024_MiB;
	desc.numFramesInFlight = 3;
	desc.frameAllocatorCapacity = 1024u * 1024u * 128u;

	apex::memory::initializeMemoryManager(&memoryManager, desc);

	ASSERT_EQ(memoryManager.m_capacity, 1024_MiB + 2724_MiB);

	ASSERT_EQ(memoryManager.m_arenaAllocators.size(), 3);
	ASSERT_EQ(memoryManager.m_arenaAllocators[0].m_capacity, 1024u * 1024u * 128);

	apex::i64 memDiff = static_cast<apex::u8*>(memoryManager.m_arenaAllocators[1].m_pBase) - static_cast<apex::u8*>(memoryManager.m_arenaAllocators[0].m_pBase);
	ASSERT_TRUE(memDiff >= 1024i64 * 1024i64 * 128i64);
}
