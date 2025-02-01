#include <gtest/gtest.h>

#define APEX_ENABLE_MEMORY_LITERALS
#include <array>
#include <ranges>

#include "Common.h"
#include "Containers/AxArray.h"
#include "Containers/AxRange.h"
#include "Containers/AxStringView.h"
#include "Core/Types.h"
#include "Math/Vector3.h"
#include "Math/Vector4.h"
#include "Memory/ArenaAllocator.h"
#include "Memory/AxHandle.h"
#include "Memory/AxManagedClass.h"
#include "Memory/MemoryManager.h"
#include "Memory/MemoryManagerImpl.h"
#include "Memory/PoolAllocator.h"
#include "Memory/SharedPtr.h"
#include "Memory/UniquePtr.h"

namespace apex::mem {

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
		apex::mem::ArenaAllocator arenaAllocator;
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
		std::vector<apex::u8> arenaBuf(BUF_SIZE);

		arenaAllocator.initialize(arenaBuf.data(), BUF_SIZE);
		ASSERT_EQ(arenaAllocator_base(), arenaBuf.data());
		ASSERT_EQ(arenaAllocator_offset(), 0);
		ASSERT_EQ(arenaAllocator_capacity(), BUF_SIZE);
	}

	TEST_F(ArenaAllocatorTest, TestReset)
	{
		constexpr size_t BUF_SIZE = 1024ui64 * 1024ui64 * 128ui64;
		std::vector<apex::u8> arenaBuf(BUF_SIZE);

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
		std::vector<apex::u8> arenaBuf(BUF_SIZE);


		{
			apex::mem::ArenaAllocator arenaAllocator;
			arenaAllocator.initialize(arenaBuf.data(), BUF_SIZE);
			basePtr = arenaAllocator_base();
		}

		// This test should fail
		//int b = *static_cast<int*>(basePtr);
	}

	TEST_F(ArenaAllocatorTest, TestAllocate)
	{
		constexpr size_t BUF_SIZE = 1024ui64 * 1024ui64 * 128ui64;
		std::vector<apex::u8> arenaBuf(BUF_SIZE);

		arenaAllocator.initialize(arenaBuf.data(), BUF_SIZE);

		int *pInt = static_cast<int*>(arenaAllocator.allocate(sizeof(int)));
		int& a = *pInt;

		ASSERT_EQ(arenaAllocator_offset(), sizeof(int));
	}

	TEST_F(ArenaAllocatorTest, TestAllocateAligned)
	{
		constexpr size_t BUF_SIZE = 1024ui64 * 1024ui64 * 128ui64;
		std::vector<apex::u8> arenaBuf(BUF_SIZE);

		arenaAllocator.initialize(arenaBuf.data(), BUF_SIZE);

		int *pInt = static_cast<int*>(arenaAllocator.allocate(sizeof(int)));

		ASSERT_PRED2(IsMultipleOf, reinterpret_cast<size_t>(pInt), 16ui64);

		int& a = *pInt;

		ASSERT_EQ(arenaAllocator_offset(), sizeof(int));
	}

	TEST_F(ArenaAllocatorTest, TestAllocateObject)
	{
		constexpr size_t BUF_SIZE = 1024ui64 * 1024ui64 * 128ui64;
		std::vector<apex::u8> arenaBuf(BUF_SIZE);

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
		std::vector<apex::u8> arenaBuf(BUF_SIZE);

		arenaAllocator.initialize(arenaBuf.data(), BUF_SIZE);

		int *pInt = static_cast<int*>(arenaAllocator.allocate(sizeof(int)));
	}

	TEST_F(ArenaAllocatorTest, TestFreeAligned)
	{
		constexpr size_t BUF_SIZE = 1024ui64 * 1024ui64 * 128ui64;
		std::vector<apex::u8> arenaBuf(BUF_SIZE);

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
		std::vector<apex::u8> poolBuf(BUF_SIZE);

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

		void TearDown() override
		{
			MemoryManager::shutdown();
		}

		PoolAllocator& getMemoryPool(size_t size) { return MemoryManager::getImplInstance().getMemoryPoolForSize(size); }
		size_t getPoolCapacity(size_t alloc_size) { return getMemoryPool(alloc_size).getTotalCapacity(); }
		size_t getPoolSize(size_t alloc_size) { return getMemoryPool(alloc_size).getTotalBlocks(); }

		auto handle_getMemoryPoolIndex(AxHandle& handle) { return handle.m_memoryPoolIdx; }

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
		EXPECT_EQ( getPoolSize(128), 4096 );     // 128 B   x 4096  = 0.5 MiB
		EXPECT_EQ( getPoolSize(160), 4096 );     // 160 B   x 4096  = 0.625 MiB
		EXPECT_EQ( getPoolSize(256), 4096 );     // 256 B   x 4096  = 1 MiB
		EXPECT_EQ( getPoolSize(320), 4096 );     // 320 B   x 4096  = 1.25 MiB
		EXPECT_EQ( getPoolSize(512), 4096 );     // 512 B   x 4096  = 2 MiB
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
		EXPECT_EQ(handle_getMemoryPoolIndex(handle), 9);
	}

	TEST_F(MemoryManagerTest, TestCheckManaged)
	{
		apex::AxHandle handle(1024);
		EXPECT_TRUE(MemoryManager::checkManaged(handle.getAs<void>()));

		auto pSomeClass = std::make_unique<SomeClass>();
		EXPECT_FALSE(MemoryManager::checkManaged(pSomeClass.get()));
	}

	TEST_F(MemoryManagerTest, TestGetMemoryPoolFromPointer)
	{
		struct Data_128bytes : AxManagedClass
		{
			byte bytes[128];
		};

		UniquePtr<Data_128bytes> pointers[1000];

		for (int i = 0; i < 1000; i++)
		{
			pointers[i] = apex::make_unique<Data_128bytes>();
		}

		for (int i = 0; i < 1000; i++)
		{
			EXPECT_EQ(MemoryManager::getImplInstance().getMemoryPoolFromPointer(pointers[i].get()).getBlockSize(), 128);
		}
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
			auto pManagedClass = apex::unique_from_handle<MyManagedClass>(hManagedClass);
			EXPECT_EQ(MemoryManager::getAllocatedSize(), 1024);
		}
		EXPECT_EQ(MemoryManager::getAllocatedSize(), 0);
	}

	struct StructWithDestructor : public AxManagedClass
	{
		inline static s32 s_count = 0;
		inline static s32 s_numDestroyed = 0;

		char m_dbgName[64];

		StructWithDestructor()
		: m_dbgName{ "StructWithDestructor" }
		{
			++s_count;
		}

		~StructWithDestructor()
		{
			printf("dtor\n");
			--s_count;
			s_numDestroyed++;
		}
	};
	static_assert(sizeof(StructWithDestructor) == 64);

	TEST_F(MemoryManagerTest, TestArrayHandle)
	{
		EXPECT_EQ(MemoryManager::getAllocatedSize(), 0);
		{
			AxHandle hArray(sizeof(AxHandle) + 8 * sizeof(int));
			auto pArray = new(hArray) AxArray<int>();
			//pArray->resizeToHandle(hArray);
			//auto& arr = *pArray;
			//EXPECT_EQ((size_t)&arr[0], (size_t)hArray.m_cachedPtr + 24);
			EXPECT_EQ(MemoryManager::getAllocatedSize(), hArray.getBlockSize());
			delete pArray;
		}
		EXPECT_EQ(MemoryManager::getAllocatedSize(), 0);

		{
			AxHandle hStruct( sizeof(StructWithDestructor) );
			EXPECT_EQ(hStruct.getBlockSize(), 64);
			EXPECT_EQ(MemoryManager::getAllocatedSize(), hStruct.getBlockSize());

			auto pStruct = new(hStruct) StructWithDestructor();
			EXPECT_EQ(StructWithDestructor::s_count, 1);

			delete pStruct;
		}
		EXPECT_EQ(StructWithDestructor::s_count, 0);
		EXPECT_EQ(MemoryManager::getAllocatedSize(), 0);

		// TODO: The following scenario should be disallowed! Figure out a way to throw a compiler error on this sort of situation. Use AxArray instead (it is memory managed and will be made RAII)
		//{
		//	AxHandle hArray = apex::make_handle<int[]>(32);
		//	EXPECT_EQ(hArray.getBlockSize(), 128);
		//	EXPECT_EQ(MemoryManager::getAllocatedSize(), hArray.getBlockSize());

		//	int* arr = new (hArray.getAs<void>()) int[32]();

		//	delete arr;
		//	//hArray.free();
		//}
		EXPECT_EQ(MemoryManager::getAllocatedSize(), 0);
	}

	struct Base : public AxManagedClass
	{
		inline static s32 s_count = 0;

		Base() { ++s_count; }
		virtual ~Base() { printf("Base::dtor\n"); --s_count; }

		virtual s32 getInt() const { return 42; }
	};

	struct Derived : public Base
	{
		inline static s32 s_count = 0;

		Derived() { ++s_count; }
		~Derived() override { printf("Derived::dtor\n"); --s_count; }

		s32 getInt() const override { return 213; }
	};

	TEST_F(MemoryManagerTest, TestUniquePtr)
	{
		EXPECT_EQ(StructWithDestructor::s_count, 0);
		{
			AxHandle hStruct = apex::make_handle<StructWithDestructor>();
			EXPECT_EQ(hStruct.getBlockSize(), 64);
			EXPECT_EQ(MemoryManager::getAllocatedSize(), hStruct.getBlockSize());
			
			auto pStruct = apex::unique_from_handle<StructWithDestructor>(hStruct);
			EXPECT_EQ(StructWithDestructor::s_count, 1);

			EXPECT_STREQ(pStruct->m_dbgName, "StructWithDestructor");
		}
		EXPECT_EQ(StructWithDestructor::s_count, 0);
		EXPECT_EQ(MemoryManager::getAllocatedSize(), 0);

		{
			AxHandle hDerived = make_handle<Derived>();
			EXPECT_EQ(hDerived.getBlockSize(), 32);
			EXPECT_EQ(MemoryManager::getAllocatedSize(), hDerived.getBlockSize());

			UniquePtr<Base> pBase = unique_from_handle<Derived>(hDerived);
			EXPECT_EQ(pBase->getInt(), 213);

			EXPECT_EQ(Base::s_count, 1);
			EXPECT_EQ(Derived::s_count, 1);
		}
		EXPECT_EQ(Base::s_count, 0);
		EXPECT_EQ(Derived::s_count, 0);
		EXPECT_EQ(MemoryManager::getAllocatedSize(), 0);

		{
			AxHandle h (sizeof(int));
			EXPECT_EQ(h.getBlockSize(), 32);
			EXPECT_EQ(MemoryManager::getAllocatedSize(), h.getBlockSize());

			auto a = new (h) AxManagedClassAdapter<int>(2);
			EXPECT_EQ(static_cast<int>(*a), 2);
			*a = 3;
			EXPECT_TRUE(*a == 3);

			delete a;
		}
		EXPECT_EQ(MemoryManager::getAllocatedSize(), 0);

		{
			AxHandle h (sizeof(int));
			EXPECT_EQ(h.getBlockSize(), 32);
			EXPECT_EQ(MemoryManager::getAllocatedSize(), h.getBlockSize());

			auto a = unique_from_handle<int>(h, 42);
			EXPECT_TRUE(*a == 42);
			*a = 213;
			EXPECT_EQ(static_cast<int>(*a), 213);
		}
		EXPECT_EQ(MemoryManager::getAllocatedSize(), 0);

		{
			auto a = make_unique<StructWithDestructor>();
			EXPECT_EQ(MemoryManager::getAllocatedSize(), 64);
		}
		EXPECT_EQ(MemoryManager::getAllocatedSize(), 0);

		{
			auto a = make_unique<int>(42);
			EXPECT_EQ(MemoryManager::getAllocatedSize(), 32);
		}
		EXPECT_EQ(MemoryManager::getAllocatedSize(), 0);

		{
			AxHandle hArray = make_handle<s32[32]>();
			EXPECT_EQ(hArray.getBlockSize(), 128);

			auto pArray = unique_from_handle<int[]>(hArray, 32);

			pArray[0] = 1;
		}
		EXPECT_EQ(MemoryManager::getAllocatedSize(), 0);

		{
			auto pArray = apex::make_unique<int[]>(32);
			EXPECT_EQ(MemoryManager::getAllocatedSize(), 160);
		}
		EXPECT_EQ(MemoryManager::getAllocatedSize(), 0);

		{
			AxHandle hArray = apex::make_handle<StructWithDestructor[]>(2);
			EXPECT_EQ(MemoryManager::getAllocatedSize(), 160);

			auto pArray = unique_from_handle<StructWithDestructor[]>(hArray, 2);
		}
		EXPECT_EQ(MemoryManager::getAllocatedSize(), 0);
	}

	TEST_F(MemoryManagerTest, TestDynamicallyAllocatedUniquePtr)
	{
		EXPECT_EQ(MemoryManager::getAllocatedSize(), 0);
		{
			UniquePtr<StructWithDestructor> * ppStruct;
			AxHandle handle(sizeof(UniquePtr<StructWithDestructor>));
			ppStruct = handle.getAs<UniquePtr<StructWithDestructor>>();
			EXPECT_EQ(MemoryManager::getAllocatedSize(), 32); // sizeof UniquePtr (8) but the closest available size is 32

			*ppStruct = apex::make_unique<StructWithDestructor>();

			EXPECT_EQ(MemoryManager::getAllocatedSize(), 32 + 64); // 64 bytes for the StructWithDestructor

			delete ppStruct;
		}
		EXPECT_EQ(MemoryManager::getAllocatedSize(), 0);

		using Int = AxManagedClassAdapter<int>;
		{
			AxHandle handle(sizeof(Int) * 16);
			auto pInts = handle.getAs<Int>();
			EXPECT_EQ(MemoryManager::getAllocatedSize(), 64); // 16 * 4 = 64

			delete[] pInts;
		}
		EXPECT_EQ(MemoryManager::getAllocatedSize(), 0);

		{
			AxHandle handle = apex::make_handle<StructWithDestructor[]>(15);
			StructWithDestructor* pStructArr = handle.getAs<StructWithDestructor>();
			EXPECT_EQ(MemoryManager::getAllocatedSize(), 1024); // 16 * 64 = 1024 (extra 8 bytes for the size of the array)

			pStructArr = new (handle) StructWithDestructor[15];

			delete[] pStructArr;
		}
		EXPECT_EQ(MemoryManager::getAllocatedSize(), 0);

		{
			UniquePtr<StructWithDestructor> * pStructArr;
			AxHandle handle = apex::make_handle<UniquePtr<StructWithDestructor>[]>(15);
			EXPECT_EQ(MemoryManager::getAllocatedSize(), 128); // 16 * 8 = 128

			pStructArr = new (handle) UniquePtr<StructWithDestructor>[15];

			delete[] pStructArr;
		}
		EXPECT_EQ(MemoryManager::getAllocatedSize(), 0);
	}

	TEST_F(MemoryManagerTest, TestUniquePtrCallElementDestructor)
	{
		StructWithDestructor::s_numDestroyed = 0;
		{
			auto pStruct = apex::make_unique<StructWithDestructor>();
			pStruct = apex::make_unique<StructWithDestructor>();
			EXPECT_EQ(StructWithDestructor::s_numDestroyed, 1);
		}
		EXPECT_EQ(StructWithDestructor::s_numDestroyed, 2);

		{

			auto pStruct = apex::make_unique<StructWithDestructor[]>(16);
			EXPECT_EQ(MemoryManager::getAllocatedSize(), 2048);
			pStruct = apex::make_unique<StructWithDestructor[]>(32);
		}
		EXPECT_EQ(StructWithDestructor::s_numDestroyed, 50);
	}

	struct IntArrayWrapper
	{
		IntArrayWrapper(size_t size)
		{
			(void)new (this) int[size];
		}

		auto& operator[](size_t idx)
		{
			return unwrap()[idx];
		}

		void fill(size_t size)
		{
			int a = 0, b = 1;
			for (size_t i = 0; i < size; i++)
			{
				unwrap()[i] = a;
				int c = a + b;
				a = b;
				b = c;
			}
		}

		int& operator*()
		{
			return *unwrap();
		}

		~IntArrayWrapper()
		{
			printf("IntArrayWrapper :: dtor\n");
		}

		int* unwrap()
		{
			return reinterpret_cast<int*>(this);
		}
	};

	TEST_F(MemoryManagerTest, TestArrayWrapper)
	{
		{
			void* mem = malloc(sizeof(int) * 32);
			IntArrayWrapper* aw = new (mem) IntArrayWrapper(32);

			(*aw)[0] = 1;
			(*aw)[1] = 2;
			(*aw)[31] = 32;

			EXPECT_EQ((*aw)[0], 1);
			EXPECT_EQ((*aw)[1], 2);
			EXPECT_EQ((*aw)[31], 32);

			aw->fill(32);

			for (size_t i = 0; i < 32; i++)
			{
				printf("%d\n", (*aw)[i]);
			}

			delete aw;
			//free(mem);
		}

		{
			AxHandle handle = apex::make_handle<int[32]>();
			EXPECT_EQ(MemoryManager::getAllocatedSize(), 128);

			auto pAW = new (handle) AxManagedClassAdapter<IntArrayWrapper>(32);
			IntArrayWrapper& aw = *pAW;

			aw.fill(32);

			for (size_t i = 0; i < 32; i++)
			{
				printf("%d\n", aw[i]);
			}

			delete pAW;
		}
		EXPECT_EQ(MemoryManager::getAllocatedSize(), 0);
	}

	TEST_F(MemoryManagerTest, TestAxArray)
	{
		EXPECT_EQ(MemoryManager::getAllocatedSize(), 0);
		{
			using math::Vector4;
			AxArray<Vector4> arr(32);
			EXPECT_EQ(MemoryManager::getAllocatedSize(), 512);

			arr.append({ 1.f, 0.f, 1.f, 1.f });
			arr.emplace_back( 2.f, 3.f, 0.f, 1.f );

			u32 i = 0;
			for (auto& element : arr)
			{
				if (i == 0) EXPECT_EQ(element, Vector4( 1.f, 0.f, 1.f, 1.f ));
				if (i == 1) EXPECT_EQ(element, Vector4( 2.f, 3.f, 0.f, 1.f ));

				++i;
			}
		}
		EXPECT_EQ(MemoryManager::getAllocatedSize(), 0);

		{
			AxArray<AxStringRef> strArr;
			strArr.reserve(32);
			EXPECT_EQ(MemoryManager::getAllocatedSize(), 512); // 32 * 16 (sizeof const char* (8) + sizeof size_t (8))

			strArr.append("Athang");
			strArr.emplace_back("Oishi");
			strArr[1] = "Oishi Saha";

			EXPECT_STREQ(strArr[0].c_str(), "Athang");
			EXPECT_STREQ(strArr[1].c_str(), "Oishi Saha");

			for (auto& str : strArr)
			{
				printf("%s\n", str.c_str());
			}

		}
		EXPECT_EQ(MemoryManager::getAllocatedSize(), 0);
	}

	TEST_F(MemoryManagerTest, TestAxArrayInitializerList)
	{
		EXPECT_EQ(MemoryManager::getAllocatedSize(), 0);
		{
			AxArray<int> iArr = { 1, 2, 3, 4 };
			EXPECT_EQ(MemoryManager::getAllocatedSize(), 32);

			EXPECT_EQ(iArr.size(), 4);
			EXPECT_EQ(iArr[0], 1);
			EXPECT_EQ(iArr[1], 2);
			EXPECT_EQ(iArr[2], 3);
			EXPECT_EQ(iArr[3], 4);
		}
		EXPECT_EQ(MemoryManager::getAllocatedSize(), 0);

		{
			AxArray<AxStringRef> strArr = { "Athang", "Oishi", "Saumitra" };
			EXPECT_EQ(MemoryManager::getAllocatedSize(), 48); // 3 * 16 (sizeof const char* (8) + sizeof size_t (8))

			EXPECT_EQ(strArr.size(), 3);
			EXPECT_STREQ(strArr[0].c_str(), "Athang");
			EXPECT_STREQ(strArr[1].c_str(), "Oishi");
			EXPECT_STREQ(strArr[2].c_str(), "Saumitra");
		}
		EXPECT_EQ(MemoryManager::getAllocatedSize(), 0);

		{
			AxArray<const char*> strArr = { "Athang", "Oishi", "Saumitra" };
			EXPECT_EQ(MemoryManager::getAllocatedSize(), 32); // 3 * 8 (but the closest available size is 32)

			EXPECT_EQ(strArr.size(), 3);
			EXPECT_STREQ(strArr[0], "Athang");
			EXPECT_STREQ(strArr[1], "Oishi");
			EXPECT_STREQ(strArr[2], "Saumitra");
		}
		EXPECT_EQ(MemoryManager::getAllocatedSize(), 0);
	}

	TEST_F(MemoryManagerTest, TestUniquePtrArray)
	{
		EXPECT_EQ(MemoryManager::getAllocatedSize(), 0);
		using Int = AxManagedClassAdapter<int>;
		{
			AxArray<UniquePtr<Int>> arr;
			arr.reserve(1000);

			EXPECT_GE(mem::MemoryManager::getAllocatedSize(), arr.capacity() * sizeof(UniquePtr<Int>));
		}
		EXPECT_EQ(mem::MemoryManager::getAllocatedSize(), 0);

		{
			AxArray<UniquePtr<StructWithDestructor>> arr;
			arr.reserve(1000);

			EXPECT_GE(mem::MemoryManager::getAllocatedSize(), arr.capacity() * sizeof(UniquePtr<StructWithDestructor>));

			for (int i = 0; i < 1000; i++)
			{
				arr.emplace_back(apex::make_unique<StructWithDestructor>());
			}
			EXPECT_EQ(StructWithDestructor::s_count, 1000);
		}
		EXPECT_EQ(StructWithDestructor::s_count, 0);
		EXPECT_EQ(mem::MemoryManager::getAllocatedSize(), 0);
	}

	static_assert(std::ranges::viewable_range<AxArray<int>>);

	TEST_F(MemoryManagerTest, TestAxArrayRanges)
	{
		{
			AxArray<int> iArr = { 1, 2, 3, 4, 5, 6, 7, 8 };
			static_assert(apex::ranges::range<decltype(iArr)>);

			ranges::AxRange range(iArr);
			{
				
			}
		}
	}

	TEST_F(MemoryManagerTest, TestSharedPtr)
	{
		ASSERT_EQ(MemoryManager::getAllocatedSize(), 0);
		{
			AxHandle hInt(sizeof(int));
			auto pInt = apex::shared_from_handle<int, cncy::NullLock>(hInt, 42);
			{
				auto pInt2 = pInt;
				EXPECT_EQ(*pInt, 42);
				EXPECT_EQ(*pInt2, 42);

				*pInt2 = 213;
				EXPECT_EQ(*pInt, 213);
				EXPECT_EQ(*pInt2, 213);

				EXPECT_EQ(pInt.use_count(), 2);
				EXPECT_EQ(pInt2.use_count(), 2);

			}
			*pInt = 1024;
			EXPECT_EQ(pInt.use_count(), 1);

			EXPECT_EQ(*pInt, 1024);
		}
		EXPECT_EQ(MemoryManager::getAllocatedSize(), 0);

		{
			AxHandle hStruct(sizeof(StructWithDestructor));
			auto pStruct = apex::shared_from_handle<StructWithDestructor, cncy::NullLock>(hStruct);
			EXPECT_EQ(StructWithDestructor::s_count, 1);
			{
				auto pStruct2 = pStruct;
				EXPECT_EQ(pStruct2.use_count(), 2);
				EXPECT_EQ(StructWithDestructor::s_count, 1);
			}
			EXPECT_EQ(StructWithDestructor::s_count, 1);
		}
		EXPECT_EQ(MemoryManager::getAllocatedSize(), 0);

		{
			AxHandle hArray = apex::make_handle<int[32]>();
			auto pArray = apex::shared_from_handle<int[], cncy::NullLock>(hArray, 32);
			EXPECT_EQ(MemoryManager::getAllocatedSize(), 128 + 32);

			auto pArray2 = pArray;
			EXPECT_EQ(pArray.use_count(), 2);
			EXPECT_EQ(pArray2.use_count(), 2);
		}
		EXPECT_EQ(MemoryManager::getAllocatedSize(), 0);

		{
			auto pArray = apex::make_shared<StructWithDestructor[], cncy::NullLock>(32);
			EXPECT_EQ(MemoryManager::getAllocatedSize(), 4096 + 32);

			auto pArray2 = pArray;
			{
				auto pArray3 = pArray2;
				EXPECT_EQ(pArray.use_count(), 3);
				EXPECT_EQ(pArray2.use_count(), 3);
				EXPECT_EQ(pArray3.use_count(), 3);
				EXPECT_EQ(StructWithDestructor::s_count, 32);
			}
			EXPECT_EQ(pArray.use_count(), 2);
			EXPECT_EQ(pArray2.use_count(), 2);
			EXPECT_EQ(StructWithDestructor::s_count, 32);
		}
		EXPECT_EQ(StructWithDestructor::s_count, 0);
		EXPECT_EQ(MemoryManager::getAllocatedSize(), 0);
	}

}
