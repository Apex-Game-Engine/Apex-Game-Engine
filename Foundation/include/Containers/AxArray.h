#pragma once
#include "Core/Asserts.h"
#include "Core/TypeTraits.h"
#include "Core/Utility.h"
#include "Memory/AxHandle.h"
#include "Memory/UniquePtr.h"

namespace apex {

	enum StorageType { Dynamic, Fixed, Static };

	/**
	 * \brief Container of contiguous elements of the same type
	 * Defines common functionality across AxArray, AxDynamicArray, AxStaticArray
	 * \tparam T Type of elements stored in the array
	 */
	template <typename T, StorageType >
	class AxBaseArray
	{
		using ElemType = T;

	public:
		using value_type = T;
		using pointer = value_type*;
		using const_pointer = const value_type*;
		using reference = value_type&;
		using const_reference = const value_type&;

		// Iterator class for AxArray
		template <typename IterType>
		class Iterator
		{
		public:
			using iterator_category = std::contiguous_iterator_tag;
			using value_type        = IterType;
			using difference_type   = ptrdiff_t;
			using pointer           = std::conditional_t<std::is_const_v<IterType>, const ElemType*, ElemType*>;
			using reference         = std::conditional_t<std::is_const_v<IterType>, const ElemType&, ElemType&>;;

			Iterator() = default;
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

			reference operator*() const { return *m_ptr; }
			pointer operator->() const { return m_ptr; }

		private:
			IterType* m_ptr {};
		};

	public:
		using iterator = Iterator<T>;
		using const_iterator = Iterator<const T>;

		AxBaseArray() = default;

		explicit AxBaseArray(size_t capacity)
		{
			reserve(capacity);
		}

		AxBaseArray(AxBaseArray&& other) noexcept
		: m_capacity(std::move(other.m_capacity))
		, m_size(std::move(other.m_size))
		, m_data(std::move(other.m_data))
		{
			other.m_data = nullptr;
			other.m_capacity = 0;
			other.m_size = 0;
		}

		AxBaseArray(AxBaseArray const& other) noexcept
		: m_capacity(other.m_capacity)
		, m_size(other.m_size)
		{
			Allocate(m_capacity, m_size);
			apex::memcpy_s<value_type>(m_data, m_size, other.m_data, other.m_size);
		}

		AxBaseArray(std::initializer_list<value_type> init_list)
		: m_capacity(init_list.size())
		, m_size(init_list.size())
		{
			Allocate(m_capacity, m_size);
			apex::memcpy_s<value_type>(m_data, m_size, init_list.begin(), init_list.size());
		}

		~AxBaseArray()
		{
			DestroyAll();
			delete m_data;
		}

		AxBaseArray& operator=(AxBaseArray&& other) noexcept
		{
			if (this != &other)
			{
				m_capacity = std::move(other.m_capacity);
				m_size = std::move(other.m_size);
				m_data = std::move(other.m_data);

				other.m_data = nullptr;
				other.m_capacity = 0;
				other.m_size = 0;
			}
			return *this;
		}

