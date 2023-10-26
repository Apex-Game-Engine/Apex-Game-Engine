#include <gtest/gtest.h>

#include "Containers/AxStaticString.h"
#include "Containers/AxStringRef.h"
#include "Core/Asserts.h"
#include "Math/Vector3.h"

namespace apex
{
	namespace ecs
	{
		union Entity;
	}
}

namespace apex::internal
{
    constexpr uint32_t fnv_prime_32 = 16777619;
    constexpr uint64_t fnv_prime_64 = 1099511628211U;

    constexpr uint32_t fnv_offset_basis_32 = 2166136261;
    constexpr uint64_t fnv_offset_basis_64 = 14695981039346656037U;

    inline constexpr uint64_t hash_fnv1a_64(const char* const str, const uint64_t hash = fnv_offset_basis_64)
    {
        // hash = fnv_offset_basis_64
        // hash = hash ^ str[idx]
        // hash = hash * fnv_prime_64
        return (str[0] == '\0') ? hash : hash_fnv1a_64(&str[1], (hash ^ str[0]) * fnv_prime_64);
    }

    enum Things
    {
	    eNothing = 0
    };

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
        size_t prefixSize = sizeof("auto __cdecl apex::internal::TypeName<") - 1;
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

constexpr auto hash64(const char* str)
{
    return apex::internal::hash_fnv1a_64(str);
}

template <typename T>
constexpr auto type_name()
{
    constexpr auto name = apex::internal::TypeName<T>();
    return apex::AxStaticString<name.m_size>{name};
}

template <typename T>
constexpr auto type_hash()
{
	return hash64(type_name<T>().data());
}

template <typename T>
constexpr auto type_hash(T const&)
{
	return type_hash<T>();
}

TEST(TestTypeInfo, TestCompileTimeTypeName)
{
    using apex::math::Vector3;

    apex::math::Vector3 vec;

    auto type = type_hash(vec);
    type = type_hash<apex::ecs::Entity>();

    switch (type)
    {
    case type_hash<int>():
        printf("type is <%s>\n", type_name<int>().data());
        break;
    case type_hash<apex::AxManagedClass>():
		printf("type is <%s>\n", type_name<apex::AxManagedClass>().data());
        break;
    case type_hash<apex::ecs::Entity>():
		printf("type is <%s>\n", type_name<apex::ecs::Entity>().data());
        break;
    case type_hash<Vector3>():
		printf("type is <%s>\n", type_name<Vector3>().data());
        break;
    case type_hash<apex::internal::Things>():
		printf("type is <%s>\n", type_name<apex::internal::Things>().data());
        break;
    }

    printf("strlen of name : %d\n", type_name<apex::ecs::Entity>().size());
}
