#pragma once

#include "Core/Asserts.h"
#include "AxManagedClass.h"

namespace apex {

	template <typename T>
	class UniquePtr;

	template <apex::managed_class T>
	class UniquePtr<T> : public AxManagedClass
	{
		static_assert(!std::is_reference_v<T>);

	public:
		using underlying_type = apex::remove_managed_adapter_t<T>;
		using value_type     = underlying_type;
		using pointer        = value_type*;
		using reference      = value_type&;
		using stored_ptr     = T*;

		// default ctor
		constexpr UniquePtr() noexcept : m_ptr() {}

		// parameter initializing ctor
		constexpr explicit UniquePtr(pointer ptr) noexcept : m_ptr(ptr) {}

		template <typename WrappedPtr = std::enable_if_t<apex::is_managed_adapted_class_v<T>, T>>
		constexpr explicit UniquePtr(WrappedPtr ptr) noexcept : m_ptr(ptr) {}

		// move ctor
		constexpr UniquePtr(UniquePtr&& other) noexcept : m_ptr(other.release()) {}

		// move assignment
		constexpr UniquePtr& operator=(UniquePtr&& other) noexcept
		{
			if (this != std::addressof(other))
			{
				reset(other.release());
			}
			return *this;
		}

		// converting move ctor
		template <typename T2> requires apex::convertible_to<T2, T>
		constexpr UniquePtr(UniquePtr<T2>&& other) noexcept : m_ptr(other.release()) {}

		// converting move assignment
		template <typename T2> requires apex::convertible_to<T2, T>
		constexpr UniquePtr& operator=(UniquePtr<T2>&& other) noexcept
		{
			reset(other.release());
			return *this;
		}

		// nullptr ctor
		constexpr UniquePtr(nullptr_t) noexcept : m_ptr() {}

		// nullptr assignment
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
			axAssertFmt(m_ptr, "Attempted to dereference a null UniquePtr");
			return *_ConvertToPointer();
		}

		[[nodiscard]] constexpr pointer operator->() const noexcept
		{
			axAssertFmt(m_ptr, "Attempted to dereference a null UniquePtr");
			return _ConvertToPointer();
		}

		[[nodiscard]] constexpr pointer get() const noexcept
		{
			return _ConvertToPointer();
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
			stored_ptr old = std::exchange(m_ptr, stored_ptr(ptr));
			apex::default_delete(old);
		}

		UniquePtr(const UniquePtr&)            = delete;
		UniquePtr& operator=(UniquePtr const&) = delete;

	protected:
		auto _ConvertToPointer() const noexcept -> pointer
		{
			if constexpr (apex::is_managed_adapted_class_v<T>)
			{
				return apex::from_managed_adapter<value_type>(m_ptr);
			}
			else
			{
				return m_ptr;
			}
		}

	private:
		stored_ptr m_ptr;

		template <typename> friend class UniquePtr;
	};

	template <apex::managed_class T>
	class UniquePtr<T[]> : public AxManagedClass
	{
		static_assert(!std::is_reference_v<T>);

	public:
		using element_type = T;
		using pointer      = T*;
		using reference    = T&;

		// default ctor
		constexpr UniquePtr() noexcept : m_ptr() {}

		// converting parameter ctor
		template <typename T2> requires apex::ptr_convertible_to_array<T, T2>
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
		template <typename U> requires apex::array_convertible_to_array<T, U>
		constexpr UniquePtr(UniquePtr<U>&& other) noexcept : m_ptr(other.release()) {}

		template <typename U> requires apex::array_convertible_to_array<T, U>
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

		[[nodiscard]] constexpr explicit operator bool() const noexcept
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

		template <typename U> requires apex::ptr_convertible_to_array<T, U>
		constexpr void reset(U ptr) noexcept
		{
			pointer old = std::exchange(m_ptr, ptr);
			apex::default_delete<T[]>(old);
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
		using inner_element_type = std::remove_extent_t<T>;
		using element_type = AxManagedClassAdapter<inner_element_type>;

		return apex::unique_from_handle<element_type[]>(handle, size);
	}

	// make a UniquePtr
	template <apex::managed_class T, typename... Args> requires (!std::is_array_v<T>) // managed_class , not array
	[[nodiscard]] constexpr auto make_unique(Args&&... args) noexcept -> UniquePtr<T>
	{
		AxHandle handle = make_handle<T>();
	    return apex::unique_from_handle<T>(handle, std::forward<Args>(args)...);
	}

	// make a UniquePtr
	template <typename T, typename... Args> requires (!apex::managed_class<T> && !std::is_array_v<T>) // not managed_class , not array
	[[nodiscard]] constexpr auto make_unique(Args&&... args) noexcept -> UniquePtr<AxManagedClassAdapter<T>>
	{
		return apex::make_unique<AxManagedClassAdapter<T>>(std::forward<Args>(args)...);
	}

	// make a UniquePtr
	template <typename T> requires (apex::managed_class<std::remove_extent_t<T>> && std::is_array_v<T> && std::extent_v<T> == 0) // managed_class , array
	[[nodiscard]] constexpr auto make_unique(const size_t size) -> UniquePtr<T>
	{
		AxHandle handle = make_handle<T>(size);
		return apex::unique_from_handle<T>(handle, size);
	}

	// make a UniquePtr
	template <typename T> requires (!apex::managed_class<std::remove_extent_t<T>> && std::is_array_v<T> && std::extent_v<T> == 0) // not managed_class , array
	[[nodiscard]] constexpr auto make_unique(const size_t size) -> UniquePtr<AxManagedClassAdapter<std::remove_extent_t<T>>[]>
	{
		using inner_element_type = std::remove_extent_t<T>;
		using element_type = AxManagedClassAdapter<inner_element_type>;

	    return apex::make_unique<element_type[]>(size);
	}


	template <typename Base, typename T> requires (apex::managed_class<T> && std::is_base_of_v<Base, T>)
	[[nodiscard]] constexpr auto static_unique_cast(UniquePtr<T>&& ptr) noexcept -> UniquePtr<Base>
	{
		return UniquePtr<Base>(static_cast<Base*>(ptr.release()));
	}

}
