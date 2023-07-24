#include <gtest/gtest.h>

#define APEX_ENABLE_MEMORY_LITERALS
#include "Common.h"
#include "Core/Types.h"
#include "Math/Vector3.h"
#include "Memory/ArenaAllocator.h"
#include "Memory/AxHandle.h"
#include "Memory/Memory.h"
#include "Memory/MemoryManager.h"
#include "Memory/MemoryManagerImpl.h"
#include "Memory/PoolAllocator.h"

namespace apex::memory {

	struct SomeClass
	{
		int m_int;
		float m_float;
		double m_double;

		SomeClass() : m_int(1), m_float(1.234f), m_double(3.141592) {}
	};

	class ArenaAllocatorTest : public testing::Test
	{
	public:
		auto arenaAllocator_base() const { return arenaAllocator.m_pBase; }
		auto arenaAllocator_offset() const { return arenaAllocator.m_offset; }
		auto arenaAllocator_capacity() const { return arenaAllocator.m_capacity; }
	protected:
		apex::memory::ArenaAllocator arenaAllocator;
	};

	TEST_F(ArenaAllocatorTest, TestConstructor)
	{
		ASSERT_EQ(arenaAllocator_base(), nullptr);
		ASSERT_EQ(arenaAllocator_offset(), 0);
		ASSERT_EQ(arenaAllocator_capacity(), 0);
	}

	TEST_F(ArenaAllocatorTest, TestInitialize)
	{
		constexpr size_t BUF_SIZE = 1024ui64 * 1024ui64 * 128ui64;
		std::vector<apex::uint8> arenaBuf(BUF_SIZE);

		arenaAllocator.initialize(arenaBuf.data(), BUF_SIZE);
		ASSERT_EQ(arenaAllocator_base(), arenaBuf.data());
		ASSERT_EQ(arenaAllocator_offset(), 0);
		ASSERT_EQ(arenaAllocator_capacity(), BUF_SIZE);
	}

	TEST_F(ArenaAllocatorTest, TestReset)
	{
		constexpr size_t BUF_SIZE = 1024ui64 * 1024ui64 * 128ui64;
		std::vector<apex::uint8> arenaBuf(BUF_SIZE);

		arenaAllocator.initialize(arenaBuf.data(), BUF_SIZE);
		arenaAllocator.reset();
		ASSERT_EQ(arenaAllocator_base(), arenaBuf.data());
		ASSERT_EQ(arenaAllocator_offset(), 0);
		ASSERT_EQ(arenaAllocator_capacity(), BUF_SIZE);
	}

	TEST_F(ArenaAllocatorTest, TestDestrutor)
	{
		void *basePtr;

		constexpr size_t BUF_SIZE = 1024ui64 * 1024ui64 * 128ui64;
		std::vector<apex::uint8> arenaBuf(BUF_SIZE);


		{
			apex::memory::ArenaAllocator arenaAllocator;
			arenaAllocator.initialize(arenaBuf.data(), BUF_SIZE);
			basePtr = arenaAllocator_base();
		}

		// This test should fail
		//int b = *static_cast<int*>(basePtr);
	}

	TEST_F(ArenaAllocatorTest, TestAllocate)
	{
		constexpr size_t BUF_SIZE = 1024ui64 * 1024ui64 * 128ui64;
		std::vector<apex::uint8> arenaBuf(BUF_SIZE);

		arenaAllocator.initialize(arenaBuf.data(), BUF_SIZE);

		int *pInt = static_cast<int*>(arenaAllocator.allocate(sizeof(int)));
		int& a = *pInt;

		ASSERT_EQ(arenaAllocator_offset(), sizeof(int));
	}

	TEST_F(ArenaAllocatorTest, TestAllocateAligned)
	{
		constexpr size_t BUF_SIZE = 1024ui64 * 1024ui64 * 128ui64;
		std::vector<apex::uint8> arenaBuf(BUF_SIZE);

		arenaAllocator.initialize(arenaBuf.data(), BUF_SIZE);

		int *pInt = static_cast<int*>(arenaAllocator.allocate(sizeof(int)));

		ASSERT_PRED2(IsMultipleOf, reinterpret_cast<size_t>(pInt), 16ui64);

		int& a = *pInt;

		ASSERT_EQ(arenaAllocator_offset(), sizeof(int));
	}

