#pragma once
#include "Memory/AxManagedClass.h"
#include "Memory/UniquePtr.h"
#include "Core/Asserts.h"

namespace apex {

	/**
	 * \brief Doubly linked list
	 * \tparam T Type of the elements stored in the list
	 */
	template <typename T>
	class AxList : public AxManagedClass
	{
	public:
		using underlying_type = T;

	private:

		class IListNode : public AxManagedClass
		{
		public:
			virtual ~IListNode() = default;

			virtual underlying_type& data() = 0;
			virtual UniquePtr<IListNode>& next() = 0;
			virtual IListNode* prev() = 0;
			virtual void prev(IListNode* node) = 0;
		};

		struct ListNode : public IListNode
		{
		public:
			ListNode() requires std::is_default_constructible_v<underlying_type> = default;
			~ListNode() = default;

			template <typename... Args>
			ListNode(Args&&... args) : m_data(std::forward<Args>(args)...) {}

			underlying_type& data() override { return m_data; }
			UniquePtr<IListNode>& next() override { return m_next; }
			IListNode* prev() override { return m_prev; }
			void prev(IListNode* node) override { m_prev = static_cast<ListNode*>(node); }

		private:
			underlying_type      m_data;
			UniquePtr<IListNode> m_next{}; // Next node in the list (strong reference)
			ListNode*            m_prev{}; // Previous node in the list (weak reference)

			friend struct StaticAsserts;
		};

		struct StaticAsserts
		{
			static_assert(offsetof(ListNode, m_data) == 0);
		};
		static_assert(apex::convertible_to<ListNode, IListNode>);

	public:
		using node_t = IListNode;
		using node_ptr_t = UniquePtr<IListNode>;

		// Iterator class for AxList
		template <typename IterType>
		class Iterator
		{
		public:
			using iterator_category = std::bidirectional_iterator_tag;
			using value_type = underlying_type;
			using difference_type = std::ptrdiff_t;
			using pointer = value_type*;
			using reference = value_type&;

			Iterator() : m_ptr(), m_list() {}
			Iterator(IterType* node, AxList* list) : m_ptr(node), m_list(list) {}
			Iterator& operator++() { m_ptr = m_ptr->next().get(); return *this; } // pre-increment
			Iterator operator++(int) { Iterator tmp(*this); operator++(); return tmp; } // post-increment
			Iterator& operator--() { m_ptr = m_ptr->prev(); return *this; } // pre-decrement
			Iterator operator--(int) { Iterator tmp(*this); operator--(); return tmp; } // post-decrement
			bool operator==(const Iterator& rhs) const { return m_ptr == rhs.m_ptr; }
			bool operator!=(const Iterator& rhs) const { return m_ptr != rhs.m_ptr; }
			reference operator*() const { return m_ptr->data(); }
			pointer operator->() const { return &m_ptr->data(); }

		private:
			IterType* m_ptr;
			AxList* m_list;

			friend class AxList;
		};

	public:
		//using stored_type = std::conditional_t<apex::managed_class<T>, underlying_type, AxManagedClassAdapter<T>>;
		using value_type = underlying_type;
		using pointer = value_type*;
		using const_pointer = const value_type*;
		using reference = value_type&;
		using const_reference = const value_type&;
		using iterator = Iterator<node_t>;
		using const_iterator = Iterator<const node_t>;

		AxList() = default;

		void append(const value_type& value)
		{
			node_ptr_t node = std::move(apex::make_unique<ListNode>(value));
			_AppendNode(std::move(node));
		}

		void append(value_type&& value)
		{
			node_ptr_t node = std::move(apex::make_unique<ListNode>(std::forward<value_type>(value)));
			_AppendNode(std::move(node));
		}

		template <typename... Args>
		void emplace_back(Args&&... args) requires std::is_constructible_v<underlying_type, Args...>
		{
			node_ptr_t node = apex::make_unique<ListNode>(std::forward<Args>(args)...);
			_AppendNode(std::move(node));
		}

		iterator insert(iterator const& it, const value_type& value)
		{
			axAssertMsg(it.m_ptr, "Invalid iterator");
			axAssertMsg(this == it.m_list, "Iterator does not belong to this list");

			TODO("Not implemented yet!");

			return {};
		}

		iterator remove(iterator const& it)
		{
			axAssertMsg(it.m_ptr, "Invalid iterator");
			axAssertMsg(this == it.m_list, "Iterator does not belong to this list");

			iterator next = it;
			++next;
			_RemoveNode(it.m_ptr);
			return next;
		}

		[[nodiscard]] auto back() -> reference { return *m_tail->data(); }
		[[nodiscard]] auto back() const -> const_reference { return *m_tail->data(); }

		[[nodiscard]] auto front() -> reference { return m_head->data(); }
		[[nodiscard]] auto front() const -> const_reference { return m_head->data(); }

		[[nodiscard]] size_t size() const { return m_size; }
		[[nodiscard]] bool   empty() const { return m_size == 0; }

		#pragma region Iterator functions
		[[nodiscard]] iterator begin() { return iterator(m_head.get(), this); }
		[[nodiscard]] iterator end() { return iterator(nullptr, this); }

		[[nodiscard]] const_iterator cbegin() const { return const_iterator(m_head.get(), this); }
		[[nodiscard]] const_iterator cend() const { return const_iterator(m_tail.get(), this); }

		[[nodiscard]] const_iterator begin() const { return cbegin(); }
		[[nodiscard]] const_iterator end() const { return cend(); }
	#pragma endregion

	private:
		void _AppendNode(node_ptr_t&& node)
		{
			if (!m_head)
			{
				m_head = std::move(node);
				m_tail = m_head.get();
				++m_size;
			}
			else
			{
				_InsertNode(std::move(node), m_tail);
				m_tail = m_tail->next().get();
			}
		}

		void _InsertNode(node_ptr_t&& node, node_t* prev)
		{
			node->prev(prev);
			if (prev->next())
			{
				prev->next()->prev(node.get());
				node->next().swap(prev->next());
			}
			prev->next() = std::move(node);
			++m_size;
		}

		void _RemoveNode(node_t* node)
		{
			if (node->next())
			{
				node->next()->prev(node->prev());
			}
			if (node == m_tail)
			{
				m_tail = node->prev();
			}

			if (node == m_head.get())
			{
				node_ptr_t tmp = std::move(m_head);
				m_head = std::move(tmp->next());
				m_head->prev(nullptr);
			}
			else if (node->prev())
			{
				node_t* prev = node->prev();
				node_ptr_t tmp = std::move(prev->next());
				prev->next() = std::move(node->next());
			}
			--m_size;
		}

	private:
		node_ptr_t m_head{}; // Head node of the list (strong reference)
		node_t*    m_tail{}; // Tail node of the list (weak reference)
		size_t m_size{};
	};

}
