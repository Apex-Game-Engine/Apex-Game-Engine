#pragma once

#include <Api.h>
#include <Containers/AxStaticString.h>
#include <Containers/AxStringRef.h>
#include <Core/Types.h>

namespace apex {
namespace core {

	namespace internal
	{
		struct APEX_API TypeIndex final
		{
			static uint32 next()
			{
				static uint32 value{};
				return value++;
			}
		};
	}

	template <typename Type, typename = void>
	struct TypeIndex final
	{
		static constexpr uint32 value() noexcept
		{
			static const uint32 value = internal::TypeIndex::next();
			return value;
		}
	};

	namespace internal {
		template <typename T>
	    constexpr auto TypeName()
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
	        size_t prefixSize = sizeof("auto __cdecl apex::core::internal::TypeName<") - 1;
	        size_t suffixSize = sizeof(">(void)") - 1;
	        auto name = apex::AxStringView{ __FUNCSIG__, sizeof(__FUNCSIG__) -1};
	        name.m_size -= prefixSize + suffixSize;
	        name.m_str += prefixSize;

	        size_t typePrefixSize = 0;

    		switch (name.m_str[0])
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
    		}

	        typePrefixSize = (name.m_str[typePrefixSize] != ' ') ? typePrefixSize : 0;
	        name.m_size -= typePrefixSize;
		    name.m_str += typePrefixSize;
		#endif
	        return name;
	    }
	}

	template <typename T>
	struct TypeInfo
	{
		static auto name()
		{
			constexpr auto cname = internal::TypeName<T>();
			static auto name = AxStaticString<cname.m_size>{cname.m_str};
			return AxStringRef{name.data(), name.size()};
		}
	};

	template <typename T>
	auto typeof(T&& /* unused */) -> TypeInfo<std::remove_cvref_t<T>>
	{
		return {};
	}

}
}

#define APEX_REGISTER_TYPE(TYPE) \
	static constexpr apex::uint32 g_TypeIndex_##TYPE = apex::core::TypeIndex<TYPE>::value()
