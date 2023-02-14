#include <gtest/gtest.h>
#include "Foundation/axAsserts.h"
#include "Foundation/axLogging.h"

void testDebug()
{
	axDebug("Debug message");
}

TEST(TestLogging, TestConsoleLogger)
{
	testing::internal::CaptureStdout();

	apex::logging::ConsoleSink_st consoleSink;

	apex::logging::Logger::initialize();
	apex::logging::Logger::get().m_sinks.push_back(&consoleSink);

	testDebug();

	std::string output = testing::internal::GetCapturedStdout();
	printf("output: %s\n", output.c_str());
	ASSERT_TRUE(output == "[C:\\Users\\athan\\source\\repos\\ApexGameEngine-Vulkan\\Tests\\Foundation\\TestLogging.cpp::(testDebug):7] <Debug> :: Debug message\n"
		|| output == "[TestLogging.cpp::(testDebug):7] <Debug> :: Debug message\n"
	);
}

TEST(TestAsserts, TestAssert)
{
#ifdef APEX_CONFIG_DEBUG
	ASSERT_DEATH(axAssert(1 == 0);, ".*");
#endif
}

