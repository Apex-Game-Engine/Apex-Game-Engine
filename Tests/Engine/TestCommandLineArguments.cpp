#include <gtest/gtest.h>
#include "Api.h"
#include "CommandLineArguments.h"
#include "Memory/MemoryManager.h"

class TestCommandLineArguments : public ::testing::Test
{
public:
	static void SetUpTestSuite()
	{
		apex::memory::MemoryManager::initialize({0, 0});
	}

	static void TearDownTestSuite()
	{
		apex::memory::MemoryManager::shutdown();
	}
};

TEST_F(TestCommandLineArguments, TestParser)
{
	apex::CommandLineArguments cmdLine;
	cmdLine.defineOptions({
		{ .shortName = "h", .longName = "headless" },
		{ .shortName = "l", .longName = "load", .hasValue = true }
	});

	const char* argv[] = {
		"-h",
		"--load",
		"Assets\\Levels\\mainLevel.apx"
	};

	cmdLine.parse(std::size(argv), argv);

	int count = 0;
	for (auto& opt : cmdLine.getSelectedOptions())
	{
		switch (count)
		{
		case 0:
			EXPECT_STREQ(opt.longName, "headless");
			break;
		case 1:
			EXPECT_STREQ(opt.longName, "load");
			EXPECT_STREQ(opt.value, argv[2]);
			break;
		}
		count++;
	}
}

TEST_F(TestCommandLineArguments, TestCmdString)
{
	apex::CommandLineArguments cmdLine;
	cmdLine.defineOptions({
		{ .shortName = "h", .longName = "headless" },
		{ .shortName = "l", .longName = "load", .hasValue = true }
	});

	const char* cmdline = "-h --load Assets\\Levels\\mainLevel.apx";

	cmdLine.parse(cmdline);

	int count = 0;
	for (auto& opt : cmdLine.getSelectedOptions())
	{
		switch (count)
		{
		case 0:
			EXPECT_STREQ(opt.longName, "headless");
			break;
		case 1:
			EXPECT_STREQ(opt.longName, "load");
			EXPECT_STREQ(opt.value, "Assets\\Levels\\mainLevel.apx");
			break;
		}
		count++;
	}
}
