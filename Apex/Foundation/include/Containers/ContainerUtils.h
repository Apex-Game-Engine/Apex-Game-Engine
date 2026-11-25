#pragma once

namespace apex {
	
	template <typename Type>
	struct AxTypeHelper
	{
		using value_type = Type;
		using reference = Type&;
		using const_reference = const Type&;
		using pointer = Type*;
		using const_pointer = const Type*;
	};

}