		AxBaseArray& operator=(AxBaseArray const& other) noexcept
		{
			if (this != &other)
			{
				m_capacity = other.m_capacity;
				m_size = other.m_size;
				Allocate(m_capacity, m_size);
				apex::memcpy_s<value_type>(m_data, m_size, other.m_data, other.m_size);
			}
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
					Fill(m_data + oldSize, m_data + new_size, std::forward<Args>(args)...);
				}
			}
		}

		void reserve(size_t capacity)
		{
			if (m_capacity < capacity)
			{
				ReallocateAndMove(capacity);
			}
		}

		template <typename... Args>
		reference emplace_back(Args&&... args)
		{
			axAssertFmt(m_size < m_capacity, "Array size exceeds capacity!");
			ConstructInPlace(&m_data[m_size], std::forward<Args>(args)...);
			return m_data[m_size++];
		}

		template <typename... Args>
		reference emplace(size_t index, Args&&... args)
		{
			axAssert(index < m_size + 1 && index < m_capacity);
			axAssertFmt(m_size < m_capacity, "Array size exceeds capacity!");
			// shift elements to the right from index upto size
			ShiftElementsRight(index);
			ConstructInPlace(&m_data[index], std::forward<Args>(args)...);
			m_size++;
			return m_data[index];
		}

		void append(const value_type& obj)
		{
			axAssertFmt(m_size < m_capacity, "Array size exceeds capacity!");
			m_data[m_size++] = obj;
		}

		void append(value_type&& obj)
		{
			axAssertFmt(m_size < m_capacity, "Array size exceeds capacity!");
			m_data[m_size++] = std::move(obj);
		}

		void pop_back()
		{
			axAssertFmt(m_size > 0, "Cannot pop from empty array!");
			DestroyInPlace(m_data + m_size - 1);
			m_size--;
		}

		void insert(size_t index, const value_type& obj)
		{
			axAssertFmt(index < m_size + 1 && index < m_capacity, "Array insertion index in out of range!");
			axAssertFmt(m_size < m_capacity, "Array size exceeds capacity!");
			// shift elements to the right from index upto size
			ShiftElementsRight(index);
			m_data[index] = obj;
			m_size++;
		}

		void insert(size_t index, value_type&& obj)
		{
			axAssertFmt(index < m_size + 1 && index < m_capacity, "Array insertion index in out of range!");
			axAssertFmt(m_size < m_capacity, "Array size exceeds capacity!");
			// shift elements to the right from index upto size
			ShiftElementsRight(index);
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
				ShiftElementsLeft(index);
			// pop last element
			pop_back();
		}

		void reset()
		{
			m_capacity = 0;
			m_size = 0;
			DestroyAll();
		}

		void clear()
		{
			m_size = 0;
			DestroyAll();
		}

		[[nodiscard]] auto data() -> pointer              { return m_data; }
		[[nodiscard]] auto data() const -> const_pointer  { return const_cast<AxBaseArray*>(this)->data(); }
		[[nodiscard]] auto dataMutable() const -> pointer { return const_cast<AxBaseArray*>(this)->data(); }

		[[nodiscard]] auto back() -> reference             { axAssert(m_size > 0); return m_data[m_size - 1]; }
		[[nodiscard]] auto back() const -> const_reference { return const_cast<AxBaseArray*>(this)->back(); }

		[[nodiscard]] auto front() -> reference             { axAssert(m_size > 0); return m_data[0]; }
		[[nodiscard]] auto front() const -> const_reference { return const_cast<AxBaseArray*>(this)->front(); }

		[[nodiscard]] auto at(size_t index) -> reference             { return this->operator[](index); }
		[[nodiscard]] auto at(size_t index) const -> const_reference { return const_cast<AxBaseArray*>(this)->at(index); }

		[[nodiscard]] auto operator[](size_t index) -> reference             { axAssert(index < m_size); return m_data[index]; }
		[[nodiscard]] auto operator[](size_t index) const -> const_reference { return const_cast<AxBaseArray*>(this)->operator[](index); }
		
		[[nodiscard]] size_t size() const     { return m_size; }
		[[nodiscard]] size_t capacity() const { return m_capacity; }
		[[nodiscard]] bool   empty() const    { return m_size == 0; }

	#pragma region Iterator functions
		[[nodiscard]] iterator begin()              { return iterator(m_data); }
		[[nodiscard]] iterator end()                { return iterator(m_data + m_size); }
		[[nodiscard]] const_iterator begin() const  { return cbegin(); }
		[[nodiscard]] const_iterator end() const    { return cend(); }
		[[nodiscard]] const_iterator cbegin() const { return const_iterator(m_data); }
		[[nodiscard]] const_iterator cend() const   { return const_iterator(m_data + m_size); }
	#pragma endregion

	protected:
		template <typename... Args>
		T* ConstructInPlace(T* mem, Args&&... args) requires (std::is_constructible_v<T, Args...>)
		{
			return std::construct_at(mem, std::forward<Args>(args)...);
		}

		void Allocate(size_t capacity, size_t init_size)
		{
			AxHandle newHandle (sizeof(value_type) * capacity);
			//AxHandle newHandle = apex::make_handle<value_type[]>(capacity);
			SetDataHandle(newHandle, init_size);
		}

		void ReallocateAndMove(size_t new_capacity)
		{
			auto oldData = m_data;
			auto oldSize = m_size;
			auto oldCapacity = m_capacity;

			Allocate(new_capacity, oldSize);

			if (oldSize > 0)
				apex::memmove_s<T>(m_data, m_capacity, oldData, oldSize);

			delete oldData;
		}

		void DestroyInPlace(T* ptr)
		{
			if constexpr (!std::is_trivially_destructible_v<T>)
				std::destroy_at(ptr);
		}

		void DestroyAll()
		{
			if constexpr (!std::is_trivially_destructible_v<T>)
				for (size_t i = 0; i < m_size; ++i)
				{
					DestroyInPlace(&m_data[i]);
				}
		}

		void ShiftElementsRight(size_t index)
		{
			apex::memmove_s<ElemType>(&m_data[index+1], m_capacity - index, &m_data[index], m_size - index);
		}

		void ShiftElementsLeft(size_t index)
		{
			const size_t moveNum = m_size - index;
			apex::memmove_s<ElemType>(&m_data[index], moveNum, &m_data[index+1], moveNum);
		}

		template <typename... Args>
		void Fill(ElemType* first, ElemType* last, Args&&... args) requires (std::is_constructible_v<T, Args...>)
		{
			ElemType* ptr = first;
			for (; ptr != last; ++ptr)
			{
				ConstructInPlace(ptr, std::forward<Args>(args)...);
			}
		}

		void SetDataHandle(AxHandle& handle, size_t init_size)
		{
			m_capacity = handle.getBlockSize() / sizeof(value_type);
			m_size = init_size;
			m_data = handle.getAs<ElemType>();
		}

	protected:
		size_t m_capacity { 0 };
		size_t m_size { 0 };

		ElemType* m_data { nullptr };
		
		friend class AxArrayTest;
	};

	template <typename T> using AxArray = AxBaseArray<T, Dynamic>;

	// TODO: Implement AxArray<T, Static> and AxArray<T, Fixed>
	// Static: statically or stack allocated storage


	template <typename ArrayT>
	void keepUniquesOnly_slow(ArrayT& arr)
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
	struct AxArrayRef
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

		[[nodiscard]] auto size() const -> size_t { return count; }

		[[nodiscard]] auto begin() -> T* { return data; }
		[[nodiscard]] auto end() -> T* { return data + count; }

		[[nodiscard]] auto begin() const -> T const* { return data; }
		[[nodiscard]] auto end() const -> T const* { return data + count; }

		[[nodiscard]] auto cbegin() const -> T const* { return data; }
		[[nodiscard]] auto cend() const -> T const* { return data + count; }
	};

	template <typename T>
	auto make_array_ref(void* data, size_t count) -> AxArrayRef<T>
	{
		AxArrayRef<T> ref;
		ref.data = static_cast<T*>(data);
		ref.count = count;
		return ref;
	}

	template <typename T, size_t Size>
	auto make_array_ref(T const(&arr)[Size]) -> AxArrayRef<const T>
	{
		AxArrayRef<const T> ref;
		ref.data = arr;
		ref.count = Size;
		return ref;
	}

	template <typename T, size_t Size>
	auto make_array_ref(T (&arr)[Size]) -> AxArrayRef<T>
	{
		AxArrayRef<T> ref;
		ref.data = arr;
		ref.count = Size;
		return ref;
	}


	/**
	 * \brief Fixed size array
	 * \tparam T type of elements stored in the array
	 * \tparam Size number of elements in the array
	 */
	template <typename T, size_t Size>
	struct AxStaticArray
	{
		T data[Size];

		[[nodiscard]] auto operator[](size_t index) -> T&
		{
			axAssert(index < Size);
			return data[index];
		}

		[[nodiscard]] auto operator[](size_t index) const -> T const&
		{
			return const_cast<AxStaticArray* const>(this)->operator[](index);
		}

		[[nodiscard]] auto size() const -> size_t { return Size; }

		[[nodiscard]] auto begin() -> T* { return data; }
		[[nodiscard]] auto end() -> T* { return data + Size; }

		[[nodiscard]] auto begin() const -> T const* { return data; }
		[[nodiscard]] auto end() const -> T const* { return data + Size; }

		[[nodiscard]] auto cbegin() const -> T const* { return data; }
		[[nodiscard]] auto cend() const -> T const* { return data + Size; }

		[[nodiscard]] auto getRef() -> AxArrayRef<T>
		{
			return make_array_ref(data, Size);
		}

		[[nodiscard]] auto getRef() const -> AxArrayRef<const T>
		{
			return make_array_ref(data, Size);
		}
	};
}
