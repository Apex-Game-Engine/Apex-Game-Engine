﻿#pragma once
#include "Core/Asserts.h"
#include "Math/Utility.h"
#include "Memory/AxHandle.h"
#include "Memory/AxManagedClass.h"

namespace apex {

	/**
	 * \brief Non-resizable dynamic array
	 * \tparam T type of elements stored in the array
	 */
	template <typename T, typename = SelfManaged>
	class AxArray : public AxManagedClass
	{
	public:
		template <typename IterType>
		class Iterator
		{
		public:
			using iterator_category = std::contiguous_iterator_tag;
			using value_type        = IterType;
			using difference_type   = ptrdiff_t;
			using pointer           = IterType*;
			using reference         = IterType&;

			Iterator() : m_ptr() {}
			Iterator(IterType* ptr) : m_ptr(ptr) {}
			Iterator& operator++() { ++m_ptr; return *this; } // pre-increment
			Iterator operator++(int) { Iterator tmp(*this); operator++(); return tmp; } // post-increment
			bool operator==(const Iterator& rhs) const { return m_ptr == rhs.m_ptr; }
			bool operator!=(const Iterator& rhs) const { return m_ptr != rhs.m_ptr; }
			reference operator*() { return *m_ptr; }

		private:
			IterType* m_ptr;
		};

	protected:
		template <typename... Args>
		[[deprecated("Do not allocate memory on your own. If you need to allocate static memory and instantiate an array on it then use the AxStaticArray.")]]
		void constructFromMemory(void* mem, size_t mem_size, Args&&... args) requires (std::is_constructible_v<T, Args...>)
		{
			m_capacity = mem_size / sizeof(value_type);
			m_size = 0;
			m_data = new (mem) stored_type[m_capacity](std::forward<Args>(args)...);
		}

		[[deprecated("Do not allocate memory on your own. If you need to allocate static memory and instantiate an array on it then use the AxStaticArray.")]]
		void constructFromMemory(void* mem, size_t mem_size) requires (!std::is_default_constructible_v<T>)
		{
			m_capacity = mem_size / sizeof(value_type);
			m_size = 0;
			m_data = static_cast<stored_type*>(mem);
		}

		template <typename... Args>
		void fill(Args&&... args) requires (std::is_constructible_v<T, Args...>)
		{
			new (m_data) stored_type[m_size](std::forward<Args>(args)...);
		}

		void defaultInit()
		{
			if constexpr (std::is_trivially_default_constructible_v<value_type>)
			{
				fill();
			}
		}

		explicit AxArray(AxHandle& handle)
		{
			constructFromMemory(handle.getAs<uint8>() + sizeof(AxArray), handle.getBlockSize() - sizeof(AxArray));
		}

	public:
		using stored_type = std::conditional_t<apex::managed_class<T>, T, AxManagedClassAdapter<T>>;
		using value_type = T;
		using pointer = value_type*;
		using const_pointer = const value_type*;
		using reference = value_type&;
		using const_reference = const value_type&;
		using iterator = Iterator<stored_type>;
		using const_iterator = Iterator<const stored_type>;

		static_assert(sizeof(stored_type) == sizeof(value_type));

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
		}

		~AxArray()
		{
			delete[] m_data; // TODO: Change m_data to UniquePtr and remove this insanity from here
		}

		AxArray& operator=(AxArray&& other) noexcept
		{
			m_capacity = std::move(other.m_capacity);
			m_size = std::move(other.m_size);
			m_data = std::move(other.m_data);

			return *this;
		}

		AxArray(std::initializer_list<value_type> init_list)
		: m_capacity(init_list.size())
		, m_size(init_list.size())
		{
			AxHandle handle(sizeof(value_type) * m_capacity);
			m_data = handle.getAs<stored_type>();
			memcpy_s(m_data, m_size * sizeof(value_type), init_list.begin(), init_list.size() * sizeof(typename decltype(init_list)::value_type));
		}

		// TODO: Make the destructor free the memory. Aim to make AxHandle internal use only and hide most of its (potentially dangerous) capabilities


		/*template <typename... Args>
		void resizeToHandle(AxHandle& handle, Args&&... args) requires (std::is_constructible_v<T, Args...>)
		{
			delete[] m_data;

			m_capacity = handle.getBlockSize() / sizeof(value_type);
			m_size = m_capacity;
			m_data = new (handle) stored_type[m_capacity](std::forward<Args>(args)...);
		}*/

		template <typename... Args>
		void resize(size_t new_size, Args&&... args)
		{
			if (new_size > m_capacity)
			{
				// _resize();
			}
			m_size = new_size;
			fill(std::forward<Args>(args)...);
		}

		void setDataHandle(AxHandle& handle)
		{
			m_capacity = handle.getBlockSize() / sizeof(value_type);
			m_size = 0;
			m_data = handle.getAs<stored_type>();
		}

		void reserve(size_t capacity)
		{
			AxHandle handle (sizeof(value_type) * capacity);
			setDataHandle(handle);
		}
		
		static size_t totalRequiredMemory(size_t capacity)
		{
			return sizeof(AxArray) + sizeof(value_type) * capacity;
		}

		template <typename... Args>
		reference emplace_back(Args&&... args)
		{
			new ((void*)&m_data[m_size]) value_type(std::forward<Args>(args)...);
			return m_data[m_size++];
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
			m_size--;
		}

