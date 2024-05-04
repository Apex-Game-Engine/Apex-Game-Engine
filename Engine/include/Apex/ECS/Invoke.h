#pragma once
#include "Entity.h"

namespace apex {
namespace ecs {

    template <typename, typename, typename>
    struct is_invocable;

	template <typename Func, typename... Types>
    struct is_invocable<Func, type_list<Types...>&, void>
	{
        static constexpr bool value = std::is_invocable_v<Func, Types&...>;
    };

	template <typename Func, typename... Types>
	struct is_invocable<Func, const type_list<Types...>&, void>
	{
		static constexpr bool value = std::is_invocable_v<Func, const Types&...>;
	};

	template <typename Func, typename... Types>
	struct is_invocable<Func, Entity, type_list<Types...>&>
    {
		static constexpr bool value = std::is_invocable_v<Func, Entity, Types&...>;
	};

	template <typename Func, typename... Types>
    struct is_invocable<Func, Entity, const type_list<Types...>&>
    {
        static constexpr bool value = std::is_invocable_v<Func, Entity, const Types&...>;
    };

    template <typename Func, typename Args0, typename Args1 = void>
    inline constexpr bool is_invocable_v = is_invocable<Func, Args0, Args1>::value;

    template <typename... SelectedComponents, typename... Storages>
    constexpr std::tuple<SelectedComponents&...> get_components(type_list<SelectedComponents...>, std::tuple<Storages...> pools, Entity entity)
	{
		return std::tie(std::get<storage_for_type_t<SelectedComponents>*>(pools)->getElement(entity)...);
	}

	template <typename Func, typename... Components>
	auto invoke(Func&& func, Entity entity, std::tuple<storage_for_type_t<Components>*...> pools)
	{
        using components_t = non_empty_type_list_t<type_list<Components...>>;

		if constexpr (std::is_invocable_v<Func, Entity>)
        {
            func(entity);
        }
        else if constexpr (is_invocable_v<Func, Entity, components_t&>)
        {
            // Extract components from pools and pass them to func
            auto components = get_components(components_t{}, pools, entity);
            std::apply([&func, entity](auto&... comps) { func(entity, comps...); }, components);
        }
        else if constexpr (is_invocable_v<Func, Entity, const components_t&>)
        {
            // Extract const components from pools and pass them to func
            auto components = get_components(components_t{}, pools, entity);
            std::apply([&func, entity](const auto&... comps) { func(entity, comps...); }, components);
        }
        else if constexpr (is_invocable_v<Func, components_t&>)
        {
            // Extract components from pools and pass them to func
            auto components = get_components(components_t{}, pools, entity);
            std::apply([&func](auto&... comps) { func(comps...); }, components);
        }
        else if constexpr (is_invocable_v<Func, const components_t&>)
        {
            // Extract const components from pools and pass them to func
            auto components = get_components(components_t{}, pools, entity);
            std::apply([&func](const auto&... comps) { func(comps...); }, components);
        }
        else
        {
	        []<bool flag = false>() { static_assert(flag, "Unsupported function signature for apex::ecs::invoke"); }();
        }
	}

}
}
