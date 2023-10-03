#pragma once

#include "Api.h"
#include "Containers/AxStringRef.h"
#include "Core/Types.h"

namespace apex {
namespace ecs {

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

	template <typename T>
	struct TypeInfo
	{
		static AxStringRef className();
	};

}
}
