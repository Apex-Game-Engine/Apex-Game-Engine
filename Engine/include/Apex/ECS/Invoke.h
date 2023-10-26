#pragma once
#include "Entity.h"

namespace apex {
namespace ecs {

	template <typename Func, typename... Components>
	auto invoke(Func&& func, Entity entity, std::tuple<storage_for_type_t<Components>*...> pools)
	{
		if constexpr (std::is_invocable_v<Func, Entity>)
        {
            func(entity);
        }
        else if constexpr (std::is_invocable_v<Func, Entity, Components&...>)
        {
            // Extract components from pools and pass them to func
            auto components = std::make_tuple(std::get<storage_for_type_t<Components>*>(pools)->getElement(entity)...);
            std::apply([&func, entity](auto... comps) { func(entity, comps...); }, components);
        }
        else if constexpr (std::is_invocable_v<Func, Entity, const Components&...>)
        {
            // Extract const components from pools and pass them to func
            auto components = std::make_tuple(std::get<storage_for_type_t<Components>*>(pools)->getElement(entity)...);
            std::apply([&func, entity](const auto&... comps) { func(entity, comps...); }, components);
        }
        else if constexpr (std::is_invocable_v<Func, Components&...>)
        {
            // Extract components from pools and pass them to func
            auto components = std::make_tuple(std::get<storage_for_type_t<Components>*>(pools)->getElement(entity)...);
            std::apply([&func](auto... comps) { func(comps...); }, components);
        }
        else if constexpr (std::is_invocable_v<Func, const Components&...>)
        {
            // Extract const components from pools and pass them to func
            auto components = std::make_tuple(std::get<storage_for_type_t<Components>*>(pools)->getElement(entity)...);
            std::apply([&func](const auto&... comps) { func(comps...); }, components);
        }
        else
        {
	        []<bool flag = false>() { static_assert(flag, "Unsupported function signature for apex::ecs::invoke"); }();
        }
	}

}
}
