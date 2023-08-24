#pragma once
#include <optional>

#include "AxManagedClass.h"

namespace apex {

	namespace internal::unique_ptr {
		/**
		 * @brief checks if T2 is a non-array type convertible to T
		 */
		template <typename T, typename T2> 
		concept convertible_to = std::negation_v<std::is_array<T2>> && std::convertible_to<T2, T>;

		/**
		 * @brief checks if U is a pointer type convertible to T[]
		 */
		template <typename T, typename U> 
		concept ptr_convertible_to_array = std::same_as<U, T*> || (std::is_pointer_v<U> && std::convertible_to<std::remove_pointer_t<U> (*)[], T (*)[]>);

		/**
		 * @brief checks if U is an array type convertible to T[]
		 */
		template <typename T, typename U> 
		concept array_convertible_to_array = std::is_array_v<U> && std::is_pointer_v<U> && std::convertible_to<std::remove_pointer_t<U> (*)[], T (*)[]>;

		template <typename T> requires (!std::is_array_v<T>)
		void default_delete(T* ptr)
		{
			delete ptr;
		}

		template <typename T> requires (std::is_array_v<T>)
		void default_delete(std::remove_extent_t<T>* ptr)
		{
			delete[] ptr;
		}
	}

	template <typename T>
	class UniquePtr {};

	template <apex::managed_class T>
	class UniquePtr<T>
	{
		static_assert(!std::is_reference_v<T>);

	public:
		using element_type = T;
		using pointer      = T*;
		using reference    = T&;

		// default ctor
		constexpr UniquePtr() noexcept : m_ptr() {}

		// parameter initializing ctor
		constexpr explicit UniquePtr(pointer ptr) noexcept : m_ptr(ptr) {}

		// move ctor
		constexpr UniquePtr(UniquePtr&& other) noexcept : m_ptr(other.release()) {}

		constexpr UniquePtr& operator=(UniquePtr&& other) noexcept
		{
			if (this != std::addressof(other))
			{
				reset(other.release());
			}
			return *this;
		}

		// converting move ctor
		template <typename T2> requires internal::unique_ptr::convertible_to<T, T2>
		constexpr UniquePtr(UniquePtr<T2>&& other) noexcept : m_ptr(other.release()) {}

		template <typename T2> requires internal::unique_ptr::convertible_to<T, T2>
		constexpr UniquePtr& operator=(UniquePtr<T2>&& other) noexcept
		{
			reset(other.release());
			return *this;
		}

		// nullptr ctor
		constexpr UniquePtr(nullptr_t) noexcept : m_ptr() {}

		constexpr UniquePtr& operator=(nullptr_t) noexcept
		{
			reset();
			return *this;
		}

		// dtor
		constexpr ~UniquePtr() noexcept
		{
			if (m_ptr)
			{
				reset();
			}
		}

		// member methods

		[[nodiscard]] constexpr reference operator*() const noexcept
		{
			return *m_ptr;
		}

		[[nodiscard]] constexpr pointer operator->() const noexcept
		{
			return m_ptr;
		}

		[[nodiscard]] constexpr pointer get() const noexcept
		{
			return m_ptr;
		}

		constexpr explicit operator bool() const noexcept
		{
			return static_cast<bool>(m_ptr);
		}

		constexpr void swap(UniquePtr& other) noexcept
		{
			std::swap(m_ptr, other.m_ptr);
		}

		constexpr pointer release() noexcept
		{
			return std::exchange(m_ptr, nullptr);
		}

		constexpr void reset(pointer ptr = nullptr) noexcept
		{
			pointer old = std::exchange(m_ptr, ptr);
			internal::unique_ptr::default_delete(old);
		}

		UniquePtr(const UniquePtr&)            = delete;
		UniquePtr& operator=(UniquePtr const&) = delete;

	private:
		pointer m_ptr;

		template <typename> friend class UniquePtr;
	};

	template <apex::managed_class T>
	class UniquePtr<T[]>
	{
		static_assert(!std::is_reference_v<T>);

	public:
		using element_type = T;
		using pointer      = T*;
		using reference    = T&;

		// default ctor
		constexpr UniquePtr() noexcept : m_ptr() {}

		// converting parameter ctor
		template <typename T2> requires internal::unique_ptr::ptr_convertible_to_array<T, T2>
		constexpr UniquePtr(T2 ptr) noexcept : m_ptr(ptr) {}

		// move ctor
		constexpr UniquePtr(UniquePtr&& other) noexcept : m_ptr(other.release()) {}

		constexpr UniquePtr& operator=(UniquePtr&& other) noexcept
		{
			if (this != std::addressof(other))
			{
				reset(other.release());
			}
			return *this;
		}

		// converting move ctor
		template <typename U> requires internal::unique_ptr::array_convertible_to_array<T, U>
		constexpr UniquePtr(UniquePtr<U>&& other) noexcept : m_ptr(other.release()) {}

		template <typename U> requires internal::unique_ptr::array_convertible_to_array<T, U>
		constexpr UniquePtr& operator=(UniquePtr<U>&& other) noexcept
		{
			reset(other.release());
			return *this;
		}

		// nullptr ctor
		constexpr UniquePtr(nullptr_t) noexcept : m_ptr() {}

		constexpr UniquePtr& operator=(nullptr_t) noexcept
		{
			reset();
			return *this;
		}

