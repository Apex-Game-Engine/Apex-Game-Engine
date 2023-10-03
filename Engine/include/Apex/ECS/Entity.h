#pragma once

#include <Core/Types.h>
#include <Containers/AxSparseMap.h>

namespace apex {
namespace ecs {

	using entity_id = uint32;
	using component_id = uint32;

	constexpr entity_id kMaxEntityId = constants::uint32_MAX - 1;

	template <typename EntityType>
	struct EntityMasks;

	union Entity
	{
		entity_id uint { kMaxEntityId };

		struct Masks
		{
			uint32 identifier : 20;
			uint32 version : 12;
		} masks;

		static_assert(sizeof(masks) == sizeof(uint));

		Entity() = default;
		constexpr Entity(entity_id value) : uint(value) {}

		constexpr operator entity_id() const { return uint; }
	};
	constexpr Entity null_entity = constants::uint32_MAX;

	
	template <typename Component>
	struct storage_for_type
	{
		using type = AxSparseMap<entity_id, Component>;
	};
	template <typename Component>
	using storage_for_type_t = typename storage_for_type<Component>::type;

}
}
