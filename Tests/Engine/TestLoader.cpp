#include <gtest/gtest.h>

#include "Apex/ModelLoader.h"

TEST(TestLoader, TestLoader)
{
	apex::ModelLoader loader;

	(void)loader.readMetadata("assets/Box.glb");
}
