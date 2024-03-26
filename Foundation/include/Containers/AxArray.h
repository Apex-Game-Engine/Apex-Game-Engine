#pragma once
#include "Core/Asserts.h"
#include "Core/TypeTraits.h"
#include "Core/Utility.h"
#include "Memory/AxHandle.h"
#include "Memory/AxManagedClass.h"
#include "Memory/UniquePtr.h"

namespace apex {

	/**
	 * \brief Resizable dynamic array
	 * \tparam T type of elements stored in the array
	 */
	template <typename T, typename = SelfManaged>
	class AxArray : public AxManagedClass
	{
	public:
		using underlying_type = T;

		template <typename IterType>
		class Iterator
		{
		public:
			using iterator_category = std::contiguous_iterator_tag;
			using value_type        = IterType;
			using difference_type   = ptrdiff_t;
			using pointer           = std::conditional_t<std::is_const_v<IterType>, const underlying_type*, underlying_type*>;
			using reference         = std::conditional_t<std::is_const_v<IterType>, const underlying_type&, underlying_type&>;;

			Iterator() : m_ptr() {}
			Iterator(IterType* ptr) : m_ptr(ptr) {}
			Iterator& operator++() { ++m_ptr; return *this; } // pre-increment
			Iterator operator++(int) { Iterator tmp(*this); operator++(); return tmp; } // post-increment
			Iterator& operator--() { --m_ptr; return *this; } // pre-decrement
			Iterator operator--(int) { Iterator tmp(*this); operator--(); return tmp; } // post-decrement
			Iterator operator+(difference_type size) { return { m_ptr + size }; } // TODO: add an assert to check for overflow?
			Iterator operator-(difference_type size) { return { m_ptr - size }; } // TODO: add an assert to check for overflow?
			bool operator==(const Iterator& rhs) const { return m_ptr == rhs.m_ptr; }
			bool operator!=(const Iterator& rhs) const { return m_ptr != rhs.m_ptr; }
			bool operator<(const Iterator& rhs) const { return m_ptr < rhs.m_ptr; }
			bool operator>(const Iterator& rhs) const { return m_ptr > rhs.m_ptr; }
			difference_type operator-(const Iterator& rhs) const { return m_ptr - rhs.m_ptr; }
			reference operator*() const { return *_ToUnderlyingPointer(); }
			pointer operator->() const { return _ToUnderlyingPointer(); }

		protected:
			[[nodiscard]] pointer _ToUnderlyingPointer() const
			{
				if constexpr (std::same_as<std::remove_cv_t<underlying_type>, std::remove_cv_t<IterType>>)
				{
					return m_ptr;
				}
				else
				{
					return apex::from_managed_adapter(m_ptr);
				}
			}

		private:
			IterType* m_ptr;
		};

	public:
		using stored_type = std::conditional_t<apex::managed_class<T>, underlying_type, AxManagedClassAdapter<T>>;

		using value_type = underlying_type;
		using pointer = value_type*;
		using const_pointer = const value_type*;
		using reference = value_type&;
		using const_reference = const value_type&;
		using iterator = Iterator<stored_type>;
		using const_iterator = Iterator<const stored_type>;

		// static_assert(sizeof(stored_type) == sizeof(value_type));
		static constexpr auto _size_check = TAssertEqualSize<stored_type, value_type>::value();

		AxArray() = default;

		explicit AxArray(size_t capacity)
		{
			reserve(capacity);
		}

		AxArray(AxArray&& other) noexcept
		: m_capacity(std::move(other.m_capacity))
		, m_size(std::move(other.m_size))
		, m_data(std::move(other.m_data))
		{
			other.m_data = nullptr;
		}

		AxArray(std::initializer_list<value_type> init_list)
		: m_capacity(init_list.size())
		, m_size(init_list.size())
		{
			_Allocate(m_capacity, m_size);
			apex::memcpy_s<value_type>(m_data, m_size, init_list.begin(), init_list.size());
		}

		~AxArray()
		{
			_DestroyAll();
			delete m_data;
		}

		AxArray& operator=(AxArray&& other) noexcept
		{
			m_capacity = std::move(other.m_capacity);
			m_size = std::move(other.m_size);
			m_data = std::move(other.m_data);

			other.m_data = nullptr;

			return *this;
		}

		template <typename... Args>
		void resize(size_t new_size, Args&&... args)
		{
			reserve(new_size);

			size_t oldSize = m_size;
			m_size = new_size;

			if constexpr (sizeof...(args) > 0)
			{
				if (new_size > oldSize)
				{
					fill(m_data + oldSize, m_data + new_size, std::forward<Args>(args)...);
				}
			}
		}

