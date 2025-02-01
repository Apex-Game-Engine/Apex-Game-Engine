#pragma once

#include "AxManagedClass.h"
#include <mutex>

#include "UniquePtr.h"
#include "Concurrency/Concurrency.h"
#include "Core/Asserts.h"

namespace apex {

	using default_shared_ptr_lock = cncy::NullLock;

	template <typename T, typename Lock = default_shared_ptr_lock>
	class SharedPtr;

	template <apex::managed_class T, concurrency::lockable Lock>
	class SharedPtr<T, Lock> : public AxManagedClass
	{
		static_assert(!std::is_reference_v<T>);

	public:
		using underlying_type = apex::remove_managed_adapter_t<T>;
		using value_type      = underlying_type;
		using pointer         = value_type*;
		using reference       = value_type&;
		using mutex_type      = Lock;

	private:
		struct SharedPtrData : public AxManagedClass
		{
			UniquePtr<T> ptr{ nullptr };
			size_t       refCount{ 0 };
			Lock         mutex{};
		};

		using storage_type = SharedPtrData;

	public:
		// default ctor
		constexpr SharedPtr() noexcept : m_data(nullptr) {}

		// parameter initializing ctor
		constexpr explicit SharedPtr(pointer ptr) noexcept : m_data(nullptr)
		{
			_Allocate();
			// std::lock_guard lock(m_data->mutex);
			m_data->ptr.reset(ptr);
			m_data->refCount = 1;
		}

		template <typename TWrappedPtr = std::enable_if_t<apex::is_managed_adapted_class_v<T>, T>>
		constexpr explicit SharedPtr(TWrappedPtr* ptr) noexcept : m_data(nullptr)
		{
			_Allocate();
			// std::lock_guard lock(m_data->mutex);
			m_data->ptr.reset(apex::from_managed_adapter(ptr));
			m_data->refCount = 1;
		}

		// copy ctor
		constexpr SharedPtr(const SharedPtr& other) noexcept
			: m_data(other.m_data)
		{
			if (m_data)
			{
				std::lock_guard lock(m_data->mutex);
				++m_data->refCount;
			}
		}

		// copy assignment
		constexpr SharedPtr& operator=(const SharedPtr& other) noexcept
		{
			if (this != std::addressof(other))
			{
				reset();
				m_data = other.m_data;
				if (m_data)
				{
					std::lock_guard lock(m_data->mutex);
					++m_data->refCount;
				}
			}
			return *this;
		}

		// move ctor
		constexpr SharedPtr(SharedPtr&& other) noexcept
			: m_data(other.m_data)
		{
			other.m_data = nullptr;
		}

		// move assignment
		constexpr SharedPtr& operator=(SharedPtr&& other) noexcept
		{
			if (this != std::addressof(other))
			{
				reset();
				m_data = other.m_data;
				other.m_data = nullptr;
			}
			return *this;
		}

		// nullptr ctor
		constexpr SharedPtr(nullptr_t) noexcept : m_data(nullptr) {}

		// nullptr assignment
		constexpr SharedPtr& operator=(nullptr_t) noexcept
		{
			reset();
			return *this;
		}

		// dtor
		constexpr ~SharedPtr() noexcept
		{
			reset();
		}

		// member methods

		[[nodiscard]] constexpr reference operator*() const noexcept
		{
			axAssertFmt(m_data, "Attempted to dereference a null SharedPtr");
			axAssertFmt(m_data->ptr, "Attempted to dereference a null SharedPtr");
			return *m_data->ptr;
		}

		[[nodiscard]] constexpr pointer operator->() const noexcept
		{
			axAssertFmt(m_data, "Attempted to dereference a null SharedPtr");
			axAssertFmt(m_data->ptr, "Attempted to dereference a null SharedPtr");
			return m_data->ptr.get();
		}

		[[nodiscard]] constexpr pointer get() const noexcept
		{
			axAssertFmt(m_data, "Attempted to dereference a null SharedPtr");
			return m_data->ptr.get();
		}

		[[nodiscard]] constexpr explicit operator bool() const noexcept
		{
			return static_cast<bool>(m_data) && static_cast<bool>(m_data->ptr);
		}

