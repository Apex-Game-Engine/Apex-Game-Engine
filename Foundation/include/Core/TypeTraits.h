#pragma once

namespace apex {

	// `type_list` and `type_list_index` taken from https://github.com/skypjack/entt/blob/master/src/entt/core/type_traits.hpp

	template <typename... Types>
	struct type_list
	{
		using type = type_list;

		static constexpr size_t size = sizeof...(Types);
	};

	template <typename, typename>
	struct type_list_index;

	template <typename Type, typename First, typename... Other>
	struct type_list_index<Type, type_list<First, Other...>>
	{
		using value_type = size_t;

		static constexpr value_type value = 1 + type_list_index<Type, type_list<Other...>>::value;
	};

	template <typename Type, typename... Other>
	struct type_list_index<Type, type_list<Type, Other...>>
	{
		static_assert(type_list_index<Type, type_list<Other...>>::value == sizeof...(Other), "Non-unique type");

		using value_type = size_t;

		static constexpr value_type value = 0;
	};

	template <typename Type>
	struct type_list_index<Type, type_list<>>
	{
		using value_type = size_t;

		static constexpr value_type value = 0;
	};

	template <typename Type, typename List>
	inline constexpr size_t type_list_index_v = type_list_index<Type, List>::value;

}
