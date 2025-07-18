#include <vector>
#include <gtest/gtest.h>

#include "Containers/AxRange.h"

namespace apex {

	class AxRangeTest : public testing::Test
	{
	public:
		void SetUp() override
		{
			vec = { 1, 2, 4, 6, 3, 4, 5, 7, 8, 9 };
		}

	protected:
		std::vector<int> vec;
	};

	TEST_F(AxRangeTest, TestAxRangeIteration)
	{
		ranges::AxRange<std::vector<int>> r(vec.begin(), vec.end());
		{
			int i = 0;
			for (int &value : r)
			{
				printf("%d, ", value);
				EXPECT_EQ(value, vec[i]);
				i++;
			}
			printf("\n");
			EXPECT_EQ(i, 10);
		}
	}

	TEST_F(AxRangeTest, TestAxViewFromAxRange)
	{
		ranges::AxRange<std::vector<int>> range (vec.begin(), vec.end());
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

	TEST_F(AxRangeTest, TestAxViewFromVector)
	{
		const ranges::AxView view ( vec, [](int const& i) { return i % 2 == 0; } );

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

	TEST_F(AxRangeTest, TestAxViewFromConstRefViewFn)
	{
		auto isEven = [](int const& i) { return i % 2 == 0; };

		int i = 0;
		for (auto value : ranges::AxView( vec, isEven ))
		{
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
		EXPECT_EQ(i, 5);
	}

	TEST_F(AxRangeTest, TestAxViewFuncLogicalAnd)
	{
		std::vector nums = { 1, 2, 3, 45, 23, 51, 2, 612, 34, 5 };
		using namespace apex::ranges::operators;

		{
			auto isEven = [](int const& i) { return i % 2 == 0; };
			auto isDivisibleBy3 = [](int const& i) { return i % 3 == 0; };

			for (auto v : ranges::AxView( nums, isEven ))
			{
				printf("%d, ", v);
			}
			printf("\n");

			for (auto v : ranges::AxView( nums, isDivisibleBy3 ))
			{
				printf("%d, ", v);
			}
			printf("\n");

			for (auto v : ranges::AxView( nums, isDivisibleBy3 && isEven ))
			{
				printf("%d, ", v);
			}
			printf("\n");
		}
	}

	TEST_F(AxRangeTest, TestAxViewNested)
	{
		{
			auto isEven = [](int const& i) { return i % 2 == 0; };
			auto isDivisibleBy3 = [](int const& i) { return i % 3 == 0; };

			for (auto v : ranges::view( ranges::AxView( vec, isEven ), isDivisibleBy3 ))
			{
				printf("%d, ", v);
			}
			printf("\n");
		}
	}

	TEST_F(AxRangeTest, TestReversed)
	{
		{
			for (auto v : ranges::reversed(vec))
			{
				printf("%d, ", v);
			}
			printf("\n");
		}
	}

}
