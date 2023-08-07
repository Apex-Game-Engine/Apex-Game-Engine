#include <cstdio>
#include <Memory/AxManagedClass.h>
#include <gtest/gtest.h>

#include <Core/Logging.h>

#include "Memory/MemoryManager.h"

GTEST_API_ int main(int argc, char **argv) {
	printf("Running main() from %s\n", __FILE__);

	apex::logging::Logger::initialize();

	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
