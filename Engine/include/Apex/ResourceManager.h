#pragma once
#include "Containers/AxArray.h"
#include "Containers/AxList.h"
#include "Containers/AxStringRef.h"
#include "Memory/SharedPtr.h"
#include "Memory/UniquePtr.h"

namespace apex {

	struct BaseResourceMetadata
	{
		AxStringRef name{};
		AxStringRef path{};
	};

	template <typename T>
	struct ResourceMetadata;

	template <typename T>
	class Resource
	{
	public:
		using value_type = T;
		using reference = T&;
		using const_reference = const T&;
		using pointer = T*;
		using const_pointer = const T*;

		bool valid() const { return m_resource != nullptr; }

		reference get()	{ return *m_resource; }
		const_reference get() const { return *m_resource; }

		pointer operator->() { return m_resource.get(); }
		const_pointer operator->() const { return m_resource.get(); }

		reference operator*() { return *m_resource; }
		const_reference operator*() const { return *m_resource; }
	
		ResourceMetadata<T> m_metadata{};
		UniquePtr<T>        m_resource{};
	};

	template <typename T>
	using ResourcePool = AxList<Resource<T>>;

}
