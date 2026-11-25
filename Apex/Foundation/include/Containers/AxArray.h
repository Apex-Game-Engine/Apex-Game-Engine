#pragma once
#include "ContainerUtils.h"
#include "Core/Asserts.h"
#include "Core/Utility.h"
#include "Memory/MemoryManager.h"

namespace apex {

	enum StorageType { Dynamic, Fixed, Static };

	 /// \brief Container of contiguous elements of the same type
	 /// \details Defines common functionality across AxArray, AxFixedArray, AxStaticArray
	 /// \tparam T Type of elements stored in the array
	template <typename T, StorageType kStorageType>
	class AxArrayBase
	{
		using TypeHelper = AxTypeHelper<T>;

	public:
		using size_type = u32;
		using value_type = TypeHelper::value_type;
		using pointer = TypeHelper::pointer;
		using const_pointer = TypeHelper::const_pointer;
		using reference = TypeHelper::reference;
		using const_reference = TypeHelper::const_reference;

		// Iterator class for AxArray
		template <typename IterType>
		class Iterator
		{
		public:
			using iterator_category = std::contiguous_iterator_tag;
			using value_type        = IterType;
			using difference_type   = ptrdiff_t;
			using pointer           = std::conditional_t<std::is_const_v<IterType>, AxArrayBase::const_pointer, AxArrayBase::pointer>;
			using reference         = std::conditional_t<std::is_const_v<IterType>, AxArrayBase::const_reference, AxArrayBase::reference>;

			Iterator() = default;
			Iterator(IterType* ptr) : m_ptr(ptr) {}

			Iterator& operator++() { ++m_ptr; return *this; } // pre-increment
			Iterator operator++(int) { Iterator tmp(*this); operator++(); return tmp; } // post-increment
			Iterator& operator--() { --m_ptr; return *this; } // pre-decrement
			Iterator operator--(int) { Iterator tmp(*this); operator--(); return tmp; } // post-decrement

			Iterator operator+(difference_type size) const { return { m_ptr + size }; } // TODO: add an assert to check for overflow?
			Iterator operator-(difference_type size) const { return { m_ptr - size }; } // TODO: add an assert to check for overflow?
			Iterator& operator+=(difference_type size) { m_ptr += size; return *this; }
			Iterator& operator-=(difference_type size) { m_ptr -= size; return *this; }

			bool operator==(const Iterator& rhs) const { return m_ptr == rhs.m_ptr; }
			bool operator!=(const Iterator& rhs) const { return m_ptr != rhs.m_ptr; }
			bool operator<(const Iterator& rhs) const { return m_ptr < rhs.m_ptr; }
			bool operator>(const Iterator& rhs) const { return m_ptr > rhs.m_ptr; }

			difference_type operator-(const Iterator& rhs) const { return m_ptr - rhs.m_ptr; }

			reference operator*() const { return *m_ptr; }
			pointer operator->() const { return m_ptr; }

			reference operator[](difference_type offset) { return *(m_ptr + offset); }

		private:
			IterType* m_ptr {};
		};

	public:
		using iterator = Iterator<T>;
		using const_iterator = Iterator<const T>;

		AxArrayBase() = default;

		explicit AxArrayBase(size_type capacity)
		{
			reserve(capacity);
		}

		AxArrayBase(AxArrayBase&& other) noexcept
		: m_capacity(std::move(other.m_capacity))
		, m_size(std::move(other.m_size))
		, m_data(std::move(other.m_data))
		{
			other.m_data = nullptr;
			other.m_capacity = 0;
			other.m_size = 0;
		}

		AxArrayBase(AxArrayBase const& other) noexcept
		: m_capacity(other.m_capacity)
		, m_size(other.m_size)
		{
			Allocate(m_capacity, m_size);
			apex::memcpy_s<value_type>(m_data, m_size, other.m_data, other.m_size);
		}

		AxArrayBase(std::initializer_list<value_type> init_list)
		: m_capacity(init_list.size())
		, m_size(init_list.size())
		{
			Allocate(m_capacity, m_size);
			apex::memcpy_s<value_type>(m_data, m_size, init_list.begin(), init_list.size());
		}