		void insert(const value_type& obj, size_t index)
		{
			axAssert(index < m_size + 1 && index < m_capacity);
			axAssertMsg(m_size < m_capacity, "Array size exceeds capacity!");
			apex::memmove_s<stored_type>(&m_data[index+1], m_capacity - index, &m_data[index], m_size - index);
			m_data[index] = obj;
			m_size++;
		}

		void insert(value_type&& obj, size_t index)
		{
			insert(std::forward<value_type>(obj), index);
		}

		void remove(size_t index)
		{
			axAssert(index < m_size);
			const size_t moveNum = m_size - index;
			if (index < m_size - 1)
				apex::memmove_s<stored_type>(&m_data[index], moveNum, &m_data[index+1], moveNum);
			m_size--;
		}

		void reset()
		{
			m_capacity = 0;
			m_size = 0;
		}

		void clear()
		{
			m_size = 0;
		}

		[[nodiscard]] auto data() { return m_data; }

		[[nodiscard]] auto at(size_t index) { return this->operator[](index); }
		[[nodiscard]] auto at(size_t index) const { return this->operator[](index); }

		[[nodiscard]] auto operator[](size_t index) -> reference
		{
			axAssert(index < m_size);

			if constexpr (apex::is_managed_adapted_class_v<stored_type>)
			{
				return m_data[index].value();
			}
			else
			{
				return m_data[index];
			}
		}

		[[nodiscard]] auto operator[] (size_t index) const -> std::conditional_t<std::is_trivial_v<T>, value_type, const_reference>
		{
			axAssert(index < m_size);
			return m_data[index];
		}
		
		[[nodiscard]] size_t size() const { return m_size; }
		[[nodiscard]] size_t capacity() const { return m_capacity; }

	#pragma region Iterator functions
		[[nodiscard]] iterator begin() { return iterator(m_data); }
		[[nodiscard]] iterator end() { return iterator(m_data + m_size); }

		[[nodiscard]] const_iterator cbegin() const { return const_iterator(m_data); }
		[[nodiscard]] const_iterator cend() const { return const_iterator(m_data + m_size); }

		[[nodiscard]] const_iterator begin() const { return cbegin(); }
		[[nodiscard]] const_iterator end() const { return cend(); }
	#pragma endregion

	private:
		size_t m_capacity { 0 };
		size_t m_size { 0 };

		// TODO: Change this to AxHandle or UniquePtr
		stored_type *m_data { nullptr };

		friend class AxArrayTest;
	};

	/**
	 * \brief Non-resizable dynamic array that does not guarantee order of elements
	 *
	 *  - insertions and deletions in middle of array do not preserve ordering.
	 *	- append and pop from end preserve ordering.
	 *	\n For ordered insertions and deletions, use `AxArray`
	 * \tparam StoredType type of elements stored in the array
	 */
	template <typename StoredType>
	class AxUnorderedArray
	{
	public:
		template <typename IterType>
		class Iterator
		{
		public:
			using iterator_category = std::contiguous_iterator_tag;
			using value_type        = IterType;
			using difference_type   = ptrdiff_t;
			using pointer           = IterType*;
			using reference         = IterType&;

			Iterator(IterType* ptr) : m_ptr(ptr) {}
			Iterator& operator++() { ++m_ptr; return *this; } // pre-increment
			Iterator operator++(int) { Iterator tmp(*this); operator++(); return tmp; } // post-increment
			bool operator==(const Iterator& rhs) const { return m_ptr == rhs.m_ptr; }
			bool operator!=(const Iterator& rhs) const { return m_ptr != rhs.m_ptr; }
			reference operator*() { return *m_ptr; }

		private:
			IterType* m_ptr;
		};

	public:
		using value_type = StoredType;
		using pointer = value_type*;
		using const_pointer = const value_type*;
		using reference = value_type&;
		using const_reference = const value_type&;
		using iterator = Iterator<StoredType>;
		using const_iterator = Iterator<const StoredType>;

		AxUnorderedArray(void* memory, size_t capacity)
		: m_data(static_cast<pointer>(memory))
		, m_capacity(capacity)
		, m_size(0)
		{
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
			m_size--;
		}

		void remove(size_t index)
		{
			axAssert(index < m_size);
			if (index < m_size - 1)
			{
				apex::memmove_s<value_type>(&m_data[index], 1, &m_data[m_size - 1], 1);
			}
			m_size--;
		}

		[[nodiscard]] auto at(size_t index) { return this->operator[](index); }
		[[nodiscard]] auto at(size_t index) const { return this->operator[](index); }

		[[nodiscard]] auto operator[](size_t index) -> reference
		{
			axAssert(index < m_size);
			return m_data[index];
		}

		[[nodiscard]] auto operator[] (size_t index) const -> std::conditional_t<std::is_trivial_v<StoredType>, value_type, const_reference>
		{
			axAssert(index < m_size);
			return m_data[index];
		}
		
		[[nodiscard]] size_t size() const { return m_size; }
		[[nodiscard]] size_t capacity() const { return m_capacity; }

	#pragma region Iterator functions
		[[nodiscard]] iterator begin() { return iterator(m_data); }
		[[nodiscard]] iterator end() { return iterator(m_data + m_size); }

		[[nodiscard]] const_iterator cbegin() const { return const_iterator(m_data); }
		[[nodiscard]] const_iterator cend() const { return const_iterator(m_data + m_size); }

		[[nodiscard]] const_iterator begin() const { return cbegin(); }
		[[nodiscard]] const_iterator end() const { return cend(); }
	#pragma endregion

	private:
		value_type *m_data;
		size_t m_capacity;
		size_t m_size;

		friend class AxUnorderedArrayTest;
	};


}
