#include <gtest/gtest.h>

#include "Core/Files.h"
#include "Memory/MemoryManager.h"

TEST(TestFiles, TestReadFile)
{
	apex::mem::MemoryManager::initialize({});

	apex::File file = apex::File::OpenExisting(R"(D:\Repos\ApexGameEngine-Vulkan\README.md)");
	auto filebuf = file.Read();
}