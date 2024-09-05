#include <gtest/gtest.h>

#include "StringHash.h"
#include "Core/Types.h"
#include "Memory/MemoryManager.h"
#include "Memory/SharedPtr.h"

namespace apex {
namespace resources {

#if defined(APEX_CONFIG_DEBUG)
#define APEX_INCLUDE_RESOURCE_NAME_STRING 1
#endif

#ifndef APEX_INCLUDE_RESOURCE_NAME_STRING
#define APEX_INCLUDE_RESOURCE_NAME_STRING 0
#endif

	struct Identifier
	{
		constexpr Identifier(const char* name)
			: hash(internal::hash_fnv1a_32(name))
#if APEX_INCLUDE_RESOURCE_NAME_STRING
			, name(name)
#endif
		{}

		uint32 hash;
#if APEX_INCLUDE_RESOURCE_NAME_STRING
		const char* name;
#endif
	};

	template <typename T>
	class ResourceCache
	{
	public:
		using resource_ptr = decltype(apex::make_shared<T>());

		void addResource(Identifier identifier, resource_ptr resource)
		{
			m_resources[identifier.hash] = resource;
		}

		resource_ptr getResource(Identifier identifier)
		{
			return m_resources[identifier.hash];
		}

	private:
		std::unordered_map<uint32, resource_ptr> m_resources;
	};

}	
}

TEST(TestResourceManagement, TestResourceManagement)
{
	using namespace apex::resources;

	apex::memory::MemoryManager::initialize({ 0, 0 });

	{
		ResourceCache<int> cache;

		cache.addResource("test", apex::make_shared<int>(42));
		EXPECT_EQ(*cache.getResource("test"), 42);

		*cache.getResource("test") += 4;
		EXPECT_EQ(*cache.getResource("test"), 43);
	}

	apex::memory::MemoryManager::shutdown();

}

