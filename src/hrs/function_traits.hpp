#pragma once

#include "variadic.hpp"
#include <type_traits>

namespace hrs
{
    template<typename F>
    requires std::is_function_v<F>
    struct function_traits;

    template<typename R, typename... Args>
    struct function_traits<R(Args...)>
    {
        using return_type = R;
        using arguments = variadic<Args...>;
        constexpr static bool is_const_qualified = false;
        constexpr static bool is_volatile_qualified = false;
        constexpr static bool is_lvalue_ref_qualified = false;
        constexpr static bool is_rvalue_ref_qualified = false;
        constexpr static bool is_noexcept_qualified = false;
        constexpr static bool has_variadic_arguments = false;
    };

    template<typename R, typename... Args>
    struct function_traits<R(Args...) const>
    {
        using return_type = R;
        using arguments = variadic<Args...>;
        constexpr static bool is_const_qualified = true;
        constexpr static bool is_volatile_qualified = false;
        constexpr static bool is_lvalue_ref_qualified = false;
        constexpr static bool is_rvalue_ref_qualified = false;
        constexpr static bool is_noexcept_qualified = false;
        constexpr static bool has_variadic_arguments = false;
    };

    template<typename R, typename... Args>
    struct function_traits<R(Args...) volatile>
    {
        using return_type = R;
        using arguments = variadic<Args...>;
        constexpr static bool is_const_qualified = false;
        constexpr static bool is_volatile_qualified = true;
        constexpr static bool is_lvalue_ref_qualified = false;
        constexpr static bool is_rvalue_ref_qualified = false;
        constexpr static bool is_noexcept_qualified = false;
        constexpr static bool has_variadic_arguments = false;
    };

    template<typename R, typename... Args>
    struct function_traits<R(Args...) const volatile>
    {
        using return_type = R;
        using arguments = variadic<Args...>;
        constexpr static bool is_const_qualified = true;
        constexpr static bool is_volatile_qualified = true;
        constexpr static bool is_lvalue_ref_qualified = false;
        constexpr static bool is_rvalue_ref_qualified = false;
        constexpr static bool is_noexcept_qualified = false;
        constexpr static bool has_variadic_arguments = false;
    };

    template<typename R, typename... Args>
    struct function_traits<R(Args...)&>
    {
        using return_type = R;
        using arguments = variadic<Args...>;
        constexpr static bool is_const_qualified = false;
        constexpr static bool is_volatile_qualified = false;
        constexpr static bool is_lvalue_ref_qualified = true;
        constexpr static bool is_rvalue_ref_qualified = false;
        constexpr static bool is_noexcept_qualified = false;
        constexpr static bool has_variadic_arguments = false;
    };

    template<typename R, typename... Args>
    struct function_traits<R(Args...) const&>
    {
        using return_type = R;
        using arguments = variadic<Args...>;
        constexpr static bool is_const_qualified = true;
        constexpr static bool is_volatile_qualified = false;
        constexpr static bool is_lvalue_ref_qualified = true;
        constexpr static bool is_rvalue_ref_qualified = false;
        constexpr static bool is_noexcept_qualified = false;
        constexpr static bool has_variadic_arguments = false;
    };

    template<typename R, typename... Args>
    struct function_traits<R(Args...) volatile&>
    {
        using return_type = R;
        using arguments = variadic<Args...>;
        constexpr static bool is_const_qualified = false;
        constexpr static bool is_volatile_qualified = true;
        constexpr static bool is_lvalue_ref_qualified = true;
        constexpr static bool is_rvalue_ref_qualified = false;
        constexpr static bool is_noexcept_qualified = false;
        constexpr static bool has_variadic_arguments = false;
    };

    template<typename R, typename... Args>
    struct function_traits<R(Args...) const volatile&>
    {
        using return_type = R;
        using arguments = variadic<Args...>;
        constexpr static bool is_const_qualified = true;
        constexpr static bool is_volatile_qualified = true;
        constexpr static bool is_lvalue_ref_qualified = true;
        constexpr static bool is_rvalue_ref_qualified = false;
        constexpr static bool is_noexcept_qualified = false;
        constexpr static bool has_variadic_arguments = false;
    };

