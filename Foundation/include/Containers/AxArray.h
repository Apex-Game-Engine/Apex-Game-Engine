﻿#pragma once
#include "Core/Asserts.h"
#include "Math/Utility.h"
#include "Memory/AxHandle.h"
#include "Memory/AxManagedClass.h"
#include "Memory/UniquePtr.h"

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
			reference operator*() const { return *m_ptr; }
			pointer operator->() const { return m_ptr; }

		private:
			IterType* m_ptr;
		};

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
			other.m_data = nullptr;
		}

		AxArray(std::initializer_list<value_type> init_list)
		: m_capacity(init_list.size())
		, m_size(init_list.size())
		{
			AxHandle handle(sizeof(value_type) * m_capacity);
			setDataHandle(handle, m_size);
			apex::memcpy_s<value_type>(m_data, m_size, init_list.begin(), init_list.size());
		}

		~AxArray()
		{
			delete[] m_data;
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
			size_t oldSize = m_size;
			if (new_size > m_capacity)
			{
				_ReallocateAndMove(new_size);
			}
			m_size = new_size;
			fill(m_data + oldSize, m_data + new_size, std::forward<Args>(args)...);
		}

		void reserve(size_t capacity)
		{
			AxHandle handle (sizeof(value_type) * capacity);
			setDataHandle(handle, 0);
		}

		template <typename... Args>
		reference emplace_back(Args&&... args)
		{
			_ConstructInPlace(&m_data[m_size], std::forward<Args>(args)...);
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
			_DestroyInPlace(m_data + m_size - 1);
			m_size--;
		}

		void insert(const value_type& obj, size_t index)
		{
			axAssert(index < m_size + 1 && index < m_capacity);
			axAssertMsg(m_size < m_capacity, "Array size exceeds capacity!");
			// shift elements to the right from index upto size
			_ShiftElementsRight(index);
			append(obj);
		}

		void insert(value_type&& obj, size_t index)
		{
			axAssert(index < m_size + 1 && index < m_capacity);
			axAssertMsg(m_size < m_capacity, "Array size exceeds capacity!");
			// shift elements to the right from index upto size
			_ShiftElementsRight(index);
			append(std::move(obj));
		}

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

	protected:
		template <typename... Args>
		stored_type* _ConstructInPlace(stored_type* mem, Args&&... args) requires (std::is_constructible_v<T, Args...>)
		{
			return std::construct_at(mem, std::forward<Args>(args)...);
		}

		void _ReallocateAndMove(size_t new_capacity)
		{
			AxHandle newHandle (sizeof(value_type) * new_capacity);
			auto oldData = m_data;
			auto oldSize = m_size;
			auto oldCapacity = m_capacity;

			setDataHandle(newHandle, oldSize);

			apex::memmove_s<stored_type>(m_data, m_capacity, oldData, oldSize);

			delete oldData;
		}

		void _DestroyInPlace(stored_type* ptr)
		{
			std::destroy_at(ptr);
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

		template <typename... Args>
		void fill(stored_type* first, stored_type* last, Args&&... args) requires (std::is_constructible_v<T, Args...>)
		{
			stored_type* ptr = first;
			for (; ptr != last; ++ptr)
			{
				_ConstructInPlace(ptr, std::forward<Args>(args)...);
			}
		}

		void setDataHandle(AxHandle& handle, size_t init_size)
		{
			m_capacity = handle.getBlockSize() / sizeof(value_type);
			m_size = init_size;
			m_data = handle.getAs<stored_type>();
		}

	private:
		size_t m_capacity { 0 };
		size_t m_size { 0 };

		stored_type* m_data { nullptr };

		friend class AxArrayTest;
	};

}
