#pragma once
#include "AxArray.h"
#include "Core/Asserts.h"
#include "Memory/AxHandle.h"

namespace apex {

	template <typename Type>
	class AxSparseSet
	{
	public:
		using sparse_type = uint32;
		using dense_type = uint32;
		using element_type = Type;

		using sparse_array = AxArray<sparse_type>;
		using dense_array = AxArray<dense_type>;
		using element_array = AxArray<element_type>;


		explicit AxSparseSet(uint32 capacity)
		: m_capacity(capacity)
		, m_count(0)
		, m_sparse(capacity)
		, m_dense(capacity)
		, m_elements(capacity)
		{
			m_sparse.resize(capacity);
			m_dense.resize(capacity);
			m_elements.resize(capacity);
		}

		~AxSparseSet() = default;

		void insert(uint32 id, Type const& elem)
		{
			axAssert(id < m_capacity);
			axAssertMsg(!has(id), "Cannot insert element. ID already exists in the set!");

			_Insert(id, elem);
		}

		bool try_insert(uint32 id, Type const& elem)
		{
			axAssert(id < m_capacity);

			if (!has(id))
			{
				_Insert(id, elem);
				return true;
			}

			return false;
		}

		void remove(uint32 id)
		{
			axAssert(id < m_capacity);
			axAssertMsg(has(id), "Cannot delete element. ID does not exist!");

			_Remove(id);
		}

		bool try_remove(uint32 id)
		{
			axAssert(id < m_capacity);

			if (has(id))
			{
				_Remove(id);
				return true;
			}

			return false;
		}

		bool has(uint32 id)
		{
			axAssert(id < m_capacity);

			uint32 denseIdx = m_sparse[id];
			return denseIdx < m_count && m_dense[denseIdx] == id;
		}

		void clear()
		{
			m_count = 0;
		}

		auto elements() const { return ranges::AxRange<const element_array>(m_elements.begin(), m_elements.begin() + m_count); }
		auto ids() const { return ranges::AxRange<const dense_array>(m_dense.begin(), m_dense.begin() + m_count); }

		size_t capacity() const { return m_capacity; }
		size_t count() const { return m_count; }

	protected:
		void _Insert(uint32 id, Type const& elem)
		{
			m_dense[m_count] = id;
			m_sparse[id] = m_count;

			m_elements[m_count] = elem;

			++m_count;
		}

		void _Remove(uint32 id)
		{
			--m_count;

			uint32 denseIdx = m_sparse[id];
			uint32 lastId = m_dense[m_count];
			m_dense[denseIdx] = lastId;
			m_sparse[lastId] = denseIdx;

			m_elements[denseIdx] = m_elements[m_count];
		}

	private:
		uint32 m_capacity;
		uint32 m_count;
		sparse_array m_sparse;
		dense_array m_dense;
		element_array m_elements;
	};

}
