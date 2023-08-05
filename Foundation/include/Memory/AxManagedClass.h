﻿#pragma once
#include <memory>

#include "AxHandle.h"
#include "Core/Types.h"

#ifndef APEX_ENABLE_TESTS
void* operator new(size_t size);
void* operator new[](size_t size);

void operator delete(void* mem);
void operator delete[](void* mem);
#endif

namespace apex {

	namespace memory {
		class MemoryManagerTest;
		class MemoryManager;
	}

	struct AxManagedClass
	{
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

	template <typename T>
	class AxManagedClassAdapter : public AxManagedClass, public T
	{
	public:
		template <typename... Args>
		AxManagedClassAdapter(Args&&... args) : T(std::forward<Args>(args)...) {}

		constexpr operator T() { return static_cast<T>(*this); }
	};

	template <typename T> requires (std::integral<T> || std::floating_point<T>)
	class AxManagedClassAdapter<T> : public AxManagedClass
	{
	public:
		using value_type = T;
		using pointer    = T*;

		constexpr AxManagedClassAdapter() = default;
		constexpr AxManagedClassAdapter(T value) : m_value(value) {}
		constexpr AxManagedClassAdapter& operator=(T other) { m_value = other; return *this; }

		constexpr operator T() { return m_value; }

		constexpr bool operator<=>(T const& other) const { return m_value <=> other; }

	private:
		T m_value;
	};

	template <typename T>
	constexpr auto adapt_to_managed_class(T* ptr)
	{
		ptr = new (ptr) AxManagedClassAdapter<T>(ptr);
	}

	template <typename T>
	concept managed_class = std::is_base_of_v<AxManagedClass, T>;

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

//template <typename T>
//inline constexpr bool std::is_array_v<apex::AxManagedClassAdapter<T>> = false;
//
//template <typename T>
//inline constexpr bool std::is_array_v<apex::AxManagedClassAdapter<T[]>> = true;
//
//template <typename T, size_t N>
//inline constexpr bool std::is_array_v<apex::AxManagedClassAdapter<T[N]>> = true;
