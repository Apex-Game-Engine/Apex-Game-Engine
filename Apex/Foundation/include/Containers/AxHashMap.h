#pragma once
#include <optional>

#include "AxArray.h"


namespace apex {

	template <
		typename KeyType, typename ValueType,
		typename Hasher = std::hash<KeyType>,
		typename Comparer = std::equal_to<KeyType>,
		KeyType kEmpty = KeyType{ 0 },
		KeyType kTombstone = KeyType{ -1 }>
	class AxHashMap
	{
	public:
		constexpr static size_t kInvalidIndex = -1;
		constexpr static float kDefaultMaxLoadFactor = 0.75;

		using value_type = std::pair<KeyType, ValueType>;
		using pointer = value_type*;
		using const_pointer = const value_type*;
		using reference = value_type&;
		using const_reference = const value_type&;
		using wrapped_reference = std::reference_wrapper<value_type>;

		AxHashMap() = default;
		~AxHashMap() = default;

		explicit AxHashMap(size_t capacity)
		{
			m_elements.resize(capacity / m_maxLoadFactor);
		}

		NON_COPYABLE(AxHashMap);

		template <typename... Args>
		bool try_insert(const KeyType& key, const ValueType& value)
		{
			const u64 hash = Hash(key);
			const size_t index = hash % capacity();
			auto [actualIndex, found] = FindFirstEmptySlot(key, index);
			if (!found || actualIndex == kInvalidIndex)
			{
				return false;
			}
			m_elements[actualIndex] = std::make_pair(key, value);
			m_count++;
			return true;
		}

		std::optional<wrapped_reference> find(const KeyType& key)
		{
			const u64 hash = Hash(key);
			const size_t index = hash % capacity();
			if (auto actualIndex = FindFirstEqual(key, index))
			{
				return m_elements[actualIndex.value()];
			}
			return std::nullopt;
		}

		/// \brief Returns the total capacity of the container, i.e. the number of buckets allocated
		/// \return total capacity
		[[nodiscard]] size_t capacity() const { return m_elements.size(); }

		[[nodiscard]] size_t size() const { return m_count; }

	protected:
		std::pair<size_t, bool> FindFirstEmptySlot(const KeyType& key, size_t ideal_index)
		{
			const size_t cap = capacity();
			for (size_t i = 0; i < cap; i++)
			{
				size_t index = (ideal_index + i) % cap;
				KeyType k = m_elements[index].first;
				if (Compare(k, kEmpty) || Compare(k, kTombstone))
				{
					return { index, true }; // Found an empty slot
				}
				if (Compare(k, key))
				{
					return { index, false }; // Found a slot with same key
				}
			}
			return { kInvalidIndex, false }; // Did not find a slot; container is full
		}

		std::optional<size_t> FindFirstEqual(const KeyType& key, size_t ideal_index)
		{
			const size_t cap = capacity();
			for (size_t i = 0; i < cap; i++)
			{
				size_t index = (ideal_index + i) % cap;
				KeyType k = m_elements[index].first;
				if (Compare(k, kEmpty))
				{
					return std::nullopt; // Key does not exist
				}
				if (Compare(k, key))
				{
					return index; // Found key
				}
			}
			return std::nullopt; // Key does not exist
		}

		bool Compare(const KeyType& k1, const KeyType& k2)
		{
			static Comparer s_comparer {};
			return s_comparer(k1, k2);
		}

		uint64_t Hash(const KeyType& key)
		{
			static Hasher s_hasher {};
			return s_hasher(key);
		}

	private:
		float m_maxLoadFactor { kDefaultMaxLoadFactor };
		size_t m_count { 0 };
		AxArray<value_type> m_elements;
	};

}
