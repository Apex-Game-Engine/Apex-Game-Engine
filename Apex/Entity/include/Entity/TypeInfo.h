#pragma once

#include "String/AxHashString.h"
#include "String/AxStaticString.h"


namespace apex {
namespace ecs {

	//namespace internal
	//{
	//	struct TypeIndex final
	//	{
	//		static u32 next()
	//		{
	//			static u32 value{};
	//			return value++;
	//		}
	//	};
	//}

	//template <typename Type, typename = void>
	//struct TypeIndex final
	//{
	//	static constexpr u32 value() noexcept
	//	{
	//		static const u32 value = internal::TypeIndex::next();
	//		return value;
	//	}
	//};

	namespace internal {
		template <typename T>
	    constexpr auto TypeName() -> AxStringView
	    {
		#if defined(__GNUC__)
	        size_t prefixSize = sizeof("constexpr apex::AxStringView internal::TypeName() [with T = ") - 1;
	        auto name = apex::AxStringView{__PRETTY_FUNCTION__, sizeof(__PRETTY_FUNCTION__) - 1};
	        if constexpr (sizeof(__PRETTY_FUNCTION__) == sizeof(__FUNCTION__))
	        {
		        return AxStringView{};
	        }
	        else if (name.m_str[name.m_size - 1] == ']')
	        {
		        name.m_size -= prefixSize + 1;
	            name.m_str += prefixSize;
	        }
	        else
	        {
		        [flag = false]() { static_assert(flag, "Not Implemented!"); };
	        }
		#elif defined(_MSC_VER)
			constexpr size_t totalSize = sizeof(__FUNCSIG__);
	        constexpr size_t prefixSize = sizeof("class std::basic_string_view<char,struct std::char_traits<char> > __cdecl apex::ecs::internal::TypeName<") - 1;
	        constexpr size_t suffixSize = sizeof(">(void)") - 1;

	        auto name = AxStringView{ __FUNCSIG__, totalSize -1};
			name.remove_prefix(prefixSize);
			name.remove_suffix(suffixSize);


	        size_t typePrefixSize = 0;

    		switch (name[0])
    		{
    		case 'c': // "class"
	            typePrefixSize = sizeof("class");
	            break;
    		case 'u': // "union"
	            typePrefixSize = sizeof("union");
	            break;
    		case 'e': // "enum"
	            typePrefixSize = sizeof("enum");
	            break;
    		case 's': // "struct"
	            typePrefixSize = sizeof("struct");
	            break;
    		default:
    			// []<bool flag = false>() { static_assert(flag, "Incompatible typename!"); }();
				return {};
    		}

	        typePrefixSize = (name[typePrefixSize] != ' ') ? typePrefixSize : 0;
	        name.remove_prefix(typePrefixSize);
		#endif
	        return name;
	    }
	}

	template <typename T>
	struct TypeInfo
	{
		static auto name()
		{
			constexpr AxStringView cname = internal::TypeName<T>();
			static auto name = AxStaticString<cname.size()>{cname.data()};
			return AxStringView{name.data(), name.size()};
		}

		static constexpr u64 hash()
		{
			return FnvHasher64::Hash(internal::TypeName<T>());
		}
	};

	template <typename T>
	auto typeof() -> TypeInfo<std::remove_cvref_t<T>> { return {}; }

	template <typename T>
	auto typeof(T&& /* unused */) -> TypeInfo<std::remove_cvref_t<T>> { return {}; }

}
}

#define APEX_REGISTER_TYPE(TYPE) \
	static const apex::u32 g_TypeIndex_##TYPE = apex::core::TypeIndex<TYPE>::value()
