#include <gtest/gtest.h>

#include "Core/Files.h"
#include "Memory/MemoryManager.h"

TEST(TestFiles, TestReadFile)
{
	apex::mem::MemoryManager::initialize({});

	auto filebuf = apex::readFile("D:\\Repos\\ApexGameEngine-Vulkan\\README.md");

	
}