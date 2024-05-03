#pragma once
#include <memory>

#include "AxHandle.h"
#include "Core/Types.h"

#ifndef APEX_ENABLE_TESTS
void* operator new(size_t size);
void* operator new[](size_t size);

void operator delete(void* mem) noexcept;
void operator delete[](void* mem) noexcept;
#endif

namespace apex {

	namespace memory {
		class MemoryManagerTest;
		class MemoryManager;
	}

	class AxManagedClass
	{
	public:
		void* operator new(size_t);
		void* operator new[](size_t);

		void operator delete(void*);
		void operator delete[](void*);

		void* operator new(size_t size, void* mem);
		void* operator new[](size_t size, void* mem);

		void operator delete(void*, void*);
		void operator delete[](void*, void*);

		void* operator new(size_t size, AxHandle& handle);
		void* operator new[](size_t size, AxHandle& handle);

		void operator delete(void *ptr, AxHandle& handle);
		void operator delete[](void *ptr, AxHandle& handle);
	};

	struct ExternallyManaged {};
	struct SelfManaged {};

	template <typename T>
	class alignas(alignof(T)) AxManagedClassAdapter : public T, public AxManagedClass
	{
	public:
		template <typename... Args>
		AxManagedClassAdapter(Args&&... args) : T(std::forward<Args>(args)...) {}

		constexpr operator T() { return static_cast<T>(*this); }
	};

	template <typename T> requires (apex::numeric<T>)
	class AxManagedClassAdapter<T> : public AxManagedClass
	{
	public:
		using value_type = T;
		using pointer    = T*;

		constexpr AxManagedClassAdapter() = default;
		constexpr AxManagedClassAdapter(T value) : m_value(value) {}
		constexpr AxManagedClassAdapter& operator=(T other) { m_value = other; return *this; }

		constexpr operator T() const { return m_value; }

		T& value() { return m_value; }
		const T& value() const { return m_value; }

		constexpr bool operator<=>(T const& other) const { return m_value <=> other; }

	private:
		T m_value;
	};

	static_assert(sizeof(AxManagedClassAdapter<bool>) == sizeof(bool));
	static_assert(sizeof(AxManagedClassAdapter<uint8>) == sizeof(uint8));
	static_assert(sizeof(AxManagedClassAdapter<uint16>) == sizeof(uint16));
	static_assert(sizeof(AxManagedClassAdapter<uint32>) == sizeof(uint32));
	static_assert(sizeof(AxManagedClassAdapter<uint64>) == sizeof(uint64));
	static_assert(sizeof(AxManagedClassAdapter<float32>) == sizeof(float32));
	static_assert(sizeof(AxManagedClassAdapter<float64>) == sizeof(float64));

	template <typename T>
	constexpr auto adapt_to_managed_class(T* ptr)
	{
		return new (ptr) AxManagedClassAdapter<T>(ptr);
	}

	template <typename T>
	constexpr auto from_managed_adapter(AxManagedClassAdapter<T>* const ptr) -> T*
	{
		if constexpr (apex::numeric<T>)
		{
			return &ptr->value();
		}
		else
		{
			return static_cast<T*>(ptr);
		}
	}

	template <typename T>
	constexpr auto from_managed_adapter(AxManagedClassAdapter<T> const* const ptr) -> const T*
	{
		if constexpr (apex::numeric<T>)
		{
			return &ptr->value();
		}
		else
		{
			return static_cast<const T*>(ptr);
		}
	}

	template <typename T>
	concept managed_class = std::derived_from<T, AxManagedClass>;

	template <typename T>
	struct is_managed_adapted_class : std::false_type {};

	template <typename T>
	struct is_managed_adapted_class<AxManagedClassAdapter<T>> : std::true_type {};

	template <typename T>
	constexpr bool is_managed_adapted_class_v = is_managed_adapted_class<T>::value;

	template <typename T>
	concept managed_adapted_class = is_managed_adapted_class_v<T>;

	template <typename T>
	struct remove_managed_adapter { using type = T; };

	template <typename T>
	struct remove_managed_adapter<AxManagedClassAdapter<T>> { using type = T; };

	template <typename T>
	using remove_managed_adapter_t = typename remove_managed_adapter<T>::type;

}