    template<typename R, typename... Args>
    struct function_traits<R(Args...) &&>
    {
        using return_type = R;
        using arguments = variadic<Args...>;
        constexpr static bool is_const_qualified = false;
        constexpr static bool is_volatile_qualified = false;
        constexpr static bool is_lvalue_ref_qualified = false;
        constexpr static bool is_rvalue_ref_qualified = true;
        constexpr static bool is_noexcept_qualified = false;
        constexpr static bool has_variadic_arguments = false;
    };

    template<typename R, typename... Args>
    struct function_traits<R(Args...) const&&>
    {
        using return_type = R;
        using arguments = variadic<Args...>;
        constexpr static bool is_const_qualified = true;
        constexpr static bool is_volatile_qualified = false;
        constexpr static bool is_lvalue_ref_qualified = false;
        constexpr static bool is_rvalue_ref_qualified = true;
        constexpr static bool is_noexcept_qualified = false;
        constexpr static bool has_variadic_arguments = false;
    };

    template<typename R, typename... Args>
    struct function_traits<R(Args...) volatile&&>
    {
        using return_type = R;
        using arguments = variadic<Args...>;
        constexpr static bool is_const_qualified = false;
        constexpr static bool is_volatile_qualified = true;
        constexpr static bool is_lvalue_ref_qualified = false;
        constexpr static bool is_rvalue_ref_qualified = true;
        constexpr static bool is_noexcept_qualified = false;
        constexpr static bool has_variadic_arguments = false;
    };

    template<typename R, typename... Args>
    struct function_traits<R(Args...) const volatile&&>
    {
        using return_type = R;
        using arguments = variadic<Args...>;
        constexpr static bool is_const_qualified = true;
        constexpr static bool is_volatile_qualified = true;
        constexpr static bool is_lvalue_ref_qualified = false;
        constexpr static bool is_rvalue_ref_qualified = true;
        constexpr static bool is_noexcept_qualified = false;
        constexpr static bool has_variadic_arguments = false;
    };

    template<typename R, typename... Args>
    struct function_traits<R(Args...) noexcept>
    {
        using return_type = R;
        using arguments = variadic<Args...>;
        constexpr static bool is_const_qualified = false;
        constexpr static bool is_volatile_qualified = false;
        constexpr static bool is_lvalue_ref_qualified = false;
        constexpr static bool is_rvalue_ref_qualified = false;
        constexpr static bool is_noexcept_qualified = true;
        constexpr static bool has_variadic_arguments = false;
    };

    template<typename R, typename... Args>
    struct function_traits<R(Args...) const noexcept>
    {
        using return_type = R;
        using arguments = variadic<Args...>;
        constexpr static bool is_const_qualified = true;
        constexpr static bool is_volatile_qualified = false;
        constexpr static bool is_lvalue_ref_qualified = false;
        constexpr static bool is_rvalue_ref_qualified = false;
        constexpr static bool is_noexcept_qualified = true;
        constexpr static bool has_variadic_arguments = false;
    };

    template<typename R, typename... Args>
    struct function_traits<R(Args...) volatile noexcept>
    {
        using return_type = R;
        using arguments = variadic<Args...>;
        constexpr static bool is_const_qualified = false;
        constexpr static bool is_volatile_qualified = true;
        constexpr static bool is_lvalue_ref_qualified = false;
        constexpr static bool is_rvalue_ref_qualified = false;
        constexpr static bool is_noexcept_qualified = true;
        constexpr static bool has_variadic_arguments = false;
    };

    template<typename R, typename... Args>
    struct function_traits<R(Args...) const volatile noexcept>
    {
        using return_type = R;
        using arguments = variadic<Args...>;
        constexpr static bool is_const_qualified = true;
        constexpr static bool is_volatile_qualified = true;
        constexpr static bool is_lvalue_ref_qualified = false;
        constexpr static bool is_rvalue_ref_qualified = false;
        constexpr static bool is_noexcept_qualified = true;
        constexpr static bool has_variadic_arguments = false;
    };

    template<typename R, typename... Args>
    struct function_traits<R(Args...) & noexcept>
    {
        using return_type = R;
        using arguments = variadic<Args...>;
        constexpr static bool is_const_qualified = false;
        constexpr static bool is_volatile_qualified = false;
        constexpr static bool is_lvalue_ref_qualified = true;
        constexpr static bool is_rvalue_ref_qualified = false;
        constexpr static bool is_noexcept_qualified = true;
        constexpr static bool has_variadic_arguments = false;
    };

