#pragma once

#include "Entity.h"
#include "Invoke.h"
#include "Core/TypeTraits.h"

namespace apex {
namespace ecs {

	template <typename... Gets>
	struct get_t {};

	template <typename... Excludes>
	struct exclude_t {};

	template <typename, typename>
	struct View;

	template <>
	struct View<get_t<>, exclude_t<>>
	{
		using types = type_list<>;

		View(std::tuple<>&&) {}

		template <typename Func>
		void each(Func&& func) {}
	};

	template <typename Component>
	struct View<get_t<Component>, exclude_t<>>
	{
		using types = type_list<Component>;
		using storage_type = storage_for_type_t<Component>;
		using base_storage_type = typename storage_type::base_type;

		View(std::tuple<storage_type*>&& pool)
		: m_pool(std::forward<storage_type*>(std::get<0>(pool)))
		{
		}

		template <typename Func>
		void each(Func&& func)
		{
			for (auto entity : m_pool->keys())
			{
				apex::ecs::invoke<Func, Component>(std::forward<Func>(func), entity, m_pool);
			}
		}

		bool contains(Entity entity) const
		{
			return m_pool->contains(entity);
		}

		auto count() const
		{
			return m_pool->count();
		}

		auto keys() const
		{
			return m_pool->keys();
		}

	private:
		storage_type* m_pool;
	};

	template <typename... Components>
	struct View<get_t<Components...>, exclude_t<>>
	{
		using types = type_list<Components...>;
		using storage_types = type_list<storage_for_type_t<Components>...>;
		using base_storage_type = const std::common_type_t<typename storage_for_type_t<Components>::base_type...>;

		constexpr View() = default;

		View(std::tuple<storage_for_type_t<Components>*...>&& pools)
		: m_pools(std::forward<std::tuple<storage_for_type_t<Components>*...>>(pools))
		, m_view{}
		{
			selectSmallest();
		}

		template <typename Func>
		void each(Func&& func)
		{
			for (auto entity : m_view->keys())
			{
				bool isEntityInView = std::apply([entity](const auto*... curr)
				{
					return ((curr->contains(entity) && ...));
				}, m_pools);

				if (isEntityInView)
				{
					apex::ecs::invoke<Func, Components...>(std::forward<Func>(func), entity, m_pools);
				}
			}
		}

		bool contains(Entity entity) const
		{
			return m_view && std::apply([entity](auto const*... curr) { return (curr->contains(entity) && ...); }, m_pools);
		}

		template <size_t Index>
		void select()
		{
			m_view = std::get<Index>(m_pools);
		}

		template <typename Component>
		void select()
		{
			select<type_list_index_v<Component, types>>();
		}

		void selectSmallest()
		{
			m_view = std::get<0>(m_pools);
			std::apply([this](const auto*... curr)
			{
				((m_view = curr->count() < m_view->count() ? curr : m_view), ...);
			}, m_pools);
		}

	private:
		std::tuple<storage_for_type_t<Components>*...> m_pools;
		base_storage_type* m_view;
	};

}
}
