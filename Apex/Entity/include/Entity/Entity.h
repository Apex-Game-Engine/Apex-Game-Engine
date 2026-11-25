#pragma once

#include "TypeInfo.h"
#include "Containers/AxHashMap.h"
#include "Containers/AxSparseMap.h"
#include "Memory/UniquePtr.h"
#include "String/AxHashString.h"

namespace apex {
namespace ecs {

	template <typename... Gets> struct get_t;

	template <typename... Excludes> struct exclude_t;

	template <typename, typename> struct View;


	template <typename T>
	struct Id
	{
		using value_type = T;

		constexpr static value_type Invalid()	{ return std::numeric_limits<value_type>::max(); }
		constexpr static value_type Max()		{ return Invalid() - 1; }

		value_type value;

		constexpr Id() : value(Invalid()) {}
		constexpr Id(value_type id) : value(id) {}

		constexpr bool IsValid() const { return value != Invalid(); }

		constexpr operator value_type() const { return value; }
		constexpr operator bool() const { return IsValid(); }
	};

	struct NullEntityTag {};
	constexpr NullEntityTag null;

	using Id32 = Id<u32>;
	using Id64 = Id<u64>;

	union EntityId
	{
		using id_type = Id32;
		using value_type = id_type::value_type;

		Id32 id {};

		struct Masks
		{
			u32 identifier : 20;
			u32 generation : 12;
		} masks;

		static_assert(sizeof(masks) == sizeof(id));

		constexpr EntityId() = default;
		constexpr EntityId(NullEntityTag) {}
		constexpr EntityId(Id32 value) : id(value) {}

		constexpr operator id_type() const { return id; }
		constexpr operator value_type() const { return id; }
	};

	using ComponentId = Id64;

	template <typename T> struct storage_for_type
	{
		using type = AxSparseMap<EntityId::value_type, T>;
	};
	template <typename T> using storage_for_type_t = typename storage_for_type<T>::type;

	class Registry
	{
	private:
		using base_pool_type = AxSparseSet<EntityId::value_type>;
		template <typename Component> using pool_type = AxSparseMap<EntityId::value_type, Component>;
		template <typename Component> using component_reference_type = typename pool_type<Component>::value_reference;

		static constexpr u32 kDefaultComponentPoolSize = 8u;
		static constexpr u32 kComponentPoolGrowthRate = 2u;

		EntityId::value_type m_entityCount { 0 };

		AxDenseHashMap<u64, UniquePtr<base_pool_type>> m_componentPoolMap;


	public:
		EntityId CreateEntity() 
		{
			axAssert(m_entityCount < EntityId::id_type::Max());
			return EntityId(m_entityCount++);
		}

		template <typename Component>
		void RegisterComponent()
		{
			RegisterComponent<Component>(typeof<Component>().hash());
		}

		template <typename Component>
		auto GetComponentPool() -> pool_type<Component>*
		{
			return GetComponentPool<Component>({ typeof<Component>().hash() });
		}

		template <typename Component>
		auto Add(EntityId entity) -> typename pool_type<Component>::value_reference
		{
			return Add<Component>(entity, { typeof<Component>().hash() });
		}

		template <typename Component>
		auto Get(EntityId entity) -> std::optional<std::reference_wrapper<Component>>
		{
			return Get<Component>(entity, { typeof<Component>().hash() });
		}

		template <typename... Components>
		auto View() -> View<get_t<Components...>, exclude_t<>>
		{
			return { GetComponentPools<Components...>() };
		}

	protected:
		template <typename Component>
		void RegisterComponent(ComponentId component_id)
		{
			auto [it, success] = m_componentPoolMap.emplace(component_id, nullptr);
			if (axVerifyFmt(success, "Attempting to re-register component (id: {:x} | type: {})", component_id.value, typeof<Component>().name()))
			{
				auto pPool = apex_new pool_type<Component>(kDefaultComponentPoolSize);
				it->second.reset(pPool);
			}
		}

		template <typename Component>
		auto GetComponentPool(ComponentId component_id) -> pool_type<Component>*
		{
			const auto it = m_componentPoolMap.find(component_id.value);
			if (axVerifyFmt(it != m_componentPoolMap.end(), "Component (id: {:x} | type: {}) is not registered", component_id.value, typeof<Component>().name()))
			{
				return static_cast<pool_type<Component>*>(it->second.get());
			}
			return nullptr;
		}

		template <typename Component>
		auto Add(EntityId entity, ComponentId component_id) -> component_reference_type<Component>
		{
			auto pool = GetComponentPool<Component>();
			axCriticalAssertFmt(pool, "Component pool is null! Is component (id: {:x} | type: {}) registered?", component_id.value, typeof<Component>().name());
			if (pool->capacity() <= entity)
			{
				size_t newCapacity = (entity.id.value + 1) * kComponentPoolGrowthRate;
				pool->resize(newCapacity);
				axDebugFmt("Pool of component (id: {:x} | type: {}) resized. New size: {}", component_id.value, typeof<Component>().name(), pool->capacity());
			}
			return pool->emplace(entity);
		}

		template <typename Component>
		auto Get(EntityId entity, ComponentId component_id) -> std::optional<std::reference_wrapper<Component>>
		{
			auto pool = GetComponentPool<Component>();
			axCriticalAssertFmt(pool, "Component pool is null! Is component (id: {:x} | type: {}) registered?", component_id.value, typeof<Component>().name());
			if (pool->capacity() <= entity)
			{
				return std::nullopt;
			}
			return pool->getElement(entity);
		}

		template <typename... Components>
		auto GetComponentPools() -> std::tuple<pool_type<Components>*...>
		{
			return std::make_tuple(GetComponentPool<Components>()...);
		}
	};

}	
}