		~AxArrayBase()
		{
			DestroyAll();
			Deallocate();
		}

		AxArrayBase& operator=(AxArrayBase&& other) noexcept
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

		AxArrayBase& operator=(AxArrayBase const& other) noexcept
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
		void resize(size_type new_size, Args&&... args)
		{
			reserve(new_size);

			size_type oldSize = m_size;
			m_size = new_size;

			if (new_size > oldSize)
			{
				Fill(m_data + oldSize, m_data + new_size, std::forward<Args>(args)...);
			}
		}

		void reserve(size_type capacity)
		{
			if (m_capacity < capacity)
			{
				ReallocateAndMove(capacity);
			}
		}

		template <typename... Args>
		reference emplace_back(Args&&... args)
		{
			GrowIfDynamic();
			ConstructInPlace(&m_data[m_size], std::forward<Args>(args)...);
			return m_data[m_size++];
		}

		template <typename... Args>
		reference emplace(size_type index, Args&&... args)
		{
			axAssert(index < m_size + 1 && index < m_capacity);
			GrowIfDynamic();
			// shift elements to the right from index upto size
			ShiftElementsRight(index);
			ConstructInPlace(&m_data[index], std::forward<Args>(args)...);
			m_size++;
			return m_data[index];
		}

		void append(const value_type& obj)
		{
			GrowIfDynamic();
			m_data[m_size++] = obj;
		}

		void append(value_type&& obj)
		{
			GrowIfDynamic();
			m_data[m_size++] = std::move(obj);
		}

		void pop_back()
		{
			axAssertFmt(m_size > 0, "Cannot pop from empty array!");
			DestroyInPlace(m_data + m_size - 1);
			m_size--;
		}

		void insert(size_type index, const value_type& obj)
		{
			axAssertFmt(index < m_size + 1 && index < m_capacity, "Array insertion index in out of range!");
			GrowIfDynamic();
			// shift elements to the right from index upto size
			ShiftElementsRight(index);
			m_data[index] = obj;
			m_size++;
		}

		void insert(size_type index, value_type&& obj)
		{
			axAssertFmt(index < m_size + 1 && index < m_capacity, "Array insertion index in out of range!");
			GrowIfDynamic();
			// shift elements to the right from index upto size
			ShiftElementsRight(index);
			// append(std::move(obj));
			m_data[index] = std::move(obj);
			m_size++;
		}

		// void emplace(size_type index, )

		void remove(size_type index)
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
			DestroyAll();
			Deallocate();
			m_capacity = 0;
			m_size = 0;
		}

		void clear()
		{
			DestroyAll();
			m_size = 0;
		}

		[[nodiscard]] auto data() -> pointer              { return reinterpret_cast<pointer>(m_data); }
		[[nodiscard]] auto data() const -> const_pointer  { return const_cast<AxArrayBase*>(this)->data(); }
		[[nodiscard]] auto dataMutable() const -> pointer { return const_cast<AxArrayBase*>(this)->data(); }

		[[nodiscard]] auto back() -> reference             { axAssert(m_size > 0); return m_data[m_size - 1]; }
		[[nodiscard]] auto back() const -> const_reference { return const_cast<AxArrayBase*>(this)->back(); }

		[[nodiscard]] auto front() -> reference             { axAssert(m_size > 0); return m_data[0]; }
		[[nodiscard]] auto front() const -> const_reference { return const_cast<AxArrayBase*>(this)->front(); }

		[[nodiscard]] auto at(size_type index) -> reference             { return this->operator[](index); }
		[[nodiscard]] auto at(size_type index) const -> const_reference { return const_cast<AxArrayBase*>(this)->at(index); }

		[[nodiscard]] auto operator[](size_type index) -> reference             { axAssert(index < m_size); return m_data[index]; }
		[[nodiscard]] auto operator[](size_type index) const -> const_reference { return const_cast<AxArrayBase*>(this)->operator[](index); }
		
