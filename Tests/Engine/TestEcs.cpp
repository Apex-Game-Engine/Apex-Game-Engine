#include <queue>
#include <gtest/gtest.h>

#include "Apex/ECS/Registry.h"
#include "Containers/AxList.h"
#include "Core/Delegate.h"
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
		apex::f32 damage;
	};

	struct SomeTag {};

	struct CollisionEvent
	{
		apex::ecs::Entity self;
		apex::ecs::Entity other;
	};
}

class TestEcs : public testing::Test
{
public:
	static void SetUpTestSuite()
	{
		apex::mem::MemoryManager::initialize({ 0, 0 });
	}

	static void TearDownTestSuite()
	{
		apex::mem::MemoryManager::shutdown();
	}

	void SetUp() override
	{
		auto allocated = apex::mem::MemoryManager::getAllocatedSize();
		printf("<PRE> Memory allocated: %llu\n", allocated);
	}

	void TearDown() override
	{
		auto allocated = apex::mem::MemoryManager::getAllocatedSize();
		printf("<POST> Memory allocated: %llu\n", allocated);
	}
};

TEST_F(TestEcs, TestTypeIndex)
{
	auto id = apex::core::TypeIndex<apex::u32>::value();
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
		EXPECT_EQ(entity, static_cast<apex::u32>(i));

		printf("entity: %u\n", static_cast<apex::u32>(entity));
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
		EXPECT_EQ(entity, static_cast<apex::u32>(i));

		printf("entity: %u\n", static_cast<apex::u32>(entity));
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


TEST_F(TestEcs, TestEmptyComponentView)
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

		if (i % 3 == 0)
		{
			registry.add<apex::SomeTag>(entity);
		}
	}


	i = 0;
	auto view = registry.view<apex::Transform, apex::SomeTag>();
	view.each([&i](apex::ecs::Entity entity, auto& transform)
	{
		printf("entity: %u\n", static_cast<apex::u32>(entity));

		EXPECT_EQ(static_cast<uint32_t>(entity), i);

		i += 3;
	});
}

struct DamageMessage
{
	apex::ecs::Entity target;
	apex::f32 value;
};

namespace ecs {
	template <typename Event>
	struct EventListener
	{
		using event_type = Event;

		apex::Delegate<bool(Event const&)> eventHandler;
	};

	template <typename, typename = void, typename = void, typename = void, typename = void>
	struct System;

	template <typename... T>
	struct get_t {};

	template <typename... T>
	struct exclude_t {};

	template <typename... T>
	struct signal_t {};

	template <typename... T>
	struct listen_t {};

	struct NullEventHandler
	{
		template <typename Event>
		bool onEvent(Event const& event) { return false; }
	};

	template <typename EventHandler, typename... Components, typename... Events>
	struct System<EventHandler, get_t<Components...>, exclude_t<>, signal_t<>, listen_t<Events...>>
	{
		using component_view = decltype(std::declval<apex::ecs::Registry>().view<Components...>());
		using event_view = decltype(std::declval<apex::ecs::Registry>().view<Events...>());

		apex::ecs::Registry& m_registry;
		apex::ecs::Entity m_systemEntity;
		component_view m_componentView;

		System(apex::ecs::Registry& registry);

		template <typename Event>
		bool onEvent(Event const& event) { return false; }
	};

	template <typename EventHandler, typename ... Components, typename ... Events>
	System<EventHandler, get_t<Components...>, exclude_t<>, signal_t<>, listen_t<Events...>>::System(apex::ecs::Registry& registry)
		: m_registry(registry)
		, m_systemEntity(registry.createEntity())
		, m_componentView(registry.view<Components...>())
	{
		([&]() {
			auto& eventListener = m_registry.add<EventListener<Events>>(m_systemEntity);
			eventListener.eventHandler.set<&EventHandler::template onEvent<Events>>(static_cast<EventHandler&>(*this));
		}(), ...);
	}

	template <typename... Components>
	struct System<get_t<Components...>>	: public System<NullEventHandler, get_t<Components...>, exclude_t<>, signal_t<>, listen_t<>> {};

