#include <gtest/gtest.h>
#include "Foundation/axConcurrency.h"

#include <queue>

TEST(TestConcurrency, TestSpinLock)
{
	uint32_t numThreads = std::thread::hardware_concurrency();
	constexpr static uint32_t NUM_ITERATIONS = 1024 * 1024;

	std::queue<int> q;
	apex::concurrency::SpinLock spinLock;

	uint32_t accumulator = 0;

	std::vector<std::thread> threads(numThreads);

	for (uint32_t t = 0; t < numThreads; t++)
	{
		threads[t] = std::thread([&spinLock, t, &accumulator]
		{
			apex::concurrency::LockGuard lock(spinLock);
			// std::cout << "Thread [" << i << "] :: running" << '\n';

			for (uint32_t i = 0u; i < NUM_ITERATIONS; i++)
				accumulator += 1;

			//std::this_thread::sleep_for(std::chrono::milliseconds(200));
		});

	}

	for (std::thread& thread : threads)
	{
		thread.join();
	}

	EXPECT_EQ(accumulator, numThreads * NUM_ITERATIONS);
}
