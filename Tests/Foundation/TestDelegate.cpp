#include <gtest/gtest.h>

#include "Core/Delegate.h"

namespace detail
{
	static int counter = 0;
}

void counter_inc()
{
	++detail::counter;
}

void counter_dec()
{
	--detail::counter;
}

void counter_set(int value)
{
	detail::counter = value;
}

TEST(TestDelegate, TestVoidReturnFreeFunction)
{
	EXPECT_EQ(detail::counter, 0);
	{
		apex::Delegate<void()> delegate;
		
		delegate.set<counter_inc>();
		delegate();
		EXPECT_EQ(detail::counter, 1);

		delegate.set<counter_dec>();
		delegate();
		EXPECT_EQ(detail::counter, 0);

	}

	EXPECT_EQ(detail::counter, 0);
	{
		apex::Delegate<void(int)> delegate;	
		delegate.set<counter_set>();
		delegate(42);
		EXPECT_EQ(detail::counter, 42);
	}

	counter_set(0);
}

TEST(TestDelegate, TestVoidReturnNonCapturingLambda)
{
	EXPECT_EQ(detail::counter, 0);
	{
		apex::Delegate<void()> delegate;
		delegate.set<[]() { ++detail::counter; }>();
		delegate();
		EXPECT_EQ(detail::counter, 1);

		delegate.set<[]() { --detail::counter; }>();
		delegate();
		EXPECT_EQ(detail::counter, 0);
	}

	EXPECT_EQ(detail::counter, 0);
	{
		apex::Delegate<void(int)> delegate;
		delegate.set<[](int value) { detail::counter = value; }>();
		delegate(42);
		EXPECT_EQ(detail::counter, 42);
	}
	
	counter_set(0);
}


TEST(TestDelegate, TestVoidReturnCapturingLambda)
{
	EXPECT_EQ(detail::counter, 0);
	{
		int value = 42;
		auto lambda = [&value]() { detail::counter = value; };
		apex::Delegate<void()> delegate;
		delegate.set(lambda);
		delegate();
		EXPECT_EQ(detail::counter, 42);

		value = 0;
		delegate();
		EXPECT_EQ(detail::counter, 0);
	}
}

TEST(TestDelegate, TestVoidReturnMemberFunction)
{
	struct Counter
	{
		int value = 0;
		void inc() { ++value; }
		void dec() { --value; }
		void set(int val) { value = val; }
	};

	{
		Counter counter;
		apex::Delegate<void()> delegate;
		delegate.set<&Counter::inc>(counter);
		delegate();
		EXPECT_EQ(counter.value, 1);

		delegate.set<&Counter::dec>(counter);
		delegate();
		EXPECT_EQ(counter.value, 0);
	}

	EXPECT_EQ(detail::counter, 0);
	{
		Counter counter;
		apex::Delegate<void(int)> delegate;
		delegate.set<&Counter::set>(counter);
		delegate(42);
		EXPECT_EQ(counter.value, 42);
	}
}


TEST(TestDelegate, TestMemberFunctionFromBaseClass)
{
	struct Base
	{
		int value = 0;
		void inc() { ++value; }
	};

	struct Derived : Base
	{
		void dec() { --value; }
	};

	{
		Derived derived;
		apex::Delegate<void()> delegate;
		delegate.set<&Base::inc>(derived);
		delegate();
		EXPECT_EQ(derived.value, 1);

		delegate.set<&Derived::dec>(derived);
		delegate();
		EXPECT_EQ(derived.value, 0);
	}
}

template <typename T>
void counter_inc_template(T& counter)
{
	++counter;
}

TEST(TestDelegate, TestFreeFunctionTemplate)
{
	int counter = 0;
	{
		apex::Delegate<void(int&)> delegate;
		delegate.set<counter_inc_template<int>>();
		delegate(counter);
		EXPECT_EQ(counter, 1);
	}	
}

struct CounterInvoker
{
	apex::Delegate<void(const int&)> delegate;
};

struct Counter
{
	int value = 0;

	constexpr Counter() = default;
	Counter(CounterInvoker& invoker);

	template <typename T>
	void set(T const& val) { value = val; }
};

Counter::Counter(CounterInvoker& invoker)
{
	invoker.delegate.set<&Counter::set<int>>(*this);
}

TEST(TestDelegate, TestMemberFunctionTemplate)
{
	{
		Counter counter;
		apex::Delegate<void(const int&)> delegate;
		delegate.set<&Counter::set<int>>(counter);
		delegate(42);
		EXPECT_EQ(counter.value, 42);
	}
}

TEST(TestDelegate, TestMemberFunctionTemplateWithDifferentType)
{
	{
		Counter counter;
		apex::Delegate<void(const float&)> delegate;
		delegate.set<&Counter::set<float>>(counter);
		delegate(42.0f);
		EXPECT_EQ(counter.value, 42);
	}
}

TEST(TestDelegate, TestMemberFunctionTemplateWithThisPointer)
{
	{
		CounterInvoker invoker;
		Counter counter(invoker);

		invoker.delegate(42);
		
		EXPECT_EQ(counter.value, 42);
	}
}

template <typename Derived>
struct CounterTemplate
{
	int value = 0;

	constexpr CounterTemplate() = default;
	CounterTemplate(CounterInvoker& invoker);

	template <typename U>
	void set(U const& val) { value = val; }
};

template <typename Derived>
CounterTemplate<Derived>::CounterTemplate(CounterInvoker& invoker)
{
	invoker.delegate.set<&Derived::template set<int>>(static_cast<Derived&>(*this));
}

struct DoubleCounter : public CounterTemplate<DoubleCounter>
{
	constexpr DoubleCounter() : CounterTemplate() {}
	DoubleCounter(CounterInvoker& invoker) : CounterTemplate(invoker) {}

	template <typename U>
	void set(U const& val) { CounterTemplate::set(val); }

	template <>
	void set(int const& val) { value = val * 2; }
};

TEST(TestDelegate, TestMemberFunctionTemplateFromBaseClass)
{
	{
		CounterInvoker invoker;
		DoubleCounter counter(invoker);

		counter.set(42);
		EXPECT_EQ(counter.value, 84);
	}
}

void call_delegate(apex::Delegate<bool(Counter)> const& delegate, Counter counter)
{
	EXPECT_TRUE(delegate(std::forward<Counter>(counter)));
}

TEST(TestDelegate, TestPassingIntoFunction)
{
	{
		Counter counter;
		apex::Delegate<bool(Counter)> delegate;
		counter.set(42);
		auto lambda = [](Counter counter) { return counter.value > 5; };
		delegate.set(lambda);

		call_delegate(delegate, counter);
		EXPECT_EQ(counter.value, 42);
	}
}