    template<typename R, typename... Args>
    struct function_traits<R(Args...) const & noexcept>
    {
        using return_type = R;
        using arguments = variadic<Args...>;
        constexpr static bool is_const_qualified = true;
        constexpr static bool is_volatile_qualified = false;
        constexpr static bool is_lvalue_ref_qualified = true;
        constexpr static bool is_rvalue_ref_qualified = false;
        constexpr static bool is_noexcept_qualified = true;
        constexpr static bool has_variadic_arguments = false;
    };

    template<typename R, typename... Args>
    struct function_traits<R(Args...) volatile & noexcept>
    {
        using return_type = R;
        using arguments = variadic<Args...>;
        constexpr static bool is_const_qualified = false;
        constexpr static bool is_volatile_qualified = true;
        constexpr static bool is_lvalue_ref_qualified = true;
        constexpr static bool is_rvalue_ref_qualified = false;
        constexpr static bool is_noexcept_qualified = true;
        constexpr static bool has_variadic_arguments = false;
    };

    template<typename R, typename... Args>
    struct function_traits<R(Args...) const volatile & noexcept>
    {
        using return_type = R;
        using arguments = variadic<Args...>;
        constexpr static bool is_const_qualified = true;
        constexpr static bool is_volatile_qualified = true;
        constexpr static bool is_lvalue_ref_qualified = true;
        constexpr static bool is_rvalue_ref_qualified = false;
        constexpr static bool is_noexcept_qualified = true;
        constexpr static bool has_variadic_arguments = false;
    };

    template<typename R, typename... Args>
    struct function_traits<R(Args...) && noexcept>
    {
        using return_type = R;
        using arguments = variadic<Args...>;
        constexpr static bool is_const_qualified = false;
        constexpr static bool is_volatile_qualified = false;
        constexpr static bool is_lvalue_ref_qualified = false;
        constexpr static bool is_rvalue_ref_qualified = true;
        constexpr static bool is_noexcept_qualified = true;
        constexpr static bool has_variadic_arguments = false;
    };

    template<typename R, typename... Args>
    struct function_traits<R(Args...) const && noexcept>
    {
        using return_type = R;
        using arguments = variadic<Args...>;
        constexpr static bool is_const_qualified = true;
        constexpr static bool is_volatile_qualified = false;
        constexpr static bool is_lvalue_ref_qualified = false;
        constexpr static bool is_rvalue_ref_qualified = true;
        constexpr static bool is_noexcept_qualified = true;
        constexpr static bool has_variadic_arguments = false;
    };

    template<typename R, typename... Args>
    struct function_traits<R(Args...) volatile && noexcept>
    {
        using return_type = R;
        using arguments = variadic<Args...>;
        constexpr static bool is_const_qualified = false;
        constexpr static bool is_volatile_qualified = true;
        constexpr static bool is_lvalue_ref_qualified = false;
        constexpr static bool is_rvalue_ref_qualified = true;
        constexpr static bool is_noexcept_qualified = true;
        constexpr static bool has_variadic_arguments = false;
    };

    template<typename R, typename... Args>
    struct function_traits<R(Args...) const volatile && noexcept>
    {
        using return_type = R;
        using arguments = variadic<Args...>;
        constexpr static bool is_const_qualified = true;
        constexpr static bool is_volatile_qualified = true;
        constexpr static bool is_lvalue_ref_qualified = false;
        constexpr static bool is_rvalue_ref_qualified = true;
        constexpr static bool is_noexcept_qualified = true;
        constexpr static bool has_variadic_arguments = false;
    };

    template<typename R, typename... Args>
    struct function_traits<R(Args..., ...)>
    {
        using return_type = R;
        using arguments = variadic<Args...>;
        constexpr static bool is_const_qualified = false;
        constexpr static bool is_volatile_qualified = false;
        constexpr static bool is_lvalue_ref_qualified = false;
        constexpr static bool is_rvalue_ref_qualified = false;
        constexpr static bool is_noexcept_qualified = false;
        constexpr static bool has_variadic_arguments = true;
    };

