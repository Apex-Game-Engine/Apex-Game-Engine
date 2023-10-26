#pragma once

#include "Entity.h"
#include "TypeInfo.h"
#include "TypeTraits.h"
#include "Containers/AxSparseMap.h"

#include "View.h"

static char STRBUF[512];

namespace apex {
namespace ecs {

	struct Registry
	{
		using base_pool_type = AxSparseSet<entity_id>;
		template <typename Component> using pool_type = storage_for_type_t<Component>;

		static constexpr auto kComponentPoolGrowthRate = 2u;

		uint32 m_entityCount{};
		AxSparseMap<component_id, UniquePtr<base_pool_type>> m_pools;
		size_t minPoolSize = 8;

		Entity createEntity() 
		{
			axStrongAssert(m_entityCount < kMaxEntityId);
			return m_entityCount++;
		}

		template <typename Component>
		auto add(Entity entity) -> std::conditional_t<empty<Component>, void, Component&>
		{
			pool_type<Component>* pool = assurePool<Component>();

			if (pool->capacity() <= entity)
			{
				size_t newCapacity = entity * kComponentPoolGrowthRate;

				sprintf_s(STRBUF, "Component pool [%d] resized. New size: %llu", TypeIndex<Component>::value(), newCapacity);
				axDebug(STRBUF);

				pool->resize(newCapacity);
			}

			if constexpr (empty<Component>)
			{
				return pool->insert(entity);
			}
			else
			{
				return pool->emplace(entity);
			}
		}

		template <typename Component>
		auto get(Entity entity) -> std::optional<std::conditional_t<empty<Component>, Component, std::reference_wrapper<Component>>>
		{
			pool_type<Component>* pool = assurePool<Component>();

			if (pool->capacity() <= entity)
			{
				return std::nullopt;
			}

			if constexpr (empty<Component>)
			{
				return Component{};
			}
			else
			{
				return pool->getElement(entity);
			}
		}

		template <typename Component>
		auto assurePool() -> pool_type<Component>*
		{
			const component_id typeIndex = TypeIndex<Component>::value();

			return _AssurePool<Component>(typeIndex);
		}

		template <typename... Components>
		auto view() -> View<get_t<Components...>, exclude_t<>>
		{
			return { _AssurePools<Components...>() };
		}

	protected:
		// template <typename, typename> friend struct View;

		template <typename Component>
		auto _AssurePool(component_id type_index) -> pool_type<Component>*
		{
			// TODO: Try to count the number of unique component types before hand to avoid resizing registry
			if (m_pools.capacity() <= type_index)
			{
				const size_t newCapacity = max(type_index * 1.5, type_index + 1.0);

				m_pools.resize(newCapacity);

				sprintf_s(STRBUF, "Registry resized. New size: %llu", m_pools.capacity());
				axDebug(STRBUF);
			}

			auto elemPair = m_pools.try_get(type_index);
			if (elemPair)
			{
				return static_cast<pool_type<Component>*>(elemPair.value().second.get());
			}
			else
			{
				AxHandle handle;
				auto pPool = new (handle) pool_type<Component>(minPoolSize);
				auto& pool = m_pools.emplace(type_index, pPool);
				return static_cast<pool_type<Component>*>(pool.get());
			}
		}

		template <typename... Components>
		auto _AssurePools() -> std::tuple<pool_type<Components>*...>
		{
			return std::make_tuple(_AssurePool<Components>(TypeIndex<Components>::value())...);
		}

		auto _GetPool(component_id id) -> base_pool_type*
		{
			return m_pools.getElement(id).get();
		}

		template <typename Component>
		auto _GetPool() -> pool_type<Component>*
		{
			return static_cast<pool_type<Component>*>(_GetPool(TypeIndex<Component>::value()));
		}

		template <typename... Components>
		auto _GetPools() -> std::tuple<pool_type<Components>*...>
		{
			return std::make_tuple(_GetPool<Components>()...);
		}
	};

}	
}