	template <typename EventHandler, typename... Components, typename... Events>
	struct System<EventHandler, get_t<Components...>, listen_t<Events...>> : public System<EventHandler, get_t<Components...>, exclude_t<>, signal_t<>, listen_t<Events...>> {};

}


struct SineWaveMovement
{
	apex::f32 amplitude;
	apex::f32 frequency;
};

struct Time
{
	apex::f32 time;
	apex::f32 deltaTime;
};

using S = ecs::System<ecs::NullEventHandler, ecs::get_t<apex::Transform, SineWaveMovement>, ecs::exclude_t<>, ecs::signal_t<>, ecs::listen_t<>>;
using V = S::component_view;
V v;
V::base_storage_type* s;

struct SineWaveMovementSystem : public ecs::System<ecs::get_t<apex::Transform, SineWaveMovement>>
{
	SineWaveMovementSystem(apex::ecs::Registry& registry) : System(registry) {}

	void run(Time time)
	{
		m_componentView.each([&] (auto& transform, const auto& movement) { applyMovement(transform, movement, time); });
	}

	void applyMovement(apex::Transform& transform, SineWaveMovement const& movement, Time const& time)
	{
		transform.position.y = movement.amplitude * sinf(movement.frequency * time.time);
	}
};

struct PositionTracker
{
	apex::ecs::Entity target;
	apex::math::Vector3 relativePosition;
};

struct PositionTrackerSystem : public ecs::System<ecs::get_t<apex::Transform, PositionTracker>>
{
	//using system_t = ecs::System<ecs::get_t<apex::Transform, TrackingTarget>, ecs::exclude_t<>, ecs::signal_t<>, ecs::listen_t<>>;

	PositionTrackerSystem(apex::ecs::Registry& registry) : System(registry) {}

	void run(Time time)
	{
		m_componentView.each([&](apex::Transform& transform, PositionTracker& tracker)
		{
			const auto& targetTransform = m_registry.get<apex::Transform>(tracker.target).value().get();
			tracker.relativePosition = transform.position - targetTransform.position;
		});
	}
};

TEST_F(TestEcs, TestSystemsInteroperability)
{
	apex::ecs::Registry registry;

	auto player = registry.createEntity();
	auto enemy = registry.createEntity();

	auto& playerTransform = registry.add<apex::Transform>(player);
	playerTransform.position = { 0, 0, 0 };

	auto& sineWaveMovement = registry.add<SineWaveMovement>(player);
	sineWaveMovement.amplitude = 10.f;
	sineWaveMovement.frequency = 1.f;

	auto& enemyTransform = registry.add<apex::Transform>(enemy);
	enemyTransform.position = { 10, 0, 0 };

	auto& enemyTracker = registry.add<PositionTracker>(enemy);
	enemyTracker.target = player;

	SineWaveMovementSystem sineWaveMovementSystem(registry);
	PositionTrackerSystem positionTrackerSystem(registry);

	for (int i = 1; i <= 10; i++)
	{
		Time time { .time = 0.016f * i, .deltaTime = 0.016f };

		auto playerTransformCopy = playerTransform;

		sineWaveMovementSystem.run(time);
		positionTrackerSystem.run(time);

		sineWaveMovementSystem.applyMovement(playerTransformCopy, sineWaveMovement, time);
		EXPECT_FLOAT_EQ(playerTransform.position.y, playerTransformCopy.position.y);

		axInfoFmt("Player position: {} {} {}\n", playerTransform.position.x, playerTransform.position.y, playerTransform.position.z);
		axInfoFmt("Enemy relative position: {} {} {}\n", enemyTracker.relativePosition.x, enemyTracker.relativePosition.y, enemyTracker.relativePosition.z);
	}
}

// Event system
// 1. get all entities with event listener component
// 2. listen for events
// 3. send events


struct DamageSystem : public ecs::System<DamageSystem, ecs::get_t<apex::DamageInflictor>, ecs::exclude_t<>, ecs::signal_t<>, ecs::listen_t<apex::CollisionEvent>>
{
	//using system_t = ecs::System<ecs::get_t<apex::DamageInflictor>, ecs::exclude_t<>, ecs::signal_t<>, ecs::listen_t<apex::CollisionEvent>>;

	DamageSystem(apex::ecs::Registry& registry) : System(registry) {}