    template<typename R, typename... Args>
    struct function_traits<R(Args..., ...) const>
    {
        using return_type = R;
        using arguments = variadic<Args...>;
        constexpr static bool is_const_qualified = true;
        constexpr static bool is_volatile_qualified = false;
        constexpr static bool is_lvalue_ref_qualified = false;
        constexpr static bool is_rvalue_ref_qualified = false;
        constexpr static bool is_noexcept_qualified = false;
        constexpr static bool has_variadic_arguments = true;
    };

    template<typename R, typename... Args>
    struct function_traits<R(Args..., ...) volatile>
    {
        using return_type = R;
        using arguments = variadic<Args...>;
        constexpr static bool is_const_qualified = false;
        constexpr static bool is_volatile_qualified = true;
        constexpr static bool is_lvalue_ref_qualified = false;
        constexpr static bool is_rvalue_ref_qualified = false;
        constexpr static bool is_noexcept_qualified = false;
        constexpr static bool has_variadic_arguments = true;
    };

    template<typename R, typename... Args>
    struct function_traits<R(Args..., ...) const volatile>
    {
        using return_type = R;
        using arguments = variadic<Args...>;
        constexpr static bool is_const_qualified = true;
        constexpr static bool is_volatile_qualified = true;
        constexpr static bool is_lvalue_ref_qualified = false;
        constexpr static bool is_rvalue_ref_qualified = false;
        constexpr static bool is_noexcept_qualified = false;
        constexpr static bool has_variadic_arguments = true;
    };

    template<typename R, typename... Args>
    struct function_traits<R(Args..., ...)&>
    {
        using return_type = R;
        using arguments = variadic<Args...>;
        constexpr static bool is_const_qualified = false;
        constexpr static bool is_volatile_qualified = false;
        constexpr static bool is_lvalue_ref_qualified = true;
        constexpr static bool is_rvalue_ref_qualified = false;
        constexpr static bool is_noexcept_qualified = false;
        constexpr static bool has_variadic_arguments = true;
    };

    template<typename R, typename... Args>
    struct function_traits<R(Args..., ...) const&>
    {
        using return_type = R;
        using arguments = variadic<Args...>;
        constexpr static bool is_const_qualified = true;
        constexpr static bool is_volatile_qualified = false;
        constexpr static bool is_lvalue_ref_qualified = true;
        constexpr static bool is_rvalue_ref_qualified = false;
        constexpr static bool is_noexcept_qualified = false;
        constexpr static bool has_variadic_arguments = true;
    };

    template<typename R, typename... Args>
    struct function_traits<R(Args..., ...) volatile&>
    {
        using return_type = R;
        using arguments = variadic<Args...>;
        constexpr static bool is_const_qualified = false;
        constexpr static bool is_volatile_qualified = true;
        constexpr static bool is_lvalue_ref_qualified = true;
        constexpr static bool is_rvalue_ref_qualified = false;
        constexpr static bool is_noexcept_qualified = false;
        constexpr static bool has_variadic_arguments = true;
    };

    template<typename R, typename... Args>
    struct function_traits<R(Args..., ...) const volatile&>
    {
        using return_type = R;
        using arguments = variadic<Args...>;
        constexpr static bool is_const_qualified = true;
        constexpr static bool is_volatile_qualified = true;
        constexpr static bool is_lvalue_ref_qualified = true;
        constexpr static bool is_rvalue_ref_qualified = false;
        constexpr static bool is_noexcept_qualified = false;
        constexpr static bool has_variadic_arguments = true;
    };

    template<typename R, typename... Args>
    struct function_traits<R(Args..., ...) &&>
    {
        using return_type = R;
        using arguments = variadic<Args...>;
        constexpr static bool is_const_qualified = false;
        constexpr static bool is_volatile_qualified = false;
        constexpr static bool is_lvalue_ref_qualified = false;
        constexpr static bool is_rvalue_ref_qualified = true;
        constexpr static bool is_noexcept_qualified = false;
        constexpr static bool has_variadic_arguments = true;
    };

    template<typename R, typename... Args>
    struct function_traits<R(Args..., ...) const&&>
    {
        using return_type = R;
        using arguments = variadic<Args...>;
        constexpr static bool is_const_qualified = true;
        constexpr static bool is_volatile_qualified = false;
        constexpr static bool is_lvalue_ref_qualified = false;
        constexpr static bool is_rvalue_ref_qualified = true;
        constexpr static bool is_noexcept_qualified = false;
        constexpr static bool has_variadic_arguments = true;
    };

