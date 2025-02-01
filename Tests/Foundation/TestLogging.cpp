#include <gtest/gtest.h>
#include "Core/Asserts.h"
#include "Core/Logging.h"

void testDebug()
{
	axDebug("Debug message");
}

struct DummySink : public apex::logging::ISink
{
	void log(const apex::logging::LogMsg& log_msg) override
	{
		logMsgs.push_back(log_msg);
	}

	std::vector<apex::logging::LogMsg> logMsgs;
};

TEST(TestLogging, TestLogger)
{
	DummySink dummySink;
	apex::logging::Logger::get().addSink(&dummySink);
	testDebug();
	EXPECT_EQ(dummySink.logMsgs.size(), 1);
	EXPECT_EQ(dummySink.logMsgs[0].level, apex::logging::LogLevel::Debug);
	EXPECT_EQ(dummySink.logMsgs[0].lineno, 7);
	EXPECT_STREQ(dummySink.logMsgs[0].filename, "TestLogging.cpp");
	EXPECT_STREQ(dummySink.logMsgs[0].filepath, __FILE__);
	EXPECT_STREQ(dummySink.logMsgs[0].msg, "Debug message");

	apex::logging::Logger::get().removeSink(&dummySink);


	//axLogFmt("Hello {}", "World");
	//axDebugFmt("Hello {}", "World");
	//axWarnFmt("Hello {}", "World");
	//axErrorFmt("Hello {}", "World");
}

TEST(TestAsserts, TestAssert)
{
#ifdef APEX_CONFIG_DEBUG
	ASSERT_DEATH(axAssert(1 == 0);, ".*");
	ASSERT_DEATH(axAssertFmt(1 == 0, "Test");, ".*");
#endif
}

