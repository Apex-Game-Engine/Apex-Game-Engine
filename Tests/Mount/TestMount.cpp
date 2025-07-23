#include <cstdio>
#include <gtest/gtest.h>

#include "Core/Logging.h"
#include "Mount/Mount.h"

GTEST_API_ int main(int argc, char **argv) {
	printf("Running main() from %s\n", __FILE__);
	testing::InitGoogleTest(&argc, argv);

	apex::logging::Logger::Init();
	apex::mem::MemoryManager::initialize({});

	int res = RUN_ALL_TESTS();

	apex::mem::MemoryManager::shutdown();
	return res;
}

using namespace std::literals;

TEST(PathTest, TestValidatePathSimple)
{
	apex::mnt::Path path("assets://meshes/characters/YBot.axmesh");

	ASSERT_TRUE(path.ValidatePath());

	EXPECT_EQ(path.GetMount(),			"assets"sv);
	EXPECT_EQ(path.GetDirectoryPath(),	"meshes/characters"sv);
	EXPECT_EQ(path.GetDirectoryName(),	"characters"sv);
	EXPECT_EQ(path.GetFilePath(),		"meshes/characters/YBot.axmesh"sv);
	EXPECT_EQ(path.GetFileName(),		"YBot.axmesh"sv);
	EXPECT_EQ(path.GetFileBaseName(),	"YBot"sv);
	EXPECT_EQ(path.GetFileExtension(),	"axmesh"sv);
}

TEST(PathTest, TestValidatePathWithNumbers)
{
	apex::mnt::Path path("audio00://audio/test_samples/sample15362.ogg1");

	ASSERT_TRUE(path.ValidatePath());

	EXPECT_EQ(path.GetMount(),			"audio00"sv);
	EXPECT_EQ(path.GetDirectoryPath(),	"audio/test_samples"sv);
	EXPECT_EQ(path.GetDirectoryName(),	"test_samples"sv);
	EXPECT_EQ(path.GetFilePath(),		"audio/test_samples/sample15362.ogg1"sv);
	EXPECT_EQ(path.GetFileName(),		"sample15362.ogg1"sv);
	EXPECT_EQ(path.GetFileBaseName(),	"sample15362"sv);
	EXPECT_EQ(path.GetFileExtension(),	"ogg1"sv);

	EXPECT_EQ(path.HasFileName(), true);
	EXPECT_EQ(path.HasFileExtension(), true);
}

TEST(PathTest, TestValidatePathNoExtension)
{
	apex::mnt::Path path("anim://Animations/Data/Chapter1");

	ASSERT_TRUE(path.ValidatePath());

	EXPECT_EQ(path.GetMount(),			"anim"sv);
	EXPECT_EQ(path.GetDirectoryPath(),	"Animations/Data"sv);
	EXPECT_EQ(path.GetDirectoryName(),	"Data"sv);
	EXPECT_EQ(path.GetFilePath(),		"Animations/Data/Chapter1"sv);
	EXPECT_EQ(path.GetFileName(),		"Chapter1"sv);
	EXPECT_EQ(path.GetFileBaseName(),	"Chapter1"sv);
	EXPECT_EQ(path.GetFileExtension(),	""sv);

	EXPECT_EQ(path.HasFileName(), true);
	EXPECT_EQ(path.HasFileExtension(), false);
}

TEST(PathTest, TestValidatePathNoFileName)
{
	apex::mnt::Path path("anim://Animations/Data/Chapter1/");

	ASSERT_TRUE(path.ValidatePath());

	EXPECT_EQ(path.GetMount(),			"anim"sv);
	EXPECT_EQ(path.GetDirectoryPath(),	"Animations/Data/Chapter1"sv);
	EXPECT_EQ(path.GetDirectoryName(),	"Chapter1"sv);
	EXPECT_EQ(path.GetFilePath(),		"Animations/Data/Chapter1/"sv);
	EXPECT_EQ(path.GetFileName(),		""sv);
	EXPECT_EQ(path.GetFileBaseName(),	""sv);
	EXPECT_EQ(path.GetFileExtension(),	""sv);

	EXPECT_EQ(path.HasFileName(), false);
	EXPECT_EQ(path.HasFileExtension(), false);
}