    template<typename R, typename... Args>
    struct function_traits<R(Args..., ...) volatile&&>
    {
        using return_type = R;
        using arguments = variadic<Args...>;
        constexpr static bool is_const_qualified = false;
        constexpr static bool is_volatile_qualified = true;
        constexpr static bool is_lvalue_ref_qualified = false;
        constexpr static bool is_rvalue_ref_qualified = true;
        constexpr static bool is_noexcept_qualified = false;
        constexpr static bool has_variadic_arguments = true;
    };

    template<typename R, typename... Args>
    struct function_traits<R(Args..., ...) const volatile&&>
    {
        using return_type = R;
        using arguments = variadic<Args...>;
        constexpr static bool is_const_qualified = true;
        constexpr static bool is_volatile_qualified = true;
        constexpr static bool is_lvalue_ref_qualified = false;
        constexpr static bool is_rvalue_ref_qualified = true;
        constexpr static bool is_noexcept_qualified = false;
        constexpr static bool has_variadic_arguments = true;
    };

    template<typename R, typename... Args>
    struct function_traits<R(Args..., ...) noexcept>
    {
        using return_type = R;
        using arguments = variadic<Args...>;
        constexpr static bool is_const_qualified = false;
        constexpr static bool is_volatile_qualified = false;
        constexpr static bool is_lvalue_ref_qualified = false;
        constexpr static bool is_rvalue_ref_qualified = false;
        constexpr static bool is_noexcept_qualified = true;
        constexpr static bool has_variadic_arguments = true;
    };

    template<typename R, typename... Args>
    struct function_traits<R(Args..., ...) const noexcept>
    {
        using return_type = R;
        using arguments = variadic<Args...>;
        constexpr static bool is_const_qualified = true;
        constexpr static bool is_volatile_qualified = false;
        constexpr static bool is_lvalue_ref_qualified = false;
        constexpr static bool is_rvalue_ref_qualified = false;
        constexpr static bool is_noexcept_qualified = true;
        constexpr static bool has_variadic_arguments = true;
    };

    template<typename R, typename... Args>
    struct function_traits<R(Args..., ...) volatile noexcept>
    {
        using return_type = R;
        using arguments = variadic<Args...>;
        constexpr static bool is_const_qualified = false;
        constexpr static bool is_volatile_qualified = true;
        constexpr static bool is_lvalue_ref_qualified = false;
        constexpr static bool is_rvalue_ref_qualified = false;
        constexpr static bool is_noexcept_qualified = true;
        constexpr static bool has_variadic_arguments = true;
    };

    template<typename R, typename... Args>
    struct function_traits<R(Args..., ...) const volatile noexcept>
    {
        using return_type = R;
        using arguments = variadic<Args...>;
        constexpr static bool is_const_qualified = true;
        constexpr static bool is_volatile_qualified = true;
        constexpr static bool is_lvalue_ref_qualified = false;
        constexpr static bool is_rvalue_ref_qualified = false;
        constexpr static bool is_noexcept_qualified = true;
        constexpr static bool has_variadic_arguments = true;
    };

    template<typename R, typename... Args>
    struct function_traits<R(Args..., ...) & noexcept>
    {
        using return_type = R;
        using arguments = variadic<Args...>;
        constexpr static bool is_const_qualified = false;
        constexpr static bool is_volatile_qualified = false;
        constexpr static bool is_lvalue_ref_qualified = true;
        constexpr static bool is_rvalue_ref_qualified = false;
        constexpr static bool is_noexcept_qualified = true;
        constexpr static bool has_variadic_arguments = true;
    };

    template<typename R, typename... Args>
    struct function_traits<R(Args..., ...) const & noexcept>
    {
        using return_type = R;
        using arguments = variadic<Args...>;
        constexpr static bool is_const_qualified = true;
        constexpr static bool is_volatile_qualified = false;
        constexpr static bool is_lvalue_ref_qualified = true;
        constexpr static bool is_rvalue_ref_qualified = false;
        constexpr static bool is_noexcept_qualified = true;
        constexpr static bool has_variadic_arguments = true;
    };

