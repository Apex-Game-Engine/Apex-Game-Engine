#include <cstdio>
#include <gtest/gtest.h>

#include "Core/Logging.h"
#include "Math/Vector3.h"
#include "Entity/Entity.h"
#include "Entity/View.h"

GTEST_API_ int main(int argc, char **argv) {
	printf("Running main() from %s\n", __FILE__);
	testing::InitGoogleTest(&argc, argv);

	apex::logs::Logger::Init(false);

	return RUN_ALL_TESTS();
}

struct CPosition { apex::math::Vector3 position; };
struct CVelocity { apex::math::Vector3 velocity; };

TEST(TypeInfoTest, TestTypeName)
{
	EXPECT_STREQ("CPosition", apex::ecs::typeof<CPosition>().name().data());

	{
		CPosition pos;
		EXPECT_STREQ("CPosition", apex::ecs::typeof(pos).name().data());
	}
}

TEST(TypeInfoTest, TestTypeHash)
{
	static_assert(apex::ecs::TypeInfo<CPosition>::hash() == 0x52bf3bc23aa9cc6d);
	{
		CPosition pos;
		EXPECT_EQ(apex::ecs::typeof(pos).hash(), 0x52bf3bc23aa9cc6d);
	}
}

class RegistryTest : public testing::Test
{
public:
	void SetUp() override
	{
		apex::mem::MemoryManager::initialize({ .frameArenaSize = 0, .numFramesInFlight = 0 });
		m_pRegistry = apex::make_unique<apex::ecs::Registry>();
	}

	void TearDown() override
	{
		m_pRegistry = nullptr;
		apex::mem::MemoryManager::shutdown();
	}

	apex::ecs::Registry& GetRegistry() const { return *m_pRegistry; }

private:
	apex::UniquePtr<apex::ecs::Registry> m_pRegistry;
};

/*
TEST_F(RegistryTest, TestCreateEntity)
{
	apex::ecs::Registry& reg = GetRegistry();

	for (uint32_t i = 0; i < (1 << 20); i++)
	{
		auto e = reg.CreateEntity();
		EXPECT_EQ(e.id.value, i);
	}
	auto e = reg.CreateEntity();
	EXPECT_EQ(e.masks.identifier, 0);
	EXPECT_EQ(e.masks.version, 1);
}
*/

TEST_F(RegistryTest, TestRegisterComponent)
{
	apex::ecs::Registry& reg = GetRegistry();

	reg.RegisterComponent<CPosition>();

	auto positions = reg.GetComponentPool<CPosition>();
	EXPECT_NE(positions, nullptr);

	//auto velocities = reg.GetComponentPool<CVelocity>();
	//EXPECT_EQ(velocities, nullptr);
}

TEST_F(RegistryTest, TestAddAndGetSingleComponent)
{
	apex::ecs::Registry& reg = GetRegistry();

	reg.RegisterComponent<CPosition>();

	apex::ecs::EntityId e;
	{
		e = reg.CreateEntity();
		auto& pos = reg.Add<CPosition>(e);
		pos.position = { 15.0, 35.0, 99.99 };
	}

	{
		auto opt = reg.Get<CPosition>(e);
		EXPECT_TRUE(opt.has_value());
		auto& pos = opt.value().get();
		EXPECT_FLOAT_EQ(pos.position.x, 15.00f);
		EXPECT_FLOAT_EQ(pos.position.y, 35.00f);
		EXPECT_FLOAT_EQ(pos.position.z, 99.99f);
	}
}

TEST_F(RegistryTest, TestComponentPoolResize)
{
	apex::ecs::Registry& reg = GetRegistry();
	reg.RegisterComponent<CPosition>();

	for (int i = 0; i < 500000; i++)
	{
		auto e = reg.CreateEntity();
		reg.Add<CPosition>(e);
	}
}