		constexpr void reset() noexcept
		{
			if (m_data)
			{
				std::lock_guard lock(m_data->mutex);
				if (--m_data->refCount == 0)
				{
					apex::default_delete(m_data);
				}
			}
		}

		[[nodiscard]] constexpr size_t use_count() const noexcept
		{
			return m_data ? m_data->refCount : 0;
		}

	private:
		constexpr void _Allocate()
		{
			AxHandle handle(sizeof(storage_type));
			m_data = new (handle) storage_type();
		}

	private:
		storage_type* m_data;
	};

	template <apex::managed_class T, cncy::lockable Lock>
	class SharedPtr<T[], Lock> : public AxManagedClass
	{
		static_assert(!std::is_reference_v<T>);

	public:
		using element_type = T;
		using pointer      = T*;
		using reference    = T&;
		using mutex_type   = Lock;

	private:
		struct SharedPtrData : public AxManagedClass
		{
			UniquePtr<T[]> ptr{ nullptr };
			size_t         refCount{ 0 };
			Lock           mutex{};
		};

		using storage_type = SharedPtrData;

	public:

		// default ctor
		constexpr SharedPtr() noexcept : m_data(nullptr) {}

		// converting parameter ctor
		template <typename T2> requires apex::ptr_convertible_to_array<T, T2>
		constexpr SharedPtr(T2 ptr) noexcept : m_data(nullptr)
		{
			_Allocate();
			// std::lock_guard lock(m_data->mutex);
			m_data->ptr.reset(ptr);
			m_data->refCount = 1;
		}

		// copy ctor
		constexpr SharedPtr(const SharedPtr& other) noexcept
			: m_data(other.m_data)
		{
			if (m_data)
			{
				std::lock_guard lock(m_data->mutex);
				++m_data->refCount;
			}
		}

		// copy assignment
		constexpr SharedPtr& operator=(const SharedPtr& other) noexcept
		{
			if (this != std::addressof(other))
			{
				reset();
				m_data = other.m_data;
				if (m_data)
				{
					std::lock_guard lock(m_data->mutex);
					++m_data->refCount;
				}
			}
			return *this;
		}

		// move ctor
		constexpr SharedPtr(SharedPtr&& other) noexcept
			: m_data(other.m_data)
		{
			other.m_data = nullptr;
		}

		// move assignment
		constexpr SharedPtr& operator=(SharedPtr&& other) noexcept
		{
			if (this != std::addressof(other))
			{
				reset();
				m_data = other.m_data;
				other.m_data = nullptr;
			}
			return *this;
		}

		// nullptr ctor
		constexpr SharedPtr(nullptr_t) noexcept : m_data(nullptr) {}

		// nullptr assignment
		constexpr SharedPtr& operator=(nullptr_t) noexcept
		{
			reset();
			return *this;
		}

		// dtor
		constexpr ~SharedPtr() noexcept
		{
			reset();
		}

		// member methods

		[[nodiscard]] constexpr reference operator[](size_t idx) const noexcept
		{
			axAssertFmt(m_data, "Attempted to dereference a null SharedPtr");
			axAssertFmt(m_data->ptr, "Attempted to dereference a null SharedPtr");
			return m_data->ptr[idx];
		}

		[[nodiscard]] constexpr pointer get() const noexcept
		{
			axAssertFmt(m_data, "Attempted to dereference a null SharedPtr");
			return m_data->ptr.get();
		}

		[[nodiscard]] constexpr explicit operator bool() const noexcept
		{
			return static_cast<bool>(m_data) && static_cast<bool>(m_data->ptr);
		}

		constexpr void reset() noexcept
		{
			if (m_data)
			{
				std::lock_guard lock(m_data->mutex);
				if (--m_data->refCount == 0)
				{
					apex::default_delete(m_data);
				}
			}
		}

		[[nodiscard]] constexpr size_t use_count() const noexcept
		{
			return m_data ? m_data->refCount : 0;
		}

	private:
		constexpr void _Allocate()
		{
			AxHandle handle(sizeof(storage_type));
			m_data = new (handle) storage_type();
		}

	private:
		storage_type* m_data;
	};