		void reserve(size_t capacity)
		{
			if (m_capacity < capacity)
			{
				_ReallocateAndMove(capacity);
			}
		}

		template <typename... Args>
		reference emplace_back(Args&&... args)
		{
			axAssertMsg(m_size < m_capacity, "Array size exceeds capacity!");
			_ConstructInPlace(&m_data[m_size], std::forward<Args>(args)...);
			return m_data[m_size++];
		}

		template <typename... Args>
		reference emplace(size_t index, Args&&... args)
		{
			axAssert(index < m_size + 1 && index < m_capacity);
			axAssertMsg(m_size < m_capacity, "Array size exceeds capacity!");
			// shift elements to the right from index upto size
			_ShiftElementsRight(index);
			_ConstructInPlace(&m_data[index], std::forward<Args>(args)...);
			m_size++;
			return m_data[index];
		}

		void append(const value_type& obj)
		{
			axAssertMsg(m_size < m_capacity, "Array size exceeds capacity!");
			m_data[m_size++] = obj;
		}

		void append(value_type&& obj)
		{
			axAssertMsg(m_size < m_capacity, "Array size exceeds capacity!");
			m_data[m_size++] = std::move(obj);
		}

		void pop_back()
		{
			axAssertMsg(m_size > 0, "Cannot pop from empty array!");
			_DestroyInPlace(m_data + m_size - 1);
			m_size--;
		}

		void insert(size_t index, const value_type& obj)
		{
			axAssert(index < m_size + 1 && index < m_capacity);
			axAssertMsg(m_size < m_capacity, "Array size exceeds capacity!");
			// shift elements to the right from index upto size
			_ShiftElementsRight(index);
			m_data[index] = obj;
			m_size++;
		}

		void insert(size_t index, value_type&& obj)
		{
			axAssert(index < m_size + 1 && index < m_capacity);
			axAssertMsg(m_size < m_capacity, "Array size exceeds capacity!");
			// shift elements to the right from index upto size
			_ShiftElementsRight(index);
			// append(std::move(obj));
			m_data[index] = std::move(obj);
			m_size++;
		}

		// void emplace(size_t index, )

		void remove(size_t index)
		{
			axAssert(index < m_size);
			// shift elements to left from index upto size
			if (index < m_size - 1)
				_ShiftElementsLeft(index);
			// pop last element
			pop_back();
		}

		void reset()
		{
			m_capacity = 0;
			m_size = 0;
			_DestroyAll();
		}

		void clear()
		{
			m_size = 0;
			_DestroyAll();
		}

		[[nodiscard]] auto data() -> pointer { return _ConvertToValuePointer(); }
		[[nodiscard]] auto data() const -> const_pointer { return const_cast<AxArray * const>(this)->_ConvertToValuePointer(); }

		[[nodiscard]] auto dataMutable() const -> pointer { return const_cast<AxArray * const>(this)->_ConvertToValuePointer(); }

		[[nodiscard]] auto back() -> reference { return m_data[m_size - 1]; }
		[[nodiscard]] auto back() const -> const_reference { return m_data[m_size - 1]; }

		[[nodiscard]] auto front() -> reference { return m_data[0]; }
		[[nodiscard]] auto front() const -> const_reference { return m_data[0]; }

		[[nodiscard]] auto at(size_t index) -> reference { return this->operator[](index); }
		[[nodiscard]] auto at(size_t index) const -> std::conditional_t<std::is_trivial_v<T>, value_type, const_reference> { return this->operator[](index); }

		[[nodiscard]] auto operator[](size_t index) -> reference
		{
			axAssert(index < m_size);

			if constexpr (apex::is_managed_adapted_class_v<stored_type>)
			{
				return *from_managed_adapter(&m_data[index]);
			}
			else
			{
				return m_data[index];
			}
		}

		[[nodiscard]] auto operator[] (size_t index) const -> std::conditional_t<std::is_trivial_v<T>, value_type, const_reference>
		{
			return const_cast<AxArray* const>(this)->operator[](index);
		}
		
		[[nodiscard]] size_t size() const { return m_size; }
		[[nodiscard]] size_t capacity() const { return m_capacity; }
		[[nodiscard]] bool   empty() const { return m_size == 0; }

	#pragma region Iterator functions
		[[nodiscard]] iterator begin() { return iterator(m_data); }
		[[nodiscard]] iterator end() { return iterator(m_data + m_size); }

		[[nodiscard]] const_iterator cbegin() const { return const_iterator(m_data); }
		[[nodiscard]] const_iterator cend() const { return const_iterator(m_data + m_size); }