TEST_F(RegistryTest, TestSingleComponentView)
{
	apex::ecs::Registry& reg = GetRegistry();
	reg.RegisterComponent<CPosition>();
	reg.RegisterComponent<CVelocity>();

	auto view = reg.View<CPosition>();

	{
		auto e = reg.CreateEntity();
		auto& pos = reg.Add<CPosition>(e);
		pos.position = { 12, 34, 56 };
	}
	{
		auto e = reg.CreateEntity();
		auto& pos = reg.Add<CPosition>(e);
		auto& vel = reg.Add<CVelocity>(e);
		pos.position = { 99, 88, 77 };
		vel.velocity = { 11, 22, 33 };
	}
	{
		auto e = reg.CreateEntity();
		auto& vel = reg.Add<CVelocity>(e);
		vel.velocity = { 1, 2, 3 };
	}

	apex::u32 count = 0;
	view.for_each([&count](auto& pos)
	{
		count++;
	});
	EXPECT_EQ(count, 2);

	{
		auto& pos = view.get({0});
		EXPECT_FLOAT_EQ(pos.position.x, 12);
		EXPECT_FLOAT_EQ(pos.position.y, 34);
		EXPECT_FLOAT_EQ(pos.position.z, 56);
	}

	{
		for (auto e : view.entities())
		{
			EXPECT_LE(e, 2);
		}
	}
}

TEST_F(RegistryTest, TestMultiComponentView)
{
	apex::ecs::Registry& reg = GetRegistry();
	reg.RegisterComponent<CPosition>();
	reg.RegisterComponent<CVelocity>();

	auto view = reg.View<CPosition, CVelocity>();

	{
		auto e = reg.CreateEntity();
		auto& pos = reg.Add<CPosition>(e);
		pos.position = { 12, 34, 56 };
	}
	{
		auto e = reg.CreateEntity();
		auto& pos = reg.Add<CPosition>(e);
		auto& vel = reg.Add<CVelocity>(e);
		pos.position = { 99, 88, 77 };
		vel.velocity = { 11, 22, 33 };
	}
	{
		auto e = reg.CreateEntity();
		auto& vel = reg.Add<CVelocity>(e);
		vel.velocity = { 1, 2, 3 };
	}

	apex::u32 count = 0;
	view.for_each([&count](auto& pos, auto& vel)
	{
		EXPECT_FLOAT_EQ(pos.position.x, 99);
		EXPECT_FLOAT_EQ(pos.position.y, 88);
		EXPECT_FLOAT_EQ(pos.position.z, 77);

		EXPECT_FLOAT_EQ(vel.velocity.x, 11);
		EXPECT_FLOAT_EQ(vel.velocity.y, 22);
		EXPECT_FLOAT_EQ(vel.velocity.z, 33);

		count++;
	});
	EXPECT_EQ(count, 1);

	EXPECT_FALSE(view.contains({0}));
	EXPECT_TRUE(view.contains({1}));
	EXPECT_FALSE(view.contains({2}));

	{
		//auto it = view.entities();
	}
}

TEST_F(RegistryTest, TestMultiComponentViewIterations)
{
	auto& reg = GetRegistry();

	reg.RegisterComponent<CPosition>();
	reg.RegisterComponent<CVelocity>();

	apex::u32 pcnt = 0, vcnt = 0, pvcnt = 0;
	for (int i = 0; i < 500000; i++)
	{
		auto e = reg.CreateEntity();
		int both = 0;
		if (i % 2 == 0)
		{
			reg.Add<CPosition>(e);
			pcnt++;
			both++;
		}
		if (i % 3 == 0 || i % 7 == 0)
		{
			reg.Add<CVelocity>(e);
			vcnt++;
			both++;
		}
		pvcnt += both == 2;
	}	

	{
		apex::u32 count = 0;
		reg.View<CPosition>().for_each([&count](auto& UNUSED_ARG(pos))
		{
			count++;
		});
		EXPECT_EQ(count, pcnt);
	}
	{
		apex::u32 count = 0;
		reg.View<CVelocity>().for_each([&count](auto& UNUSED_ARG(vel))
		{
			count++;
		});
		EXPECT_EQ(count, vcnt);
	}
	{
		apex::u32 count = 0;
		reg.View<CPosition, CVelocity>().for_each([&count](auto& UNUSED_ARG(pos), auto& UNUSED_ARG(vel))
		{
			count++;
		});
		EXPECT_EQ(count, pvcnt);
	}
}
