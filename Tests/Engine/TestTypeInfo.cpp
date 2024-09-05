#include <gtest/gtest.h>

#include "StringHash.h"
#include "Apex/TypeInfo.h"
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
    enum Things
    {
	    eNothing = 0
    };

}

constexpr auto hash64(const char* str)
{
    return apex::internal::hash_fnv1a_64(str);
}

template <typename T>
constexpr auto type_name()
{
    constexpr auto name = apex::core::internal::TypeName<T>();
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


TEST(TestTypeInfo, TestTypeInfoName)
{
    using apex::math::Vector3;
    Vector3 vec;
    auto typeName = apex::core::typeof(vec).name();

    fmt::print("Type name: {}\n", typeName.c_str());

}