	template <typename Event>
	bool onEvent(Event const& event)
	{
		return false;
	}

	template <>
	bool onEvent<apex::CollisionEvent>(apex::CollisionEvent const& event)
	{
		if (auto damageInflictorOpt = m_registry.get<apex::DamageInflictor>(event.self); damageInflictorOpt.has_value())
		{
			apex::DamageInflictor& damageInflictor = damageInflictorOpt.value();
			damageInflictor.damage = 1e5f;
		}

		return false;
	}
};

template <typename Event>
struct EventQueue
{
	EventQueue(apex::ecs::Registry& registry)
		: listeners(registry.view<ecs::EventListener<Event>>())
	{}

	std::queue<Event> events;

	using view_t = decltype(std::declval<apex::ecs::Registry>().view<ecs::EventListener<Event>>());
	view_t listeners;
};

TEST_F(TestEcs, TestEventHandlerInvoking)
{
	apex::ecs::Registry registry;

	DamageSystem damageSystem(registry);

	auto player = registry.createEntity();
	auto enemy = registry.createEntity();

	auto& playerTransform = registry.add<apex::Transform>(player);
	playerTransform.position = { 0, 0, 0 };

	auto& damageInflictor = registry.add<apex::DamageInflictor>(player);
	damageInflictor.damage = 0.f;

	auto& enemyTransform = registry.add<apex::Transform>(enemy);
	enemyTransform.position = { 10, 0, 0 };

	apex::CollisionEvent collisionEvent { player, enemy };

	auto collisionEventView = registry.view<ecs::EventListener<apex::CollisionEvent>>();
	for (auto entity : collisionEventView.keys())
	{
		auto& eventListener = registry.get<ecs::EventListener<apex::CollisionEvent>>(entity).value().get();
		if (eventListener.eventHandler(collisionEvent))
			break;
	}

	/*collisionEventView.each([&](apex::ecs::Entity entity)
	{
		auto& eventListener = registry.get<ecs::EventListener<apex::CollisionEvent>>(entity).value().get();
		(void)eventListener.eventHandler(collisionEvent);
	});*/

	EXPECT_FLOAT_EQ(damageInflictor.damage, 1e5f);
}


// collision detection and response
// 1. Broad phase collision detection
//     a. compute AABBs
//     b. build BVH
//     c. make collision pairs
// 2. Narrow phase collision detection
//     a. compute collision
// 3. Collision response

//struct SphereCollider { apex::f32 radius; };
struct AABB { apex::math::Vector3 size; };

template <typename T, size_t _Bucket_size>
class BucketList
{
public:
	constexpr BucketList() = default;

	void insert(const T& value)
	{
		if (m_size == m_capacity)
		{
			m_capacity += _Bucket_size;
			m_buckets.emplace_back();
		}
		m_buckets.back()[m_size % _Bucket_size] = value;
	}

	void remove(size_t index)
	{
		apex::AxStaticArray<T, _Bucket_size>& bucket = m_buckets.back();
		bucket[index] = m_size % _Bucket_size;
	}

	void clear()
	{
		m_buckets.clear();
	}

	T& operator[](size_t index)
	{
		axAssertFmt(index < m_size, "Index out of bounds");
		size_t bucketIndex = index / _Bucket_size;
		size_t elementIndex = index % _Bucket_size;
		for (auto& bucket : m_buckets)
		{
			if (bucketIndex == 0)
				return bucket[elementIndex];
			--bucketIndex;
		}
	}

private:
	apex::AxList<apex::AxStaticArray<T, _Bucket_size>> m_buckets;
	size_t m_size{};
	size_t m_capacity{};
};

struct BoundingVolumeHierarchy
{
	struct Node
	{
		apex::ecs::Entity entity;
		apex::u32 left;
		apex::u32 right;
	};

	apex::AxList<Node> nodes;
};



struct CollisionDetectionBroadPhase : public ecs::System<ecs::get_t<apex::Transform, AABB>>
{
	CollisionDetectionBroadPhase(apex::ecs::Registry& registry) : System(registry) {}

	void run()
	{
		m_componentView.each([&](apex::Transform& transform, AABB& aabb)
		{
			
		});
	}
};