		[[nodiscard]] const_iterator begin() const { return cbegin(); }
		[[nodiscard]] const_iterator end() const { return cend(); }
	#pragma endregion

	protected:
		template <typename... Args>
		stored_type* _ConstructInPlace(stored_type* mem, Args&&... args) requires (std::is_constructible_v<T, Args...>)
		{
			return std::construct_at(mem, std::forward<Args>(args)...);
		}

		void _Allocate(size_t capacity, size_t init_size)
		{
			AxHandle newHandle (sizeof(value_type) * capacity);
			//AxHandle newHandle = apex::make_handle<value_type[]>(capacity);
			_SetDataHandle(newHandle, init_size);
		}

		void _ReallocateAndMove(size_t new_capacity)
		{
			auto oldData = m_data;
			auto oldSize = m_size;
			auto oldCapacity = m_capacity;

			_Allocate(new_capacity, oldSize);

			if (oldSize > 0)
				apex::memmove_s<stored_type>(m_data, m_capacity, oldData, oldSize);

			delete oldData;
		}

		void _DestroyInPlace(stored_type* ptr)
		{
			if constexpr (!std::is_trivially_destructible_v<T>)
				std::destroy_at(ptr);
		}

		void _DestroyAll()
		{
			if constexpr (!std::is_trivially_destructible_v<T>)
				for (size_t i = 0; i < m_size; ++i)
				{
					_DestroyInPlace(&m_data[i]);
				}
		}

		void _ShiftElementsRight(size_t index)
		{
			apex::memmove_s<stored_type>(&m_data[index+1], m_capacity - index, &m_data[index], m_size - index);
		}

		void _ShiftElementsLeft(size_t index)
		{
			const size_t moveNum = m_size - index;
			apex::memmove_s<stored_type>(&m_data[index], moveNum, &m_data[index+1], moveNum);
		}

		pointer _ConvertToValuePointer()
		{
			if constexpr (std::same_as<value_type, stored_type>)
			{
				return m_data;
			}
			else
			{
				return from_managed_adapter<value_type>(m_data);
			}
		}

		template <typename... Args>
		void fill(stored_type* first, stored_type* last, Args&&... args) requires (std::is_constructible_v<T, Args...>)
		{
			stored_type* ptr = first;
			for (; ptr != last; ++ptr)
			{
				_ConstructInPlace(ptr, std::forward<Args>(args)...);
			}
		}

		void _SetDataHandle(AxHandle& handle, size_t init_size)
		{
			m_capacity = handle.getBlockSize() / sizeof(value_type);
			m_size = init_size;
			m_data = handle.getAs<stored_type>();
		}

	protected:
		size_t m_capacity { 0 };
		size_t m_size { 0 };

		stored_type* m_data { nullptr };
		
		friend class AxArrayTest;
	};

	template <typename T>
	class AxArray<T, ExternallyManaged> : public AxArray<T>
	{
		using Base = AxArray<T>;

	public:
		explicit AxArray(AxHandle& handle)
		{
			Base::_SetDataHandle(handle, 0);
		}

		~AxArray() { /* Do nothing as the memory is externally owned and managed */ }

		template <typename... Args>
		void resize(size_t new_size, Args&&... args)
		{
			axAssertMsg(new_size <= Base::m_capacity, "Array size exceeds maximum capacity!");

			size_t oldSize = Base::m_size;
			Base::m_size = new_size;

			if constexpr (sizeof...(args) > 0)
			{
				if (new_size > oldSize)
				{
					Base::fill(Base::m_data + oldSize, Base::m_data + new_size, std::forward<Args>(args)...);
				}
			}
		}

		void reserve(size_t capacity) = delete;
	};

	template <typename T>
	void keepUniquesOnly_slow(AxArray<T>& arr)
	{
		size_t i = arr.size() - 1;
		for (; i > 0; --i)
		{
			for (size_t j = 0; j < i; j++)
			{
				if (arr[i] == arr[j])
				{
					arr.remove(i);
					break;
				}
			}
		}
	}

	/**
	 * \brief Non-owning reference to a contiguous sequence of elements
	 * \tparam T type of elements stored in the array
	 */
	template <typename T>
	struct AxArrayRef : public AxManagedClass
	{
		T* data {};
		size_t count {};

		[[nodiscard]] auto operator[](size_t index) -> T&
		{
			axAssert(index < count);
			return data[index];
		}

		[[nodiscard]] auto operator[](size_t index) const -> T const&
		{
			return const_cast<AxArrayRef* const>(this)->operator[](index);
		}
	};

}
