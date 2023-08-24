﻿#include <gtest/gtest.h>
#include "Api.h"
#include "CommandLineParser.h"

TEST(TestCommandLineParser, TestParser)
{
	apex::CommandLineParser cmdLine;
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
