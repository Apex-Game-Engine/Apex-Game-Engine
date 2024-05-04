#pragma once

namespace apex {

	// `type_list` and `type_list_index` taken from https://github.com/skypjack/entt/blob/master/src/entt/core/type_traits.hpp

	template <typename... Types>
	struct type_list
	{
		using type = type_list;
		struct type_list_tag {};

		static constexpr size_t size = sizeof...(Types);
	};

	template <typename T>
	static constexpr bool is_type_list_v = requires { typename T::type_list_tag; };

	// type_list_index
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

	// type_list_cat
	template <typename, typename>
	struct type_list_cat;

	template <typename... Types1, typename... Types2>
	struct type_list_cat<type_list<Types1...>, type_list<Types2...>>
	{
	    using type = type_list<Types1..., Types2...>;
	};

	template <typename TypeList1, typename TypeList2>
	using type_list_cat_t = typename type_list_cat<TypeList1, TypeList2>::type;

	// non_empty_type_list
	template <typename>
	struct non_empty_type_list;

	template <typename Type>
	struct non_empty_type_list<type_list<Type>>
	{
	    using type = std::conditional_t<std::is_empty_v<Type>, type_list<>, type_list<Type>>;
	};

	template <typename Type, typename... Other>
	struct non_empty_type_list<type_list<Type, Other...>>
	{
	    using non_empty_condition_type_list = typename type_list_cat<type_list<Type>, typename non_empty_type_list<type_list<Other...>>::type>::type;
	    using empty_condition_type_list = typename non_empty_type_list<type_list<Other...>>::type;

	    using type = std::conditional_t<std::is_empty_v<Type>, empty_condition_type_list, non_empty_condition_type_list>;
	};

	template <typename TypeList>
	using non_empty_type_list_t = typename non_empty_type_list<TypeList>::type;

	// Compile-time assertions
	template <size_t a, size_t b>
	struct TAssertEquality
	{
		static_assert(a == b, "Not equal!");
		static constexpr bool value() { return a == b; }
	};

	template <typename T, typename U>
	struct TAssertEqualSize
	{
		static constexpr bool value() { return TAssertEquality<sizeof(T), sizeof(U)>::value(); }
	};

	template <size_t s> struct TSizeCheck;

}
