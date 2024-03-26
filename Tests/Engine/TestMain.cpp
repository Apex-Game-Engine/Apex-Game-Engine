#include <gtest/gtest.h>

#include "Core/Logging.h"
#include "Memory/MemoryManager.h"

GTEST_API_ int main(int argc, char **argv) {
	printf("Running main() from %s\n", __FILE__);
	testing::InitGoogleTest(&argc, argv);

	apex::logging::Logger::initialize();
	//apex::memory::MemoryManager::initialize({ .frameArenaSize = 0, .numFramesInFlight = 3 });

	int retval = RUN_ALL_TESTS();

	// apex::memory::MemoryManager::shutdown();

	return retval;
}