    template<typename R, typename... Args>
    struct function_traits<R(Args..., ...) volatile & noexcept>
    {
        using return_type = R;
        using arguments = variadic<Args...>;
        constexpr static bool is_const_qualified = false;
        constexpr static bool is_volatile_qualified = true;
        constexpr static bool is_lvalue_ref_qualified = true;
        constexpr static bool is_rvalue_ref_qualified = false;
        constexpr static bool is_noexcept_qualified = true;
        constexpr static bool has_variadic_arguments = true;
    };

    template<typename R, typename... Args>
    struct function_traits<R(Args..., ...) const volatile & noexcept>
    {
        using return_type = R;
        using arguments = variadic<Args...>;
        constexpr static bool is_const_qualified = true;
        constexpr static bool is_volatile_qualified = true;
        constexpr static bool is_lvalue_ref_qualified = true;
        constexpr static bool is_rvalue_ref_qualified = false;
        constexpr static bool is_noexcept_qualified = true;
        constexpr static bool has_variadic_arguments = true;
    };

    template<typename R, typename... Args>
    struct function_traits<R(Args..., ...) && noexcept>
    {
        using return_type = R;
        using arguments = variadic<Args...>;
        constexpr static bool is_const_qualified = false;
        constexpr static bool is_volatile_qualified = false;
        constexpr static bool is_lvalue_ref_qualified = false;
        constexpr static bool is_rvalue_ref_qualified = true;
        constexpr static bool is_noexcept_qualified = true;
        constexpr static bool has_variadic_arguments = true;
    };

    template<typename R, typename... Args>
    struct function_traits<R(Args..., ...) const && noexcept>
    {
        using return_type = R;
        using arguments = variadic<Args...>;
        constexpr static bool is_const_qualified = true;
        constexpr static bool is_volatile_qualified = false;
        constexpr static bool is_lvalue_ref_qualified = false;
        constexpr static bool is_rvalue_ref_qualified = true;
        constexpr static bool is_noexcept_qualified = true;
        constexpr static bool has_variadic_arguments = true;
    };

    template<typename R, typename... Args>
    struct function_traits<R(Args..., ...) volatile && noexcept>
    {
        using return_type = R;
        using arguments = variadic<Args...>;
        constexpr static bool is_const_qualified = false;
        constexpr static bool is_volatile_qualified = true;
        constexpr static bool is_lvalue_ref_qualified = false;
        constexpr static bool is_rvalue_ref_qualified = true;
        constexpr static bool is_noexcept_qualified = true;
        constexpr static bool has_variadic_arguments = true;
    };

    template<typename R, typename... Args>
    struct function_traits<R(Args..., ...) const volatile && noexcept>
    {
        using return_type = R;
        using arguments = variadic<Args...>;
        constexpr static bool is_const_qualified = true;
        constexpr static bool is_volatile_qualified = true;
        constexpr static bool is_lvalue_ref_qualified = false;
        constexpr static bool is_rvalue_ref_qualified = true;
        constexpr static bool is_noexcept_qualified = true;
        constexpr static bool has_variadic_arguments = true;
    };

    template<typename F>
    requires std::is_function_v<F>
    constexpr inline bool is_function_const_qualified = function_traits<F>::is_const_qualified;

    template<typename F>
    requires std::is_function_v<F>
    constexpr inline bool is_function_volatile_qualified =
        function_traits<F>::is_volatile_qualified;

    template<typename F>
    requires std::is_function_v<F>
    constexpr inline bool is_function_lvalue_ref_qualified =
        function_traits<F>::is_lvalue_ref_qualified;

    template<typename F>
    requires std::is_function_v<F>
    constexpr inline bool is_function_rvalue_ref_qualified =
        function_traits<F>::is_rvalue_ref_qualified;

    template<typename F>
    requires std::is_function_v<F>
    constexpr inline bool is_function_noexcept_qualified =
        function_traits<F>::is_noexcept_qualified;

    template<typename F>
    requires std::is_function_v<F>
    constexpr inline bool is_function_has_variadic_arguments =
        function_traits<F>::has_variadic_arguments;

    template<typename F>
    requires std::is_function_v<F>
    using function_arguments = function_traits<F>::arguments;
}
