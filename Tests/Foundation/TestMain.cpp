#include <cstdio>
#include <gtest/gtest.h>

#include <Core/Logging.h>

#include "Memory/MemoryManager.h"

GTEST_API_ int main(int argc, char **argv) {
	printf("Running main() from %s\n", __FILE__);
	testing::InitGoogleTest(&argc, argv);

	apex::logging::Logger::Init();

	return RUN_ALL_TESTS();
}