		[[nodiscard]] size_type size() const     { return m_size; }
		[[nodiscard]] size_type capacity() const { return m_capacity; }
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

		void Allocate(size_type capacity, size_type init_size)
		{
			size_t allocSize = sizeof(T) * capacity;
			void* ptr = mem::MemoryManager::allocate(&allocSize);
			m_capacity = static_cast<u32>(allocSize / sizeof(T));
			m_size = init_size;
			m_data = static_cast<T*>(ptr);
		}

		void Deallocate()
		{
			mem::MemoryManager::free(m_data);
			m_data = nullptr;
		}

		void ReallocateAndMove(size_type new_capacity)
		{
			auto oldData = (T*)m_data;
			auto oldSize = m_size;
			auto oldCapacity = m_capacity;

			Allocate(new_capacity, oldSize);

			if (oldSize > 0) {
				// MoveElementsFrom(oldData, oldSize);
				MoveConstructFrom(oldData, oldSize);
			}

			mem::MemoryManager::free(oldData);
		}

		void DestroyInPlace(T* ptr)
		{
			if constexpr (!std::is_trivially_destructible_v<T>)
				std::destroy_at(ptr);
		}

		void DestroyAll()
		{
			if constexpr (!std::is_trivially_destructible_v<T>)
				for (size_type i = 0; i < m_size; ++i)
				{
					DestroyInPlace(&m_data[m_size - i - 1]);
				}
		}

		void ShiftElementsRight(size_type start, size_type shift = 1)
		{
			for (size_type i = 0; i < m_size - start; i++)
			{
				size_type dst = m_size + shift - i - 1;
				size_type src = m_size - i - 1;
				if (dst >= m_size)
				{
					ConstructInPlace(&m_data[dst], std::move(m_data[src]));
				}
				else
				{
					m_data[dst] = std::move(m_data[src]);
				}
			}
			// apex::memmove_s<ElemType>(&m_data[index+1], m_capacity - index, &m_data[index], m_size - index);
		}

		void ShiftElementsLeft(size_type start, size_type shift = 1)
		{
			for (size_type i = 0; i < m_size - start; i++)
			{
				size_type dst = start + i;
				size_type src = start + shift + i;
				m_data[dst] = std::move(m_data[src]);
			}
			// apex::memmove_s<ElemType>(&m_data[index], moveNum, &m_data[index+1], moveNum);
		}

		template <typename... Args>
		void Fill(T* first, T* last, Args&&... args) requires (std::is_constructible_v<T, Args...>)
		{
			T* ptr = first;
			for (; ptr != last; ++ptr)
			{
				ConstructInPlace(ptr, std::forward<Args>(args)...);
			}
		}

		void MoveConstructFrom(T* src, size_type srcCount)
		{
			const size_type count = std::min(srcCount, m_capacity);
			for (size_type i = 0; i < count; i++)
			{
				ConstructInPlace(&m_data[i], std::move(src[i]));
			}
		}

		void MoveElementsFrom(T* src, size_type srcCount)
		{
			const size_type count = std::min(srcCount, m_capacity);
			for (size_type i = 0; i < count; i++)
			{
				m_data[i] = std::move(src[i]);
			}
		}

		void GrowIfDynamic()
		{
			if constexpr (kStorageType == Dynamic)
			{
				if (m_size == m_capacity)
				{
					reserve(m_capacity == 0 ? 2 : m_capacity * 2);
				}
			}
			else
			{
				axAssertFmt(m_size < m_capacity, "Array size exceeds capacity!");
			}
		}

	protected:
		size_type m_capacity { 0 };
		size_type m_size { 0 };

		T* m_data { nullptr };
		