		// dtor
		constexpr ~UniquePtr() noexcept
		{
			if (m_ptr)
			{
				reset();
			}
		}

		// member methods

		[[nodiscard]] constexpr reference operator[](size_t idx) const noexcept
		{
			return m_ptr[idx];
		}

		[[nodiscard]] constexpr pointer get() noexcept
		{
			return m_ptr;
		}

		constexpr explicit operator bool() const noexcept
		{
			return static_cast<bool>(m_ptr);
		}

		constexpr pointer release() noexcept
		{
			return std::exchange(m_ptr, nullptr);
		}

		constexpr void reset(nullptr_t = nullptr) noexcept
		{
			reset(pointer());
		}

		template <typename U> requires internal::unique_ptr::ptr_convertible_to_array<T, U>
		constexpr void reset(U ptr) noexcept
		{
			pointer old = std::exchange(m_ptr, ptr);
			internal::unique_ptr::default_delete<T[]>(old);
		}
		
		UniquePtr(const UniquePtr&)            = delete;
		UniquePtr& operator=(UniquePtr const&) = delete;

	private:
		pointer m_ptr;

		template <typename> friend class UniquePtr;
	};

	// make a UniquePtr from a AxHandle
	template <apex::managed_class T, typename... Args> requires (!std::is_array_v<T>) // managed_class , not array
	[[nodiscard]] constexpr auto unique_from_handle(apex::AxHandle& handle, Args&&... args) noexcept -> UniquePtr<T>
	{
	    return UniquePtr<T>(new (handle) T(std::forward<Args>(args)...));
	}

	// make a UniquePtr from a AxHandle
	template <typename T, typename...Args> requires(!apex::managed_class<T> && !std::is_array_v<T>) // not managed_class , not array
	[[nodiscard]] constexpr auto unique_from_handle(apex::AxHandle& handle, Args&&... args) noexcept -> UniquePtr<AxManagedClassAdapter<T>>
	{
		return apex::unique_from_handle<AxManagedClassAdapter<T>>(handle, std::forward<Args>(args)...);
	}

	// make a UniquePtr from a AxHandle
	template <typename T> requires (apex::managed_class<std::remove_extent_t<T>> && std::is_array_v<T> && std::extent_v<T> == 0) // managed_class , array
	[[nodiscard]] constexpr auto unique_from_handle(apex::AxHandle& handle, const size_t size) -> UniquePtr<T>
	{
		using element_type = std::remove_extent_t<T>;
		if constexpr (std::is_default_constructible_v<element_type>)
		{
			return UniquePtr<T>(new (handle) element_type[size]());
		}
		else
		{
			return UniquePtr<T>(new (handle) element_type[size]);
		}
	}

	// make a UniquePtr from a AxHandle
	template <typename T> requires (!apex::managed_class<std::remove_extent_t<T>> && std::is_array_v<T> && std::extent_v<T> == 0) // not managed_class , array
	[[nodiscard]] constexpr auto unique_from_handle(apex::AxHandle& handle, const size_t size) -> UniquePtr<AxManagedClassAdapter<std::remove_extent_t<T>>[]>
	{
	    /*static_assert(false,
			"apex::UniquePtr does not support unmanaged array types! "
			"Use an AxArray for dynamic sized arrays or AxStaticArray for static arrays!");*/

		using inner_element_type = std::remove_extent_t<T>;
		using element_type = AxManagedClassAdapter<inner_element_type>;

		return apex::unique_from_handle<element_type[]>(handle, size);
	}

	// make a UniquePtr
	template <apex::managed_class T, typename... Args> requires (!std::is_array_v<T>) // managed_class , not array
	[[nodiscard]] constexpr auto make_unique(Args&&... args) noexcept -> UniquePtr<T>
	{
		AxHandle handle = make_handle<T>();
	    return UniquePtr<T>(new (handle) T(std::forward<Args>(args)...));
	}

	// make a UniquePtr
	template <typename T, typename... Args> requires (!apex::managed_class<T> && !std::is_array_v<T>) // not managed_class , not array
	[[nodiscard]] constexpr auto make_unique(Args&&... args) noexcept -> UniquePtr<AxManagedClassAdapter<T>>
	{
		return apex::make_unique<AxManagedClassAdapter<T>>(std::forward<Args>(args)...);
	}

	// make a UniquePtr
	template <typename T> requires (apex::managed_class<std::remove_extent_t<T>> && std::is_array_v<T> && std::extent_v<T> == 0) // managed_class , array
	[[nodiscard]] constexpr UniquePtr<T> make_unique(const size_t size)
	{
	    using element_type = std::remove_extent_t<T>;
		AxHandle handle;
		if constexpr (std::is_default_constructible_v<element_type>)
		{
			return UniquePtr<T>(new (handle) element_type[size]());
		}
		else
		{
			return UniquePtr<T>(new (handle) element_type[size]);
		}
	}

	// make a UniquePtr
	template <typename T> requires (!apex::managed_class<std::remove_extent_t<T>> && std::is_array_v<T> && std::extent_v<T> == 0) // not managed_class , array
	[[nodiscard]] constexpr auto make_unique(const size_t size) -> UniquePtr<AxManagedClassAdapter<std::remove_extent_t<T>>[]>
	{
		using inner_element_type = std::remove_extent_t<T>;
		using element_type = AxManagedClassAdapter<inner_element_type>;

	    return apex::make_unique<element_type[]>(size);
	}

}
