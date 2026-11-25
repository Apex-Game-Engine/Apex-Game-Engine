#include <cstdio>
#include <gtest/gtest.h>

#include "Core/Logging.h"
#include "Mount/Mount.h"

GTEST_API_ int main(int argc, char **argv) {
	printf("Running main() from %s\n", __FILE__);
	testing::InitGoogleTest(&argc, argv);

	apex::logging::Logger::Init(false);
	apex::mem::MemoryManager::initialize({});

	int res = RUN_ALL_TESTS();

	apex::mem::MemoryManager::shutdown();
	return res;
}

using namespace std::literals;

TEST(PathTest, TestValidatePathSimple)
{
	apex::mnt::Path path("assets://meshes/characters/YBot.axmesh");

	ASSERT_TRUE(path.IsValid());
	
	EXPECT_TRUE(path.HasMount());
	EXPECT_TRUE(path.HasFileName());
	EXPECT_TRUE(path.HasFileExtension());

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

	ASSERT_TRUE(path.IsValid());

	EXPECT_TRUE(path.HasMount());
	EXPECT_TRUE(path.HasFileName());
	EXPECT_TRUE(path.HasFileExtension());

	EXPECT_EQ(path.GetMount(),			"audio00"sv);
	EXPECT_EQ(path.GetDirectoryPath(),	"audio/test_samples"sv);
	EXPECT_EQ(path.GetDirectoryName(),	"test_samples"sv);
	EXPECT_EQ(path.GetFilePath(),		"audio/test_samples/sample15362.ogg1"sv);
	EXPECT_EQ(path.GetFileName(),		"sample15362.ogg1"sv);
	EXPECT_EQ(path.GetFileBaseName(),	"sample15362"sv);
	EXPECT_EQ(path.GetFileExtension(),	"ogg1"sv);
}

TEST(PathTest, TestValidatePathNoExtension)
{
	apex::mnt::Path path("anim://Animations/Data/Chapter1");

	ASSERT_TRUE(path.IsValid());

	EXPECT_TRUE(path.HasMount());
	EXPECT_TRUE(path.HasFileName());
	EXPECT_FALSE(path.HasFileExtension());

	EXPECT_EQ(path.GetMount(),			"anim"sv);
	EXPECT_EQ(path.GetDirectoryPath(),	"Animations/Data"sv);
	EXPECT_EQ(path.GetDirectoryName(),	"Data"sv);
	EXPECT_EQ(path.GetFilePath(),		"Animations/Data/Chapter1"sv);
	EXPECT_EQ(path.GetFileName(),		"Chapter1"sv);
	EXPECT_EQ(path.GetFileBaseName(),	"Chapter1"sv);
	EXPECT_EQ(path.GetFileExtension(),	""sv);
}

TEST(PathTest, TestValidatePathNoFileName)
{
	apex::mnt::Path path("anim://Animations/Data/Chapter1/");

	ASSERT_TRUE(path.IsValid());

	EXPECT_TRUE(path.HasMount());
	EXPECT_FALSE(path.HasFileName());
	EXPECT_FALSE(path.HasFileExtension());

	EXPECT_EQ(path.GetMount(),			"anim"sv);
	EXPECT_EQ(path.GetDirectoryPath(),	"Animations/Data/Chapter1"sv);
	EXPECT_EQ(path.GetDirectoryName(),	"Chapter1"sv);
	EXPECT_EQ(path.GetFilePath(),		"Animations/Data/Chapter1/"sv);
	EXPECT_EQ(path.GetFileName(),		""sv);
	EXPECT_EQ(path.GetFileBaseName(),	""sv);
	EXPECT_EQ(path.GetFileExtension(),	""sv);
}

TEST(PathTest, TestValidatePathNoMount)
{
	apex::mnt::Path path("D:/Game/Animations/Data/Chapter1/");

	ASSERT_TRUE(path.IsValid());

	EXPECT_FALSE(path.HasMount());
	EXPECT_FALSE(path.HasFileName());
	EXPECT_FALSE(path.HasFileExtension());

	EXPECT_EQ(path.GetMount(),			""sv);
	EXPECT_EQ(path.GetDirectoryPath(),	"D:/Game/Animations/Data/Chapter1"sv);
	EXPECT_EQ(path.GetDirectoryName(),	"Chapter1"sv);
	EXPECT_EQ(path.GetFilePath(),		"D:/Game/Animations/Data/Chapter1/"sv);
	EXPECT_EQ(path.GetFileName(),		""sv);
	EXPECT_EQ(path.GetFileBaseName(),	""sv);
	EXPECT_EQ(path.GetFileExtension(),	""sv);
}

TEST(MountTest, TestLocalMount)
{
	apex::mnt::LocalMount localMnt("samples", R"(X:\Tests\Assets)");

}

TEST(MountTest, TestResolveRelative)
{
	apex::mnt::Path path("anim://Animations/Data/Chapter1/intro_cs01_ma101.anim");

	ASSERT_TRUE(path.IsValid());

	EXPECT_EQ(path.GetMount(),			"anim"sv);
	EXPECT_EQ(path.GetDirectoryPath(),	"Animations/Data/Chapter1"sv);
	EXPECT_EQ(path.GetDirectoryName(),	"Chapter1"sv);
	EXPECT_EQ(path.GetFilePath(),		"Animations/Data/Chapter1/intro_cs01_ma101.anim"sv);
	EXPECT_EQ(path.GetFileName(),		"intro_cs01_ma101.anim"sv);
	EXPECT_EQ(path.GetFileBaseName(),	"intro_cs01_ma101"sv);
	EXPECT_EQ(path.GetFileExtension(),	"anim"sv);

	apex::mnt::Path relPath = path.ResolveRelative("../Chapter2/ch2_cs08_fa122.anim");

	ASSERT_TRUE(relPath.IsValid());

	EXPECT_TRUE(relPath.HasMount());
	EXPECT_TRUE(relPath.HasFileName());
	EXPECT_TRUE(relPath.HasFileExtension());

	EXPECT_EQ(relPath.GetMount(),			"anim"sv);
	EXPECT_EQ(relPath.GetDirectoryPath(),	"Animations/Data/Chapter2"sv);
	EXPECT_EQ(relPath.GetDirectoryName(),	"Chapter2"sv);
	EXPECT_EQ(relPath.GetFilePath(),		"Animations/Data/Chapter2/ch2_cs08_fa122.anim"sv);
	EXPECT_EQ(relPath.GetFileName(),		"ch2_cs08_fa122.anim"sv);
	EXPECT_EQ(relPath.GetFileBaseName(),	"ch2_cs08_fa122"sv);
	EXPECT_EQ(relPath.GetFileExtension(),	"anim"sv);
}