#pragma once
#include <utility>

#include "TypeTraits.h"


namespace apex {

	template <auto>
	struct delegate_arg_t
	{
	    explicit delegate_arg_t() = default;
	};

	template <auto Candidate>
	inline constexpr delegate_arg_t<Candidate> delegate_arg{};

	template <typename>
	struct Delegate;

	template <typename Ret, typename... Args>
	struct Delegate<Ret(Args...)>
	{
	    using function_type = Ret(const void*, Args...);
	    using type = Ret(Args...);
	    using result_type = Ret;
	    using args_type_list = type_list<Args...>;

	    constexpr Delegate() = default;

	    template <auto Candidate, typename... Type>
	    Delegate(delegate_arg_t<Candidate>, Type&&... value_or_instance)
	    {
	        set<Candidate>(std::forward<Type>(value_or_instance)...);
	    }

	    Delegate(function_type* function, const void* payload = nullptr)
	    {
	        set(function, payload);
	    }

	    template <auto Candidate>
	    void set()
	    {
	        instance = nullptr;

	        if constexpr(std::is_invocable_r_v<Ret, decltype(Candidate), Args...>)
	        {
	            fn = +[](const void*, Args... args) -> Ret
	            {
	                return Ret(std::invoke(Candidate, std::forward<Args>(args)...));
	            };
	        }
	        else
	        {
	            [flag = false]() { static_assert(flag, "Delegate candidate is not invocable!"); }();
	        }
	    }

	    template <auto Candidate, typename Type>
	    void set(Type& value_or_instance)
	    {
	        instance = &value_or_instance;

	        if constexpr(std::is_invocable_r_v<Ret, decltype(Candidate), Type&, Args...>)
	        {
	            fn = +[](const void* payload, Args... args) -> Ret
	            {
	                Type* value = static_cast<Type*>(const_cast<std::conditional_t<std::is_const_v<Type>, const void, void>*>(payload));
	                return Ret(std::invoke(Candidate, *value, std::forward<Args>(args)...));
	            };
	        }
	        else
	        {
	            [flag = false]() { static_assert(flag, "Delegate candidate is not invocable!"); }();
	        }
	    }

	    void set(function_type* function, const void* payload = nullptr)
	    {
	        instance = payload;
	        fn = function;
	    }

	    auto operator()(Args&&... args) const -> Ret
	    {
	        return fn(instance, std::forward<Args>(args)...);
	    }

	    explicit operator bool() const
	    {
	        return fn != nullptr;
	    }

	    const void *instance {nullptr};
	    function_type *fn {nullptr};
	};



}
