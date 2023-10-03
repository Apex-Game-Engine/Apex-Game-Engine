#include <array>
#include <vector>
#include <gtest/gtest.h>

#include "Containers/AxArray.h"
#include "Containers/AxRange.h"
#include "Containers/AxSparseMap.h"
#include "Containers/AxStringRef.h"
#include "Math/Vector3.h"
#include "Memory/MemoryManager.h"

TEST(TestContainers, TestFreeListInitialize)
{
	std::vector<uint64_t> aCounters(16);
	/*apex::containers::FreeList<uint64_t> flCounters{ aCounters.data(), aCounters.size() };

	for (uint64_t i = 0; i < 15; i++)
	{
		EXPECT_EQ(aCounters[i], reinterpret_cast<uintptr_t>(&aCounters[i+1]));
	}*/

}

namespace apex {

	struct TrivialType
	{
		int i;
		float f;
		char c;
	};

	struct NonTrivialType
	{
		NonTrivialType(int i, float f, char c) : i(i), f(f), c(c), str(new char[1024]) {}
		~NonTrivialType() { delete[] str; }

		int i;
		float f;
		char c;

		char* str;
	};

	class AxArrayTest : public testing::Test
	{
	public:
		void SetUp() override
		{
			memory::MemoryManager::initialize({ .frameArenaSize = 0, .numFramesInFlight = 0 });
		}

		void TearDown() override
		{
			memory::MemoryManager::shutdown();
		}

		//void floatArray_constructFromMemory(void* mem, size_t mem_size) { floatArray.constructFromMemory(mem, mem_size); }
		//void nonTrivialArray_constructFromMemory(void* mem, size_t mem_size) { nonTrivialArray.constructFromMemory(mem, mem_size); }

	protected:
		AxArray<float32> floatArray;
		AxArray<const char*> stringArray;
		AxArray<TrivialType> trivialArray;
		AxArray<NonTrivialType> nonTrivialArray;
	};

	TEST_F(AxArrayTest, TestFloatArrayFromStaticMemory)
	{

		constexpr size_t BUF_SIZE = 8 * sizeof(float);
		std::array<apex::uint8, BUF_SIZE> arenaBuf { 0 };

		return;

		//floatArray_constructFromMemory(arenaBuf.data(), BUF_SIZE);

		ASSERT_EQ(floatArray.size(), 0);
		ASSERT_EQ(floatArray.capacity(), 8);

		floatArray.append(1.23f);
		floatArray.append(2.34f);
		floatArray.append(3.45f);
		floatArray.append(4.56f);
		floatArray.append(5.67f);
		floatArray.append(6.78f);
		floatArray.append(7.89f);
		floatArray.append(9.00f);

		float val = 1.23f;
		for (auto& f : floatArray)
		{
			EXPECT_FLOAT_EQ(val, f);
			val += 1.11f;
		}

		ASSERT_DEATH({ floatArray.append(10.11f); }, "Array size exceeds capacity!");

		val = 1.23f;
		for (size_t i = 0; i < floatArray.size(); i++)
		{
			EXPECT_FLOAT_EQ(val, floatArray[i]);
			val += 1.11f;
		}

		floatArray.remove(2);
		val = 1.23f;
		for (size_t i = 0; i < floatArray.size(); i++)
		{
			if (i == 2)
				val += 1.11f;

			printf("%1.3f == %f\n", val, floatArray[i]);
			EXPECT_FLOAT_EQ(val, floatArray[i]);
			val += 1.11f;
		}
	}

	TEST_F(AxArrayTest, TestNonTrivialArrayFromStaticMemory)
	{
		constexpr size_t BUF_SIZE = 8 * sizeof(NonTrivialType);
		std::array<apex::uint8, BUF_SIZE> arenaBuf { 0 };

		return;
		// nonTrivialArray_constructFromMemory(arenaBuf.data(), BUF_SIZE);
		
		ASSERT_EQ(nonTrivialArray.size(), 0);
		ASSERT_EQ(nonTrivialArray.capacity(), 8);
	}

	TEST_F(AxArrayTest, TestKeepOnlyUniquesSlow)
	{
		AxArray<uint32> arr;
		arr.reserve(8);
		EXPECT_EQ(arr.size(), 0);

		arr.append(1);
		arr.append(1);
		arr.append(2);
		arr.append(1);
		arr.append(4);
		arr.append(2);
		arr.append(3);
		EXPECT_EQ(arr.size(), 7);

		keepUniquesOnly_slow(arr);

		EXPECT_EQ(arr.size(), 4);
		EXPECT_EQ(arr[0], 1);
		EXPECT_EQ(arr[1], 2);
		EXPECT_EQ(arr[2], 4);
		EXPECT_EQ(arr[3], 3);
	}

