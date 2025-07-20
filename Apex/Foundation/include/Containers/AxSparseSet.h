#pragma once
#include "AxArray.h"
#include "AxRange.h"
#include "Core/Asserts.h"

#include <optional>

namespace apex {

	/**
	 * \brief Sparse set implementation.
	 * Check https://manenko.com/2021/05/23/sparse-sets.html for details on design.
	 * \tparam KeyType Type of keys that will be stored in the set.
	 */
	template <typename KeyType>
	class AxSparseSet
	{
	public:
		using key_type = KeyType;
		using key_pointer = KeyType*;
		using const_key_pointer = const KeyType*;
		using key_reference = KeyType&;
		using const_key_reference = const KeyType&;

		using sparse_array = AxArray<key_type>;
		using dense_array = AxArray<key_type>;

		/**
		 * \brief Default constructor. Creates empty set with zero capacity.
		 */
		AxSparseSet() = default;

		/**
		 * \brief Creates an empty set with initial capacity.
		 * \param capacity Initial capacity.
		 */
		explicit AxSparseSet(u32 capacity)
		: m_sparse(capacity)
		, m_dense(capacity)
		{
			m_sparse.resize(capacity);
		}

		/**
		 * \brief Default destructor.
		 */
		~AxSparseSet() = default;

		/**
		 * \brief Resizes set to new capacity.
		 * \param capacity New capacity to resize to.
		 */
		void reserve(size_t capacity)
		{
			// TODO: Do NOT resize dense array if not required
			m_sparse.reserve(capacity);
			m_dense.reserve(capacity);

			m_sparse.resize(m_sparse.capacity());
		}

		/**
		 * \brief Adds a new key to the set. Asserts on failure.
		 * \param key New key to add.
		 */
		void add(key_type key)
		{
			axAssert(key < capacity());
			axAssertFmt(!contains(key), "Cannot add element. ID already exists in the set!");

			Insert(key);
		}

		/**
		 * \brief Attempts to add a new key to the set.
		 * \param key New key to add.
		 * \return true if successful; false otherwise.
		 */
		bool try_add(key_type key)
		{
			axAssert(key < capacity());

			if (!contains(key))
			{
				Insert(key);
				return true;
			}

			return false;
		}

		/**
		 * \brief Removes a key from the set. Asserts on failure.
		 * \param key Key to remove.
		 */
		void remove(key_type key)
		{
			axAssert(key < capacity());
			axAssertFmt(contains(key), "Cannot delete element. ID does not exist!");

			Remove(key);
		}

		/**
		 * \brief Attempts to remove an existing key from the set.
		 * \param key Key to remove.
		 * \return true if successful; false otherwise.
		 */
		bool try_remove(key_type key)
		{
			axAssert(key < capacity());

			if (contains(key))
			{
				Remove(key);
				return true;
			}

			return false;
		}

		/**
		 * \brief Queries if the set contains the given key.
		 * \param key Key to query.
		 * \return true if key exists in the set; false otherwise.
		 */
		bool contains(key_type key) const
		{
			if (key < capacity())
			{
				key_type denseIdx = m_sparse[key];
				return denseIdx < m_dense.size() && m_dense[denseIdx] == key;
			}
			return false;
		}

		/**
		 * \brief Queries the set for the given key and returns its index.
		 * \param key Key to query.
		 * \return Index of the queried key.
		 */
		key_type getIndex(key_type key) const
		{
			axAssert(key < capacity());

			key_type denseIdx = m_sparse[key];
			axAssert(denseIdx < m_dense.size() && m_dense[denseIdx] == key);

			return denseIdx;
		}

		/**
		 * \brief Queries the set for the given key and returns its index if exists.
		 * \param key Key to query.
		 * \return Optional index of the queried key. std::nullopt if not found.
		 */
		auto try_getIndex(key_type key) const -> std::optional<key_type>
		{
			axAssert(key < capacity());

			key_type denseIdx = m_sparse[key];

			if (denseIdx < m_dense.size() && m_dense[denseIdx] == key)
			{
				return denseIdx;
			}

			return std::nullopt;
		}

		/**
		 * \brief Clears all values from the set.
		 */
		void clear()
		{
			m_dense.clear();
		}

		/**
		 * \brief Returns an iterable range of mutable keys in the set.
		 * \return An AxRange of keys.
		 */
		auto keys() { return ranges::AxRange<dense_array>(m_dense.begin(), m_dense.end()); }

		/**
		 * \brief Returns an iterable range of immutable keys in the set.
		 * \return An AxRange of const keys.
		 */
		auto keys() const { return ranges::AxRange<const dense_array, typename dense_array::const_iterator>(m_dense.begin(), m_dense.end()); }

		/**
		 * \brief Returns the currently allocated max capacity of the set.
		 * \return Current capacity.
		 */
		size_t capacity() const { return m_dense.capacity(); }

		/**
		 * \brief Returns the current count of keys present in the set.
		 * \return Current key count.
		 */
		size_t count() const { return m_dense.size(); }

	protected:
		void Insert(key_type key)
		{
			m_dense.append(key);
			m_sparse[key] = m_dense.size() - 1;
		}

		void Remove(key_type key)
		{
			auto newCount = m_dense.size() - 1;

			key_type denseIdx = m_sparse[key];
			key_type lastId = m_dense[newCount];
			m_dense[denseIdx] = lastId;
			m_sparse[lastId] = denseIdx;

			m_dense.pop_back();
		}

		key_type GetIndex(key_type key) // unsafe operation. Only for internal use
		{
			key_type denseIdx = m_sparse[key];
			return denseIdx;
		}

	private:
		sparse_array m_sparse{};
		dense_array m_dense{};
	};

}