		friend class AxArrayTest;
	};

	template <typename T> using AxArray = AxArrayBase<T, Dynamic>;

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
	class AxSpan
	{
		using TypeHelper = AxTypeHelper<T>;

	public:
		using size_type = u32;
		using value_type = TypeHelper::value_type;
		using pointer = TypeHelper::pointer;
		using const_pointer = TypeHelper::const_pointer;
		using reference = TypeHelper::reference;
		using const_reference = TypeHelper::const_reference;

		constexpr AxSpan() = default;

		constexpr AxSpan(T* data, size_type count)
			: m_data(data), m_count(count)
		{
		}

		[[nodiscard]] T& operator[](size_type index)
		{
			axAssert(index < m_count);
			return m_data[index];
		}

		[[nodiscard]] const T& operator[](size_type index) const
		{
			return const_cast<AxSpan* const>(this)->operator[](index);
		}

		[[nodiscard]] auto data() -> pointer { return m_data; }
		[[nodiscard]] auto data() const -> T const* { return m_data; }

		[[nodiscard]] auto size() const -> size_type { return m_count; }

		[[nodiscard]] auto begin() -> T* { return m_data; }
		[[nodiscard]] auto end() -> T* { return m_data + m_count; }

		[[nodiscard]] auto begin() const -> T const* { return m_data; }
		[[nodiscard]] auto end() const -> T const* { return m_data + m_count; }

	private:
		T* m_data {};
		size_type m_count {};
	};

	template <typename T>
	auto make_span(void* data, size_t count) -> AxSpan<T>
	{
		return { static_cast<T*>(data), count };
	}

	template <typename T>
	auto make_span(T* data, size_t count) -> AxSpan<T>
	{
		return { data, count };
	}

	template <typename T, size_t Size>
	auto make_span(T const(&arr)[Size]) -> AxSpan<const T>
	{
		return { arr, Size };
	}

	template <typename T, size_t Size>
	auto make_span(T (&arr)[Size]) -> AxSpan<T>
	{
		return { arr, Size };
	}

	template <typename T>
	auto make_span(AxArray<T>& arr) -> AxSpan<T>
	{
		return { arr.data(), arr.size() };
	}

	template <typename T>
	auto make_span(AxArray<T> const& arr) -> AxSpan<const T>
	{
		return { arr.data(), arr.size() };
	}


	/**
	 * \brief Fixed size array
	 * \tparam T type of elements stored in the array
	 * \tparam Size number of elements in the array
	 */
	template <typename T, size_t Size>
	struct AxStaticArray : public AxTypeHelper<T>
	{
		using size_type = u32;

		T m_data[Size];
		size_type m_size = 0;

		[[nodiscard]] auto operator[](size_type index) -> T&
		{
			axAssert(index < Size);
			return m_data[index];
		}

		[[nodiscard]] auto operator[](size_type index) const -> T const&
		{
			return const_cast<AxStaticArray* const>(this)->operator[](index);
		}

		void SetSize(size_type newSize) { axAssert(newSize <= Size); m_size = newSize; }
		void clear() { m_size = 0; }

		[[nodiscard]] auto size() const -> size_type { return m_size; }
		[[nodiscard]] auto capacity() const -> size_type { return Size; }

		[[nodiscard]] auto data() -> T* { return m_data; }
		[[nodiscard]] auto data() const -> T const* { return m_data; }

		[[nodiscard]] auto begin() -> T* { return m_data; }
		[[nodiscard]] auto end() -> T* { return m_data + Size; }

		[[nodiscard]] auto begin() const -> T const* { return m_data; }
		[[nodiscard]] auto end() const -> T const* { return m_data + Size; }

		[[nodiscard]] auto cbegin() const -> T const* { return m_data; }
		[[nodiscard]] auto cend() const -> T const* { return m_data + Size; }

		[[nodiscard]] auto getRef() -> AxSpan<T>
		{
			return make_span(m_data);
		}

		[[nodiscard]] auto getRef() const -> AxSpan<const T>
		{
			return make_span(m_data);
		}
	};

	template <typename T, size_t Size>
	auto make_span(AxStaticArray<T, Size>& arr) -> AxSpan<T>
	{
		return { arr.data(), arr.size() };
	}

	template <typename T, size_t Size>
	auto make_span(AxStaticArray<T, Size> const& arr) -> AxSpan<const T>
	{
		return { arr.data(), arr.size() };
	}


	using ByteArray = AxArray<byte>;
	using ByteArrayRef = AxSpan<byte>;

}