	TEST_F(AxArrayTest, TestExternallyManaged)
	{
		AxHandle handle = apex::make_handle<uint32[]>(32);
		AxArray<uint32, ExternallyManaged> arr(handle);
		arr.resize(32, 21);
		EXPECT_EQ(arr.size(), 32);

		for (auto& a : arr)
		{
			EXPECT_EQ(a, 21);
		}
	}

	TEST_F(AxArrayTest, TestResize)
	{
		AxArray<math::Vector3> positions;
		positions.resize(25, math::Vector3{ 2, 1, 3 });

		EXPECT_GE(positions.capacity(), 25); // capacity >= 25
		EXPECT_EQ(positions.size(), 25);

		printf("capacity: %llu\n", positions.capacity());

		for (auto& pos : positions)
		{
			EXPECT_TRUE(pos == math::Vector3(2, 1, 3));
		}

		positions.resize(50, math::Vector3{ 1, 7, 8 });

		EXPECT_GE(positions.capacity(), 50); // capacity >= 25
		EXPECT_EQ(positions.size(), 50);

		printf("capacity: %llu\n", positions.capacity());

		size_t i = 0;
		for (auto& pos : positions)
		{
			if (i < 25)
				EXPECT_TRUE(pos == math::Vector3(2, 1, 3));
			else
				EXPECT_TRUE(pos == math::Vector3(1, 7, 8));

			i++;
		}
	}

	TEST(AxStringRefTest, TestAxStringRef)
	{
		AxStringRef strRefArr[2];

		new (&strRefArr[1]) AxStringRef("oishi");

		for (auto strRef : strRefArr)
		{
			if (strRef)
				printf("%s\n", strRef.c_str());
		}
	}

	struct Transform
	{
		math::Vector3 position;
		math::Vector3 rotation;
		math::Vector3 scale;
	};

	TEST(AxSparseMapTest, TestSparseMap)
	{
		memory::MemoryManager::initialize({ 0, 0 });

		{
			AxSparseMap<uint32, Transform> sparseMap(10);
			EXPECT_EQ(sparseMap.capacity(), 10);
			EXPECT_EQ(sparseMap.count(), 0);

			sparseMap.insert(2, { .position = { 1, 2, 3 } });
			sparseMap.insert(9, { .position = { 11, 22, 33 } });
			sparseMap.insert(5, { .position = { 15, 25, 35 } });

			// Test IDs
			size_t i = 0;
			for (auto& id : sparseMap.keys())
			{
				switch (i)
				{
				case 0: EXPECT_EQ(id, 2); break;
				case 1: EXPECT_EQ(id, 9); break;
				case 2: EXPECT_EQ(id, 5); break;
				}

				++i;
			}
			EXPECT_EQ(i, 3);

			// Test elements
			i = 0;
			for (auto& element : sparseMap.elements())
			{
				switch (i)
				{
				case 0: EXPECT_TRUE(element.position == math::Vector3( 1, 2, 3 )); break;
				case 1: EXPECT_TRUE(element.position == math::Vector3( 11, 22, 33 )); break;
				case 2: EXPECT_TRUE(element.position == math::Vector3( 15, 25, 35 )); break;
				}

				++i;
			}
			EXPECT_EQ(i, 3);

			sparseMap.remove(2);

			// Test IDs after removal
			i = 0;
			for (auto& id : sparseMap.keys())
			{
				switch (i)
				{
				case 0: EXPECT_EQ(id, 5); break;
				case 1: EXPECT_EQ(id, 9); break;
				}

				++i;
			}
			EXPECT_EQ(i, 2);

			// Test elements after removal
			i = 0;
			for (auto& element : sparseMap.elements())
			{
				switch (i)
				{
				case 0: EXPECT_TRUE(element.position == math::Vector3( 15, 25, 35 )); break;
				case 1: EXPECT_TRUE(element.position == math::Vector3( 11, 22, 33 )); break;
				}

				++i;
			}
			EXPECT_EQ(i, 2);

			auto [idx, element] = sparseMap.get(9);
			EXPECT_EQ(idx, 1);
			EXPECT_TRUE(element.position == math::Vector3( 11, 22, 33 ));
		}

		memory::MemoryManager::shutdown();
	}

}
