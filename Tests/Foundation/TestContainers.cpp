#include <array>
#include <gtest/gtest.h>

#include "Containers/AxArray.h"
#include "Containers/AxRange.h"
#include "Containers/AxStringRef.h"

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
		void floatArray_constructFromMemory(void* mem, size_t mem_size) { floatArray.constructFromMemory(mem, mem_size); }
		void nonTrivialArray_constructFromMemory(void* mem, size_t mem_size) { nonTrivialArray.constructFromMemory(mem, mem_size); }

	protected:
		AxArray<float32> floatArray;
		AxArray<const char*> stringArray;
		AxArray<TrivialType> trivialArray;
		AxArray<NonTrivialType> nonTrivialArray;
	};

	TEST_F(AxArrayTest, TestFloatArray)
	{
		constexpr size_t BUF_SIZE = 8 * sizeof(float);
		std::array<apex::uint8, BUF_SIZE> arenaBuf { 0 };

		floatArray_constructFromMemory(arenaBuf.data(), BUF_SIZE);

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

	TEST_F(AxArrayTest, TestNonTrivialArray)
	{
		constexpr size_t BUF_SIZE = 8 * sizeof(NonTrivialType);
		std::array<apex::uint8, BUF_SIZE> arenaBuf { 0 };

		nonTrivialArray_constructFromMemory(arenaBuf.data(), BUF_SIZE);
		
		ASSERT_EQ(nonTrivialArray.size(), 0);
		ASSERT_EQ(nonTrivialArray.capacity(), 8);
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

	TEST(AxRangeTest, TestAxRange)
	{
		{
			std::vector<int> v = { 1, 2, 4, 6, 3, 4, 5, 7, 8, 9 };

			ranges::AxRange<std::vector<int>> range (v.begin(), v.end());
			{
				int i = 0;
				for (int &value : range)
				{
					printf("%d, ", value);
					EXPECT_EQ(value, v[i]);
					i++;
				}
				printf("\n");
				EXPECT_EQ(i, 10);
			}

			{
				ranges::AxView view ( range, [](int const& i) { return i % 2 == 0; } );

				int i = 0;
				for (int value : view)
				{
					printf("%d, ", value);
					switch (i)
					{
					case 0: EXPECT_EQ(value, 2); break;
					case 1: EXPECT_EQ(value, 4); break;
					case 2: EXPECT_EQ(value, 6); break;
					case 3: EXPECT_EQ(value, 4); break;
					case 4: EXPECT_EQ(value, 8); break;
					}
					i++;
				}
				printf("\n");
				EXPECT_EQ(i, 5);
			}

			{
				const ranges::AxView view ( v, [](int const& i) { return i % 2 == 0; } );

				int i = 0;
				for (int value : view)
				{
					printf("%d, ", value);
					switch (i)
					{
					case 0: EXPECT_EQ(value, 2); break;
					case 1: EXPECT_EQ(value, 4); break;
					case 2: EXPECT_EQ(value, 6); break;
					case 3: EXPECT_EQ(value, 4); break;
					case 4: EXPECT_EQ(value, 8); break;
					}
					i++;
				}
				printf("\n");
				EXPECT_EQ(i, 5);
			}
		}
	}
}
