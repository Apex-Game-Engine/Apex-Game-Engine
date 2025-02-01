#pragma once
#include "AxArray.h"
#include "AxRange.h"
#include "AxSparseSet.h"
#include "Core/Asserts.h"

namespace apex {

	template <typename Key, typename Type>
	class AxSparseMap : public AxSparseSet<Key>
	{
	public:
		using base_type = AxSparseSet<Key>;

		using base_type::key_type;
		using base_type::sparse_array;
		using base_type::dense_array;

		using element_type = Type;
		using element_array = AxArray<element_type>;

		AxSparseMap() = default;

		explicit AxSparseMap(u32 capacity)
		: base_type(capacity)
		, m_elements(capacity)
		{
			m_elements.resize(capacity);
		}

		~AxSparseMap() = default;

		void resize(size_t capacity)
		{
			// TODO: Do NOT resize elements array if not required
			base_type::reserve(capacity);
			m_elements.resize(base_type::capacity());
		}

		void insert(key_type id, Type const& elem)
		{
			_Insert(id, elem);
		}

		void insert(key_type id, Type&& elem)
		{
			_Insert(id, std::forward<Type>(elem));
		}

		bool try_insert(key_type id, Type const& elem)
		{
			if (!contains(id))
			{
				_Insert(id, elem);
				return true;
			}

			return false;
		}

		template <typename... Args>
		Type& emplace(key_type id, Args&&... args)
		{
			return _Emplace(id, std::forward<Args>(args)...);
		}

		void remove(key_type id)
		{
			_Remove(id);
		}

		bool try_remove(key_type id)
		{
			if (contains(id))
			{
				_Remove(id);
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

		auto getElement(key_type id) -> Type&
		{
			key_type idx = getIndex(id);
			return m_elements[idx];
		}

		auto getElement(key_type id) const -> Type const&
		{
			return const_cast<AxSparseMap*>(this)->getElement(id);
		}

		auto get(key_type id) -> std::pair<key_type, Type&>
		{
			key_type idx = getIndex(id);
			return std::pair<key_type, Type&>(idx, m_elements[idx]);
		}

		auto get(key_type id) const -> std::pair<key_type, Type const&>
		{
			key_type idx = getIndex(id);
			return std::pair<key_type, Type const&>(idx, m_elements[idx]);
		}

		auto try_get(key_type id) -> std::optional<std::pair<key_type, Type&>>
		{
			auto idx = try_getIndex(id);
			if (idx)
			{
				return std::pair<key_type, Type&>(idx.value(), m_elements[idx.value()]);
			}

			return std::nullopt;
		}

		auto try_get(key_type id) const -> std::optional<std::pair<key_type, Type const&>>
		{
			auto idx = try_getIndex(id);
			if (idx)
			{
				return std::pair<key_type, Type const&>(idx.value(), m_elements[idx.value()]);
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
		void _Insert(key_type id, Type const& elem)
		{
			base_type::_Insert(id);
			auto index = base_type::_GetIndex(id);
			m_elements[index] = elem;
		}

		void _Insert(key_type id, Type&& elem)
		{
			base_type::_Insert(id);
			auto index = base_type::_GetIndex(id);
			m_elements[index] = std::move(elem);
		}

		template <typename... Args>
		Type& _Emplace(key_type id, Args&&... args)
		{
			base_type::_Insert(id);
			auto index = base_type::_GetIndex(id);
			auto pElem = std::construct_at(&m_elements[index], std::forward<Args>(args)...);
			return *pElem;
		}

		void _Remove(key_type id)
		{
			auto index = base_type::_GetIndex(id);

			base_type::_Remove(id);
			auto lastIndex = count();

			m_elements[index] = m_elements[lastIndex];
		}

	private:
		element_array m_elements{};
	};


	template <typename Key, apex::empty Type>
	class AxSparseMap<Key, Type> : public AxSparseSet<Key>
	{
	public:
		using base_type = AxSparseSet<Key>;

		using base_type::key_type;
		using base_type::sparse_array;
		using base_type::dense_array;

		using element_type = Type;

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

		void insert(key_type id, Type const& elem = {})
		{
			base_type::_Insert(id);
		}

		bool try_insert(key_type id, Type const& elem = {})
		{
			if (!contains(id))
			{
				base_type::_Insert(id);
				return true;
			}

			return false;
		}

		void remove(key_type id)
		{
			base_type::_Remove(id);
		}

		bool try_remove(key_type id)
		{
			if (contains(id))
			{
				base_type::_Remove(id);
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

		auto getElement(key_type id) const -> Type
		{
			key_type idx = getIndex(id);
			return {};
		}

		auto get(key_type id) const -> std::pair<key_type, Type>
		{
			key_type idx = getIndex(id);
			return std::pair<key_type, Type>(idx, {});
		}

		auto try_get(key_type id) const -> std::optional<std::pair<key_type, Type>>
		{
			auto idx = try_getIndex(id);
			if (idx)
			{
				return std::pair<key_type, Type>(idx.value(), {});
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