	TEST_F(ArenaAllocatorTest, TestAllocateObject)
	{
		constexpr size_t BUF_SIZE = 1024ui64 * 1024ui64 * 128ui64;
		std::vector<apex::uint8> arenaBuf(BUF_SIZE);

		arenaAllocator.initialize(arenaBuf.data(), BUF_SIZE);

		void *pInt = arenaAllocator.allocate(sizeof(SomeClass));
		SomeClass* a = new(pInt) SomeClass();

		ASSERT_EQ(arenaAllocator_offset(), sizeof(SomeClass));

		ASSERT_EQ(a->m_int, 1);
		ASSERT_EQ(a->m_float, 1.234f);
		ASSERT_EQ(a->m_double, 3.141592);
	}

	TEST_F(ArenaAllocatorTest, TestFree)
	{
		constexpr size_t BUF_SIZE = 1024ui64 * 1024ui64 * 128ui64;
		std::vector<apex::uint8> arenaBuf(BUF_SIZE);

		arenaAllocator.initialize(arenaBuf.data(), BUF_SIZE);

		int *pInt = static_cast<int*>(arenaAllocator.allocate(sizeof(int)));
	}

	TEST_F(ArenaAllocatorTest, TestFreeAligned)
	{
		constexpr size_t BUF_SIZE = 1024ui64 * 1024ui64 * 128ui64;
		std::vector<apex::uint8> arenaBuf(BUF_SIZE);

		arenaAllocator.initialize(arenaBuf.data(), BUF_SIZE);

		int *pInt = static_cast<int*>(arenaAllocator.allocate(sizeof(int)));

		ASSERT_PRED2(IsMultipleOf, reinterpret_cast<size_t>(pInt), 16ui64);

		int& a = *pInt;

		ASSERT_EQ(arenaAllocator_offset(), sizeof(int));
	}

	// Pool Allocator Tests
	class PoolAllocatorTest : public testing::Test
	{
	public:
	protected:
		PoolAllocator poolAllocator;
	};

	TEST_F(PoolAllocatorTest, TestAllocate)
	{
		constexpr size_t BLOCK_SIZE = 64;
		constexpr size_t BUF_SIZE = 5 * BLOCK_SIZE;
		std::vector<apex::uint8> poolBuf(BUF_SIZE);

		poolAllocator.initialize(poolBuf.data(), BUF_SIZE, BLOCK_SIZE);

		void* pSomeClass = poolAllocator.allocate(sizeof(SomeClass));
		EXPECT_EQ(pSomeClass, poolBuf.data());
	}

	class MemoryManagerTest : public testing::Test
	{
	public:
		void SetUp() override
		{
			memoryManagerDesc.frameArenaSize = 1024;
			memoryManagerDesc.numFramesInFlight = 3;
			MemoryManager::initialize(memoryManagerDesc);
		}

		PoolAllocator& getMemoryPool(size_t size) { return MemoryManager::getImplInstance().getMemoryPoolForSize(size); }
		size_t getPoolCapacity(size_t alloc_size) { return getMemoryPool(alloc_size).getTotalCapacity(); }
		size_t getPoolSize(size_t alloc_size) { return getMemoryPool(alloc_size).getTotalBlocks(); }

	protected:
		MemoryManagerDesc memoryManagerDesc;
	};

	TEST_F(MemoryManagerTest, TestInitialize)
	{
		printf("memoryManager :: Capacity = %lld\n", MemoryManager::getTotalCapacity() >> 20);
	}

