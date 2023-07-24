#pragma once
#include <memory>

#include "Core/Types.h"

#ifndef APEX_ENABLE_TESTS
void* operator new(size_t size);
void* operator new[](size_t size);

void operator delete(void* mem);
void operator delete[](void* mem);
#endif

namespace apex {
	struct AxHandle;

	namespace memory {
		class MemoryManager;
	}

	struct AxManagedClass
	{
		void* operator new(size_t);
		void* operator new[](size_t);

		void operator delete(void*);
		void operator delete[](void*);

		void* operator new(size_t size, AxHandle& handle);
		void* operator new[](size_t size, AxHandle& handle);

		void operator delete(void *ptr, AxHandle& handle);
		void operator delete[](void *ptr, AxHandle& handle);
	};

	template <typename T>
	concept is_managed_class = requires
	{
		std::is_base_of_v<AxManagedClass, T>;
	};


_EXPORT_STD template <class _Ty, class... _Types, std::enable_if_t<!std::is_array_v<_Ty>, int> = 0> requires apex::is_managed_class<_Ty>
_NODISCARD_SMART_PTR_ALLOC _CONSTEXPR23 std::unique_ptr<_Ty> make_unique(apex::AxHandle& handle, _Types&&... _Args) { // make a unique_ptr
    return std::unique_ptr<_Ty>(new (handle) _Ty(_STD forward<_Types>(_Args)...));
}

_EXPORT_STD template <class _Ty, std::enable_if_t<std::is_base_of_v<apex::AxManagedClass, _Ty>, int> = 0, std::enable_if_t<std::is_array_v<_Ty> && std::extent_v<_Ty> == 0, int> = 0> requires apex::is_managed_class<_Ty>
_NODISCARD_SMART_PTR_ALLOC _CONSTEXPR23 std::unique_ptr<_Ty> make_unique(apex::AxHandle& handle, const size_t _Size) { // make a unique_ptr
    using _Elem = std::remove_extent_t<_Ty>;
    return std::unique_ptr<_Ty>(new (handle) _Elem[_Size]());
}

}