	// make a SharedPtr from a AxHandle
	template <apex::managed_class T, cncy::lockable Lock = default_shared_ptr_lock, typename... Args> requires (!std::is_array_v<T>) // managed_class , not array
	[[nodiscard]] auto shared_from_handle(apex::AxHandle& handle, Args&&... args) noexcept -> SharedPtr<T, Lock>
	{
		return SharedPtr<T, Lock>(new (handle) T(std::forward<Args>(args)...));
	}

	// make a SharedPtr from a AxHandle
	template <typename T, cncy::lockable Lock = default_shared_ptr_lock, typename... Args> requires(!apex::managed_class<T> && !std::is_array_v<T>) // not managed_class , not array
	[[nodiscard]] auto shared_from_handle(apex::AxHandle& handle, Args&&... args) noexcept -> SharedPtr<AxManagedClassAdapter<T>, Lock>
	{
		return apex::shared_from_handle<AxManagedClassAdapter<T>, Lock>(handle, std::forward<Args>(args)...);
	}

	// make a SharedPtr from a AxHandle
	template <typename T, cncy::lockable Lock = default_shared_ptr_lock> requires (apex::managed_class<std::remove_extent_t<T>> && std::is_array_v<T> && std::extent_v<T> == 0) // managed_class , array
	[[nodiscard]] auto shared_from_handle(apex::AxHandle& handle, const size_t size) noexcept -> SharedPtr<T, Lock>
	{
		using element_type = std::remove_extent_t<T>;
		if constexpr (std::is_default_constructible_v<element_type>)
			return SharedPtr<T, Lock>(new (handle) element_type[size]());
		else
			return SharedPtr<T, Lock>(new (handle) std::remove_extent_t<T>[size]);
	}

	// make a SharedPtr from a AxHandle
	template <typename T, cncy::lockable Lock = default_shared_ptr_lock> requires (!apex::managed_class<std::remove_extent_t<T>> && std::is_array_v<T> && std::extent_v<T> == 0) // managed_class , array
	[[nodiscard]] auto shared_from_handle(apex::AxHandle& handle, const size_t size) noexcept -> SharedPtr<AxManagedClassAdapter<std::remove_extent_t<T>>[], Lock>
	{
		using inner_element_type = std::remove_extent_t<T>;
		using element_type = AxManagedClassAdapter<inner_element_type>;

		return apex::shared_from_handle<element_type[], Lock>(handle, size);
	}

	// make a SharedPtr
	template <apex::managed_class T, cncy::lockable Lock = default_shared_ptr_lock, typename... Args> requires(!std::is_array_v<T>) // managed_class , not array
	[[nodiscard]] auto make_shared(Args&&... args) noexcept -> SharedPtr<T, Lock>
	{
		AxHandle handle = make_handle<T>();
		return apex::shared_from_handle<T, Lock>(handle, std::forward<Args>(args)...);
	}

	// make a SharedPtr
	template <typename T, cncy::lockable Lock = default_shared_ptr_lock, typename... Args> requires(!apex::managed_class<T> && !std::is_array_v<T>) // not managed_class , not array
	[[nodiscard]] auto make_shared(Args&&... args) noexcept -> SharedPtr<AxManagedClassAdapter<T>, Lock>
	{
		return apex::make_shared<AxManagedClassAdapter<T>, Lock>(std::forward<Args>(args)...);
	}

	// make a SharedPtr
	template <typename T, cncy::lockable Lock = default_shared_ptr_lock> requires (apex::managed_class<std::remove_extent_t<T>> && std::is_array_v<T> && std::extent_v<T> == 0) // managed_class , array
	[[nodiscard]] auto make_shared(const size_t size) noexcept -> SharedPtr<T, Lock>
	{
		AxHandle handle = make_handle<T>(size);
		return shared_from_handle<T, Lock>(handle, size);
	}

	// make a SharedPtr
	template <typename T, cncy::lockable Lock = default_shared_ptr_lock> requires (!apex::managed_class<std::remove_extent_t<T>> && std::is_array_v<T> && std::extent_v<T> == 0) // not managed_class , array
	[[nodiscard]] auto make_shared(const size_t size) noexcept -> SharedPtr<AxManagedClassAdapter<std::remove_extent_t<T>>[], Lock>
	{
		using inner_element_type = std::remove_extent_t<T>;
		using element_type = AxManagedClassAdapter<inner_element_type>;

		return apex::make_shared<element_type[], Lock>(size);
	}

}