	TEST_F(MemoryManagerTest, TestMemoryPool)
	{
		using namespace literals;

		EXPECT_EQ( getPoolSize(32), 65536 );     // 32 B    x 65536 = 2 MiB
		EXPECT_EQ( getPoolSize(48), 65536 );     // 48 B    x 65536 = 3 MiB
		EXPECT_EQ( getPoolSize(64), 32768 );     // 64 B    x 32768 = 2 MiB
		EXPECT_EQ( getPoolSize(80), 16384 );     // 80 B    x 16384 = 1.25 MiB
		EXPECT_EQ( getPoolSize(128), 8192 );     // 128 B   x 8192  = 1 MiB
		EXPECT_EQ( getPoolSize(256), 8192 );     // 256 B   x 8192  = 2 MiB
		EXPECT_EQ( getPoolSize(512), 8192 );     // 512 B   x 8192  = 4 MiB
		EXPECT_EQ( getPoolSize(1024), 4096 );    // 1 KiB   x 4096  = 4 MiB
		EXPECT_EQ( getPoolSize(2048), 4096 );    // 2 KiB   x 4096  = 8 MiB
		EXPECT_EQ( getPoolSize(4096), 4096 );    // 4 KiB   x 4096  = 16 MiB
		EXPECT_EQ( getPoolSize(8192), 2048 );    // 8 KiB   x 2048  = 16 MiB
		EXPECT_EQ( getPoolSize(16_KiB), 2048 );  // 16 KiB  x 2048  = 32 MiB
		EXPECT_EQ( getPoolSize(32_KiB), 1024 );  // 32 KiB  x 1024  = 32 MiB
		EXPECT_EQ( getPoolSize(64_KiB), 512 );   // 64 KiB  x 512   = 32 MiB
		EXPECT_EQ( getPoolSize(128_KiB), 256 );  // 128 KiB x 256   = 32 MiB
		EXPECT_EQ( getPoolSize(512_KiB), 256 );  // 512 KiB x 256   = 128 MiB
		EXPECT_EQ( getPoolSize(1_MiB), 128 );    // 1 MiB   x 128   = 128 MiB
		EXPECT_EQ( getPoolSize(2_MiB), 128 );    // 1 MiB   x 128   = 256 MiB
		EXPECT_EQ( getPoolSize(4_MiB), 128 );    // 4 MiB   x 128   = 512 MiB
		EXPECT_EQ( getPoolSize(8_MiB),  64 );    // 8 MiB   x  64   = 512 MiB
		EXPECT_EQ( getPoolSize(16_MiB), 32 );    // 16 MiB  x  32   = 512 MiB
		EXPECT_EQ( getPoolSize(32_MiB), 32 );    // 32 MiB  x  32   = 1024 MiB

		AxHandle handle(1024);
		EXPECT_EQ(handle.m_memoryPoolIdx, 7);
	}

	TEST_F(MemoryManagerTest, TestCheckManaged)
	{
		apex::AxHandle handle(1024);
		EXPECT_TRUE(MemoryManager::checkManaged(handle.m_cachedPtr));

		auto pSomeClass = std::make_unique<SomeClass>();
		EXPECT_FALSE(MemoryManager::checkManaged(pSomeClass.get()));
	}

	struct MyManagedClass : AxManagedClass
	{
		char name[1000];
	};

	TEST_F(MemoryManagerTest, TestHandle)
	{
		{
			AxHandle hManagedClass(sizeof(MyManagedClass));
			auto pManagedClass = new (hManagedClass) MyManagedClass();

			strcpy_s(pManagedClass->name, "Athang Gupte");

			delete pManagedClass;
		}

		{
			AxHandle hManagedClass(sizeof(MyManagedClass));
			auto pManagedClass = apex::make_unique<MyManagedClass>(hManagedClass);
			EXPECT_EQ(MemoryManager::getAllocatedSize(), 1024);
		}
		EXPECT_EQ(MemoryManager::getAllocatedSize(), 0);

		{
			
		}
	}

//TEST(TestMemoryManager, TestInitialize)
//{
//	using namespace apex::memory::literals;
//
//	apex::memory::MemoryManagerImpl* memoryManager = apex::memory::MemoryManagerImpl::getInstance();
//
//	apex::memory::MemoryManagerDesc desc{};
//	desc.arenaManagedMemory = 1024_MiB;
//	desc.numFramesInFlight = 3;
//	desc.frameAllocatorCapacity = 1024u * 1024u * 128u;
//
//	apex::memory::MemoryManager::initialize(desc);
//
//
//	ASSERT_EQ(memoryManager->m_capacity, 1024_MiB + 2724_MiB);
//
//	ASSERT_EQ(memoryManager->m_arenaAllocators.size(), 3);
//	ASSERT_EQ(memoryManager->m_arenaAllocators[0].m_capacity, 1024u * 1024u * 128);
//
//	apex::int64 memDiff = static_cast<apex::uint8*>(memoryManager->m_arenaAllocators[1].m_pBase) - static_cast<apex::uint8*>(memoryManager->m_arenaAllocators[0].m_pBase);
//	ASSERT_TRUE(memDiff >= 1024i64 * 1024i64 * 128i64);
//}

	
}
