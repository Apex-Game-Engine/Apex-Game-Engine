#include <gtest/gtest.h>

#include "Apex/ECS/Registry.h"
#include "Math/Vector3.h"
#include "Memory/MemoryManager.h"

namespace apex
{
	struct Transform
	{
		math::Vector3 position;
		math::Vector3 rotation;
		math::Vector3 scale;
	};

	struct DamageInflictor
	{
		apex::float32 damage;
	};
}

class TestEcs : public testing::Test
{
public:
	void SetUp() override
	{
		apex::memory::MemoryManager::initialize({ 0, 0 });
	}

	void TearDown() override
	{
		apex::memory::MemoryManager::shutdown();
	}
};

TEST_F(TestEcs, TestTypeIndex)
{
	auto id = apex::ecs::TypeIndex<apex::uint32>::value();
	printf("%d\n", id);
}

TEST_F(TestEcs, TestEntityBits)
{
	apex::ecs::Entity entity;

	entity.masks.identifier = 0x0;
	entity.masks.version = 0xfff;

	EXPECT_EQ(entity, 0xfff00000);
}

TEST_F(TestEcs, TestRegistryCreateEntity)
{
	apex::ecs::Registry registry;

	apex::ecs::Entity entity = registry.createEntity();
	{
		auto& transform = registry.add<apex::Transform>(entity);
		transform.position = { 2, 1, 3 };
	}

	{
		apex::Transform& transform = registry.get<apex::Transform>(entity).value();
		EXPECT_TRUE(transform.position == apex::math::Vector3(2, 1, 3));
	}
}

TEST_F(TestEcs, TestSingleComponentView)
{
	apex::ecs::Registry registry { .minPoolSize = 16 };

	int i;
	for (i = 0; i < 20; i++)
	{
		apex::ecs::Entity entity = registry.createEntity();
		auto& transform = registry.add<apex::Transform>(entity);
		transform.position = { 10.f + i, 20.f + i, 30.f + i };
	}

	i = 0;

	auto view = registry.view<apex::Transform>();
	view.each([&registry, &i](apex::ecs::Entity entity)
	{
		EXPECT_EQ(entity, static_cast<apex::uint32>(i));

		printf("entity: %u\n", static_cast<apex::uint32>(entity));
		apex::Transform& transform = registry.get<apex::Transform>(entity).value();
		printf("%f %f %f\n", transform.position.x, transform.position.y, transform.position.z);

		EXPECT_TRUE(transform.position == apex::math::Vector3(10.f + i, 20.f + i, 30.f + i));
		i++;
	});
}

TEST_F(TestEcs, TestMultiComponentView)
{
	apex::ecs::Registry registry { .minPoolSize = 16 };

	int i;
	for (i = 0; i < 20; i++)
	{
		apex::ecs::Entity entity = registry.createEntity();
		auto& transform = registry.add<apex::Transform>(entity);
		transform.position = { 10.f + i, 20.f + i, 30.f + i };

		if (i % 2 == 0)
		{
			auto& damageInflictor = registry.add<apex::DamageInflictor>(entity);
			damageInflictor.damage = 10.f * i;
		}
	}

	auto view = registry.view<apex::Transform, apex::DamageInflictor>();

	i = 0;
	view.each([&registry, &i](apex::ecs::Entity entity)
	{
		EXPECT_EQ(entity, static_cast<apex::uint32>(i));

		printf("entity: %u\n", static_cast<apex::uint32>(entity));
		apex::Transform& transform = registry.get<apex::Transform>(entity).value();
		printf("%f %f %f\n", transform.position.x, transform.position.y, transform.position.z);
		apex::DamageInflictor& damageInflictor = registry.get<apex::DamageInflictor>(entity).value();
		printf("%f\n", damageInflictor.damage);

		EXPECT_TRUE(transform.position == apex::math::Vector3(10.f + i, 20.f + i, 30.f + i));
		EXPECT_FLOAT_EQ(damageInflictor.damage, 10.f * i);

		i += 2;
	});

	EXPECT_EQ(i, 20);

	i = 0;
	view.each([&i](apex::Transform& transform, apex::DamageInflictor& damage_inflictor)
	{
		printf("%f %f %f\n", transform.position.x, transform.position.y, transform.position.z);
		printf("%f\n", damage_inflictor.damage);

		EXPECT_TRUE(transform.position == apex::math::Vector3(10.f + i, 20.f + i, 30.f + i));
		EXPECT_FLOAT_EQ(damage_inflictor.damage, 10.f * i);

		i += 2;
	});

	EXPECT_EQ(i, 20);

	i = 0;
	view.each([&i](auto& transform, auto& damage_inflictor)
	{
		printf("%f %f %f\n", transform.position.x, transform.position.y, transform.position.z);
		printf("%f\n", damage_inflictor.damage);

		EXPECT_TRUE(transform.position == apex::math::Vector3(10.f + i, 20.f + i, 30.f + i));
		EXPECT_FLOAT_EQ(damage_inflictor.damage, 10.f * i);

		i += 2;
	});

	EXPECT_EQ(i, 20);
}

