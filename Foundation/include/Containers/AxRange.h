#pragma once

#include "Memory/AxManagedClass.h"

namespace apex {
namespace ranges {

	template <typename T>
	concept iterable = requires (T& t)
	{
		typename T::iterator;
		typename T::const_iterator;
		{ t.begin() } -> std::same_as<std::conditional_t<std::is_const_v<T>, typename T::const_iterator, typename T::iterator>>;
		{ t.end() } -> std::same_as<std::conditional_t<std::is_const_v<T>, typename T::const_iterator, typename T::iterator>>;
	};

	template <typename T>
	concept reverse_iterable = requires (T& t)
	{
		typename T::reverse_iterator;
		typename T::const_reverse_iterator;
		{ t.rbegin() } -> std::same_as<std::conditional_t<std::is_const_v<T>, typename T::const_reverse_iterator, typename T::reverse_iterator>>;
		{ t.rend() } -> std::same_as<std::conditional_t<std::is_const_v<T>, typename T::const_reverse_iterator, typename T::reverse_iterator>>;
	};

	template <typename T>
	concept range = iterable<T> || reverse_iterable<T>;

	template <typename Func, typename T>
	concept view_func = requires (Func f, T const& t)
	{
		{ f(t) } -> std::same_as<bool>;
	};

	template <typename F1, typename F2>
	auto operator&(F1&& func1, F2&& func2)
	{
		return [func1 = std::move(func1), func2 = std::move(func2)](auto it)
		{
			return func1(it) && func2(it);
		};
	}
	
	template <typename T>
	concept is_view = requires
	{
		typename T::is_view;
	};

	template <range Range, typename Iterator = typename Range::iterator>
	class AxRange
	{
	public:
		using iterator = Iterator;

		AxRange(iterator begin, iterator end) : m_begin(begin), m_end(end) {}

		AxRange(Range& range) : m_begin(range.begin()), m_end(range.end()) {}

		[[nodiscard]] iterator begin() const
		{
			return m_begin;
		}
		[[nodiscard]] iterator end() const
		{
			return m_end;
		}

	private:
		iterator m_begin;
		iterator m_end;
	};

	template <reverse_iterable T>
	auto reversed(T& range) -> AxRange<T, typename T::reverse_iterator>
	{
		return AxRange<T, typename T::reverse_iterator>(range.rbegin(), range.rend());
	}

	template <range Range, typename ViewFn, typename RangeIterator = typename AxRange<Range>::iterator>
	class AxView : public AxManagedClass
	{
	public:
		using base_iterator = RangeIterator;

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
		struct is_view { };

		using iterator = Iterator;

		AxView(AxRange<Range> const& range, ViewFn const& view_fn)
		: m_range(range)
		, m_viewFn(view_fn)
		{
		}

		AxView(AxRange<Range> const& range, ViewFn&& view_fn)
		: m_range(range)
		, m_viewFn(std::forward<ViewFn>(view_fn))
		{
		}

		AxView(AxRange<Range>&& range, ViewFn const& view_fn)
		: m_range(std::forward<AxRange<Range>>(range))
		, m_viewFn(view_fn)
		{
		}

		AxView(AxRange<Range>&& range, ViewFn&& view_fn)
		: m_range(std::forward<AxRange<Range>>(range))
		, m_viewFn(std::forward<ViewFn>(view_fn))
		{
		}

		AxView(Range& range, ViewFn&& view_fn) : AxView(std::forward<AxRange<Range>>(AxRange(range)), std::forward<ViewFn>(view_fn)) {}
		AxView(Range& range, ViewFn const& view_fn) : AxView(std::forward<AxRange<Range>>(AxRange(range)), view_fn) {}

		//AxView(AxView const& inner, ViewFn&& view_fn) : AxView(inner.m_range, )

		[[nodiscard]] iterator begin() const
		{
			(void)m_range.begin();
			return Iterator(m_range.begin(), static_cast<AxView const&>(*this));
		}

		[[nodiscard]] iterator end() const
		{
			return Iterator(m_range.end(), static_cast<AxView const&>(*this));
		}

	private:
		AxRange<Range> m_range;
		ViewFn m_viewFn;

		friend class Iterator;

		template <typename Rng, typename VFn>
		friend auto view(Rng&& rng, VFn&& vfn);
	};

	template <typename Rng, typename VFn>
	auto view(Rng&& rng, VFn&& vfn)
	{
		if constexpr (is_view<Rng>)
		{
			return view(rng.m_range, rng.m_viewFn, vfn);
		}
		else
		{
			return AxView(std::forward<Rng>(rng), std::forward<VFn>(vfn));
		}
	}

	template <typename Range, typename ViewFn1, typename ViewFn2>
	auto view(Range&& range, ViewFn1&& view_fn1, ViewFn2&& view_fn2)
	{
		return AxView(std::forward<Range>(range), view_fn1 & view_fn2);
	}

}
}
