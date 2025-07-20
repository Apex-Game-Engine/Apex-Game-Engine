#pragma once
#include "AxArray.h"
#include "AxRange.h"
#include "AxSparseSet.h"
#include "Core/Asserts.h"

namespace apex {

	template <typename KeyType, typename ValueType>
	class AxSparseMap : public AxSparseSet<KeyType>
	{
	public:
		using base_type = AxSparseSet<KeyType>;

		using base_type::sparse_array;
		using base_type::dense_array;
		
		using base_type::key_type;
		using base_type::key_pointer;
		using base_type::const_key_pointer;
		using base_type::key_reference;
		using base_type::const_key_reference;

		using element_type = ValueType;
		using element_array = AxArray<element_type>;

		using value_type = ValueType;
		using value_pointer = ValueType*;
		using const_value_pointer = const ValueType*;
		using value_reference = ValueType&;
		using const_value_reference = const ValueType&;

		using pair_type = std::pair<KeyType, ValueType&>;
		using const_pair_type = std::pair<KeyType, const ValueType&>;

		AxSparseMap() = default;

		explicit AxSparseMap(u32 capacity)
		: base_type(capacity)
		, m_elements(capacity)
		{
			m_elements.resize(capacity);
		}

		~AxSparseMap() = default;

		NON_COPYABLE(AxSparseMap);

		AxSparseMap(AxSparseMap&& other) noexcept { *this = std::move(other); }
		AxSparseMap& operator=(AxSparseMap&& other) noexcept
		{
			base_type::operator=(std::move(other));
			m_elements = std::move(other.m_elements);
			return *this;
		}
		

		void resize(size_t capacity)
		{
			// TODO: Do NOT resize elements array if not required
			base_type::reserve(capacity);
			m_elements.resize(base_type::capacity());
		}

		void insert(key_type id, ValueType const& elem)
		{
			Insert(id, elem);
		}

		void insert(key_type id, ValueType&& elem)
		{
			Insert(id, std::forward<ValueType>(elem));
		}

		bool try_insert(key_type id, ValueType const& elem)
		{
			if (!contains(id))
			{
				Insert(id, elem);
				return true;
			}

			return false;
		}

		template <typename... Args>
		auto emplace(key_type id, Args&&... args) -> ValueType&
		{
			return Emplace(id, std::forward<Args>(args)...);
		}

		void remove(key_type id)
		{
			Remove(id);
		}

		bool try_remove(key_type id)
		{
			if (contains(id))
			{
				Remove(id);
				return true;
			}

			return false;
		}

		bool contains(key_type id) const
		{
			return base_type::contains(id);
		}

		key_type getIndex(key_type id) const
		{
			return base_type::getIndex(id);
		}

		auto try_getIndex(key_type id)
		{
			return base_type::try_getIndex(id);
		}

		auto getElement(key_type id) -> value_reference
		{
			key_type idx = getIndex(id);
			return m_elements[idx];
		}

		auto getElement(key_type id) const -> const_value_reference
		{
			return const_cast<AxSparseMap*>(this)->getElement(id);
		}

		auto get(key_type id) -> pair_type
		{
			key_type idx = getIndex(id);
			return { idx, m_elements[idx] };
		}

		auto get(key_type id) const -> const_pair_type
		{
			key_type idx = getIndex(id);
			return { idx, m_elements[idx] };
		}

		auto try_get(key_type id) -> std::optional<pair_type>
		{
			auto idx = try_getIndex(id);
			if (idx)
			{
				return { idx.value(), m_elements[idx.value()] };
			}

			return std::nullopt;
		}

		auto try_get(key_type id) const -> std::optional<const_pair_type>
		{
			auto idx = try_getIndex(id);
			if (idx)
			{
				return { idx.value(), m_elements[idx.value()] };
			}

			return std::nullopt;
		}

		void clear() { base_type::clear(); }

		auto elements() { return ranges::AxRange<element_array>(m_elements.begin(), m_elements.begin() + count()); }
		auto elements() const { return ranges::AxRange<const element_array>(m_elements.begin(), m_elements.begin() + count()); }

		auto keys() { return base_type::keys(); }
		auto keys() const { return base_type::keys(); }

		size_t capacity() const { return base_type::capacity(); }
		size_t count() const { return base_type::count(); }

	protected:
		void Insert(key_type id, ValueType const& elem)
		{
			base_type::Insert(id);
			auto index = base_type::GetIndex(id);
			m_elements[index] = elem;
		}

		void Insert(key_type id, ValueType&& elem)
		{
			base_type::Insert(id);
			auto index = base_type::GetIndex(id);
			m_elements[index] = std::move(elem);
		}

		template <typename... Args>
		ValueType& Emplace(key_type id, Args&&... args)
		{
			base_type::Insert(id);
			auto index = base_type::GetIndex(id);
			auto pElem = std::construct_at(&m_elements[index], std::forward<Args>(args)...);
			return *pElem;
		}

		void Remove(key_type id)
		{
			auto index = base_type::GetIndex(id);

			base_type::Remove(id);
			auto lastIndex = count();

			m_elements[index] = m_elements[lastIndex];
		}

	private:
		element_array m_elements{};
	};


	template <typename KeyType, apex::empty ValueType>
	class AxSparseMap<KeyType, ValueType> : public AxSparseSet<KeyType>
	{
	public:
		using base_type = AxSparseSet<KeyType>;

		using base_type::key_type;
		using base_type::sparse_array;
		using base_type::dense_array;

		using element_type = ValueType;

		using pair_type = std::pair<KeyType, ValueType&>;

		AxSparseMap() = default;

		explicit AxSparseMap(u32 capacity)
		: base_type(capacity)
		{
		}

		~AxSparseMap() = default;


		void resize(size_t capacity)
		{
			// TODO: Do NOT resize elements array if not required
			base_type::reserve(capacity);
		}

		void insert(key_type id, ValueType const& elem = {})
		{
			base_type::Insert(id);
		}

		bool try_insert(key_type id, ValueType const& elem = {})
		{
			if (!contains(id))
			{
				base_type::Insert(id);
				return true;
			}

			return false;
		}

		void remove(key_type id)
		{
			base_type::Remove(id);
		}

		bool try_remove(key_type id)
		{
			if (contains(id))
			{
				base_type::Remove(id);
				return true;
			}

			return false;
		}

		bool contains(key_type id) const
		{
			return base_type::contains(id);
		}

		key_type getIndex(key_type id) const
		{
			return base_type::getIndex(id);
		}

		auto try_getIndex(key_type id)
		{
			return base_type::try_getIndex(id);
		}

		auto getElement(key_type id) const -> ValueType
		{
			key_type idx = getIndex(id);
			return {};
		}

		auto get(key_type id) const -> std::pair<key_type, ValueType>
		{
			key_type idx = getIndex(id);
			return std::pair<key_type, ValueType>(idx, {});
		}

		auto try_get(key_type id) const -> std::optional<std::pair<key_type, ValueType>>
		{
			auto idx = try_getIndex(id);
			if (idx)
			{
				return std::pair<key_type, ValueType>(idx.value(), {});
			}

			return std::nullopt;
		}

		void clear() { base_type::clear(); }

		auto keys() { return base_type::keys(); }
		auto keys() const { return base_type::keys(); }

		size_t capacity() const { return base_type::capacity(); }
		size_t count() const { return base_type::count(); }

	};

}
