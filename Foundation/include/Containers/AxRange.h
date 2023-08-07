#pragma once

namespace apex {
namespace ranges {

	template <typename T>
	concept range = requires (T& t)
	{
		typename T::iterator;
		typename T::const_iterator;
		{ t.begin() } -> std::same_as<std::conditional_t<std::is_const_v<T>, typename T::const_iterator, typename T::iterator>>;
		{ t.end() } -> std::same_as<std::conditional_t<std::is_const_v<T>, typename T::const_iterator, typename T::iterator>>;
	};

	template <range Range>
	class AxRange
	{
	public:
		using iterator = std::conditional_t<std::is_const_v<Range>, typename Range::const_iterator, typename Range::iterator>;

		AxRange(iterator begin, iterator end) : m_begin(begin), m_end(end) {}

		AxRange(Range& rng) : AxRange(rng.begin(), rng.end()) {}

		[[nodiscard]] iterator begin() const { return m_begin; }
		[[nodiscard]] iterator end() const { return m_end; }

	private:
		iterator m_begin;
		iterator m_end;
	};

	template <range Range, typename ViewFn>
	class AxView
	{
	public:
		using base_iterator = typename AxRange<Range>::iterator;

		class Iterator : public base_iterator
		{
		public:
			Iterator() = default;

			Iterator(base_iterator const& base_iter, AxView const& owner)
			: base_iterator(base_iter)
			, m_owner(owner)
			{
				if (!isEnd() && !inView())
					next();
			}

			Iterator& operator++() // pre-increment
			{
				next();
				return *this;
			}

			Iterator operator++(int) // post-increment
			{
				Iterator tmp(*this);
				next();
				return tmp;
			}

			void next()
			{
				do
				{
					base_iterator::operator++();
				}
				while (!isEnd() && !inView());
			}

			[[nodiscard]] bool isEnd() const
			{
				return *this == m_owner.m_range.end();
			}

			[[nodiscard]] bool inView() const
			{
				return m_owner.m_viewFn(base_iterator::operator*());
			}

		private:
			AxView const& m_owner;
		};

	public:
		using iterator = Iterator;

		AxView(AxRange<Range> const& range, ViewFn&& view_fn)
		: m_range(range)
		, m_viewFn(std::forward<ViewFn>(view_fn))
		{
		}

		AxView(Range& range, ViewFn&& view_fn) : AxView(AxRange(range), std::forward<ViewFn>(view_fn)) {}

		[[nodiscard]] iterator begin() const { return Iterator(m_range.begin(), (AxView const&)*this); }
		[[nodiscard]] iterator end() const { return Iterator(m_range.end(), (AxView const&)*this); }

	private:
		AxRange<Range> const& m_range;
		ViewFn m_viewFn;

		friend class Iterator;
	};
	
}
}
