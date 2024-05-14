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
	    using function_type = Ret(const void*, Args&&...);
	    using type = Ret(Args...);
	    using result_type = Ret;
	    using args_type_list = type_list<Args...>;

	    constexpr Delegate() = default;

	    template <auto Candidate, typename... Type>
	    Delegate(delegate_arg_t<Candidate>, Type&&... value_or_instance)
	    {
	        set<Candidate>(std::forward<Type>(value_or_instance)...);
	    }

		template <typename Candidate>
		Delegate(Candidate& candidate)
		{
			set(candidate);
		}

	    Delegate(function_type* function, const void* payload = nullptr)
	    {
	        set(function, payload);
	    }

	    /**
	     * \brief Sets the delegate to a free function or static member function.
	     * \tparam Candidate Candidate free function or static member function decltype. Must match the delegate signature.
	     */
	    template <auto Candidate>
	    void set()
	    {
	        instance = nullptr;

	        if constexpr(std::is_invocable_r_v<Ret, decltype(Candidate), Args&&...>)
	        {
	            fn = +[](const void*, Args&&... args) -> Ret
	            {
	                return Ret(std::invoke(Candidate, std::forward<Args>(args)...));
	            };
	        }
	        else
	        {
	            [flag = false]() { static_assert(flag, "Delegate candidate is not invocable!"); }();
	        }
	    }

	    /**
	     * \brief Sets the delegate to a member function or functor instance.
	     * \tparam Candidate Candidate member function or functor decltype. Must match the delegate signature.
	     * \tparam Type Type of functor or class instance.
	     * \param value_or_instance Functor or class instance
	     *
	     * The instance is captured by reference, so it must be alive during the delegate lifetime.
	     */
	    template <auto Candidate, typename Type>
	    void set(Type& value_or_instance)
	    {
	        instance = &value_or_instance;

	        if constexpr(std::is_invocable_r_v<Ret, decltype(Candidate), Type&, Args&&...>)
	        {
	            fn = +[](const void* payload, Args&&... args) -> Ret
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

	    /**
	     * \brief Sets the delegate to a lambda or function instance. Also works with capturing lambdas.
	     * \tparam Candidate Candidate function or lambda decltype. Must match the delegate signature. Deduced from the argument.
	     * \param candidate Candidate function or lambda instance
	     *
	     * In case of a lambda, the lambda instance is captured by reference, so it must be alive during the delegate lifetime.
	     */
	    template <typename Candidate>
		void set(Candidate& candidate)
	    {
		    set<&Candidate::operator()>(candidate);
	    }

	    /**
	     * \brief Sets the delegate to function pointer or static member function, with a payload.
	     * \param function Function pointer or static member function, with additional `const void*` parameter before the arguments.
	     * \param payload Payload to be passed to the function, before the arguments.
	     */
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
