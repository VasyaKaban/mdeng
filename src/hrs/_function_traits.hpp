#pragma once

#include <type_traits>
#include "variadic.hpp"

/*

empty,
const,
volatile,
noexcept,
&,
&&,
...,

//48

//empty
//...

//cv:
const,
const ...,
volatile,
volatile ...,
const volatile,
const volatile ...,

//ref + cv
&,
& ...,
const &,
const & ...,
volatile &,
volatile & ...,
const volatile &,
const volatile & ...,
&&,
&& ...,
const &&,
const && ...,
volatile &&,
volatile && ...,
const volatile &&,
const volatile && ...,

//noexcept + cv
noexcept,
noexcept ...,
const noexcept,
const noexcept ...,
volatile noexcept,
volatile noexcept ...,
const volatile noexcept,
const volatile noexcept ...,

//noexcept + ref
noexcept &,
noexcept & ...,
const noexcept &,
const noexcept & ...,
volatile noexcept &,
volatile noexcept & ...,
const volatile noexcept &,
const volatile noexcept & ...,

noexcept &&,
noexcept && ...,
const noexcept &&,
const noexcept && ...,
volatile noexcept &&,
volatile noexcept && ...,
const volatile noexcept &&,
const volatile noexcept && ...,


 */
namespace hrs
{
	template<typename F>
		requires std::is_function_v<F>
	struct function_traits;

	template<typename F>
		requires std::is_member_function_pointer_v<F>
	struct member_function_traits;

	template<typename C, typename F>
	struct member_function_traits<F C::*> : function_traits<F>
	{
		using class_type = C;
	};

	template<typename F>
		requires std::is_function_v<F>
	constexpr inline bool is_function_const_qualified =
		function_traits<F>::is_const_qualified;

	template<typename F>
		requires std::is_function_v<F>
	constexpr inline bool is_function_volatile_qualified =
		function_traits<F>::is_volatile_qualified;

	template<typename F>
		requires std::is_function_v<F>
	constexpr inline bool is_function_lvalue_reference_qualified =
		function_traits<F>::is_lvalue_reference_qualified;

	template<typename F>
		requires std::is_function_v<F>
	constexpr inline bool is_function_rvalue_reference_qualified =
		function_traits<F>::is_rvalue_reference_qualified;

	template<typename F>
		requires std::is_function_v<F>
	constexpr inline bool is_function_noexcept_qualified =
		function_traits<F>::is_noexcept_qualified;

	template<typename F>
		requires std::is_function_v<F>
	constexpr inline bool function_has_varidic_arguments =
		function_traits<F>::has_variadic_arguments;

	template<typename F>
		requires std::is_function_v<F>
	using function_return_type = typename function_traits<F>::return_type;

	template<typename F>
		requires std::is_function_v<F>
	using function_arguments = typename function_traits<F>::arguments;

	template<typename F>
		requires std::is_member_function_pointer_v<F>
	constexpr inline bool is_member_function_const_qualified =
		member_function_traits<F>::is_const_qualified;

	template<typename F>
		requires std::is_member_function_pointer_v<F>
	constexpr inline bool is_member_function_volatile_qualified =
		member_function_traits<F>::is_volatile_qualified;

	template<typename F>
		requires std::is_member_function_pointer_v<F>
	constexpr inline bool is_member_function_lvalue_reference_qualified =
		member_function_traits<F>::is_lvalue_reference_qualified;

	template<typename F>
		requires std::is_member_function_pointer_v<F>
	constexpr inline bool is_member_function_rvalue_reference_qualified =
		member_function_traits<F>::is_rvalue_reference_qualified;

	template<typename F>
		requires std::is_member_function_pointer_v<F>
	constexpr inline bool is_member_function_noexcept_qualified =
		member_function_traits<F>::is_noexcept_qualified;

	template<typename F>
		requires std::is_member_function_pointer_v<F>
	constexpr inline bool member_function_has_varidic_arguments =
		member_function_traits<F>::has_variadic_arguments;

	template<typename F>
		requires std::is_member_function_pointer_v<F>
	using member_function_return_type = typename member_function_traits<F>::return_type;

	template<typename F>
		requires std::is_member_function_pointer_v<F>
	using member_function_arguments = typename member_function_traits<F>::arguments;

	template<typename F>
		requires std::is_member_function_pointer_v<F>
	using member_function_class = typename member_function_traits<F>::class_type;

	//empty
	template<typename R, typename ...Args>
	struct function_traits<R (Args...)>
	{
		constexpr static bool is_const_qualified = false;
		constexpr static bool is_volatile_qualified = false;
		constexpr static bool is_lvalue_reference_qualified = false;
		constexpr static bool is_rvalue_reference_qualified = false;
		constexpr static bool is_noexcept_qualified = false;
		constexpr static bool has_variadic_arguments = false;
		using return_type = R;
		using arguments = variadic<Args...>;
	};

	//...
	template<typename R, typename ...Args>
	struct function_traits<R (Args..., ...)> : function_traits<R (Args...)>
	{
		constexpr static bool has_variadic_arguments = true;
	};

	//const
	template<typename R, typename ...Args>
	struct function_traits<R (Args...) const> : function_traits<R (Args...)>
	{
		constexpr static bool is_const_qualified = true;
	};

	//const ...
	template<typename R, typename ...Args>
	struct function_traits<R (Args..., ...) const> : function_traits<R (Args..., ...)>
	{
		constexpr static bool is_const_qualified = true;
	};

	//volatile
	template<typename R, typename ...Args>
	struct function_traits<R (Args...) volatile> : function_traits<R (Args...)>
	{
		constexpr static bool is_volatile_qualified = true;
	};

	//volatile ...
	template<typename R, typename ...Args>
	struct function_traits<R (Args..., ...) volatile> : function_traits<R (Args..., ...)>
	{
		constexpr static bool is_volatile_qualified = true;
	};

	//const volatile
	template<typename R, typename ...Args>
	struct function_traits<R (Args...) const volatile> : function_traits<R (Args...) const>
	{
		constexpr static bool is_volatile_qualified = true;
	};

	//const volatile ...
	template<typename R, typename ...Args>
	struct function_traits<R (Args..., ...) const volatile> : function_traits<R (Args..., ...) const>
	{
		constexpr static bool is_volatile_qualified = true;
	};

	//&,
	template<typename R, typename ...Args>
	struct function_traits<R (Args...) &> : function_traits<R (Args...)>
	{
		constexpr static bool is_lvalue_reference_qualified = true;
	};

	//& ...,
	template<typename R, typename ...Args>
	struct function_traits<R (Args..., ...) &> : function_traits<R (Args..., ...)>
	{
		constexpr static bool is_lvalue_reference_qualified = true;
	};

	//const &,
	template<typename R, typename ...Args>
	struct function_traits<R (Args...) const &> : function_traits<R (Args...) const>
	{
		constexpr static bool is_lvalue_reference_qualified = true;
	};

	//const & ...,
	template<typename R, typename ...Args>
	struct function_traits<R (Args..., ...) const &> : function_traits<R (Args..., ...) const >
	{
		constexpr static bool is_lvalue_reference_qualified = true;
	};

	//volatile &,
	template<typename R, typename ...Args>
	struct function_traits<R (Args...) volatile &> : function_traits<R (Args...) volatile>
	{
		constexpr static bool is_lvalue_reference_qualified = true;
	};

	//volatile & ...,
	template<typename R, typename ...Args>
	struct function_traits<R (Args..., ...) volatile &> : function_traits<R (Args..., ...) volatile>
	{
		constexpr static bool is_lvalue_reference_qualified = true;
	};

	//const volatile &,
	template<typename R, typename ...Args>
	struct function_traits<R (Args...) const volatile &> : function_traits<R (Args...)  const volatile>
	{
		constexpr static bool is_lvalue_reference_qualified = true;
	};

	//const volatile & ...,
	template<typename R, typename ...Args>
	struct function_traits<R (Args..., ...) const volatile &> : function_traits<R (Args..., ...) const volatile>
	{
		constexpr static bool is_lvalue_reference_qualified = true;
	};

	//&&,
	template<typename R, typename ...Args>
	struct function_traits<R (Args...) &&> : function_traits<R (Args...)>
	{
		constexpr static bool is_rvalue_reference_qualified = true;
	};

	//&& ...,
	template<typename R, typename ...Args>
	struct function_traits<R (Args..., ...) &&> : function_traits<R (Args..., ...)>
	{
		constexpr static bool is_rvalue_reference_qualified = true;
	};

	//const &&,
	template<typename R, typename ...Args>
	struct function_traits<R (Args...) const &&> : function_traits<R (Args...) const>
	{
		constexpr static bool is_rvalue_reference_qualified = true;
	};

	//const && ...,
	template<typename R, typename ...Args>
	struct function_traits<R (Args..., ...) const &&> : function_traits<R (Args..., ...) const >
	{
		constexpr static bool is_rvalue_reference_qualified = true;
	};

	//volatile &&,
	template<typename R, typename ...Args>
	struct function_traits<R (Args...) volatile &&> : function_traits<R (Args...) volatile>
	{
		constexpr static bool is_rvalue_reference_qualified = true;
	};

	//volatile && ...,
	template<typename R, typename ...Args>
	struct function_traits<R (Args..., ...) volatile &&> : function_traits<R (Args..., ...) volatile>
	{
		constexpr static bool is_rvalue_reference_qualified = true;
	};

	//const volatile &&,
	template<typename R, typename ...Args>
	struct function_traits<R (Args...) const volatile &&> : function_traits<R (Args...)  const volatile>
	{
		constexpr static bool is_rvalue_reference_qualified = true;
	};

	//const volatile && ...,
	template<typename R, typename ...Args>
	struct function_traits<R (Args..., ...) const volatile &&> : function_traits<R (Args..., ...) const volatile>
	{
		constexpr static bool is_rvalue_reference_qualified = true;
	};

	//noexcept
	template<typename R, typename ...Args>
	struct function_traits<R (Args...) noexcept> : function_traits<R (Args...)>
	{
		constexpr static bool is_noexcept_qualified = true;
	};

	//noexcept...,
	template<typename R, typename ...Args>
	struct function_traits<R (Args..., ...) noexcept> : function_traits<R (Args..., ...)>
	{
		constexpr static bool is_noexcept_qualified = true;
	};

	//const noexcept
	template<typename R, typename ...Args>
	struct function_traits<R (Args...) const noexcept> : function_traits<R (Args...) const>
	{
		constexpr static bool is_noexcept_qualified = true;
	};

	//const noexcept...,
	template<typename R, typename ...Args>
	struct function_traits<R (Args..., ...) const noexcept> : function_traits<R (Args..., ...) const>
	{
		constexpr static bool is_noexcept_qualified = true;
	};

	//volatile noexcept
	template<typename R, typename ...Args>
	struct function_traits<R (Args...) volatile noexcept> : function_traits<R (Args...) volatile>
	{
		constexpr static bool is_noexcept_qualified = true;
	};

	//volatile noexcept...,
	template<typename R, typename ...Args>
	struct function_traits<R (Args..., ...) volatile noexcept> : function_traits<R (Args..., ...) volatile>
	{
		constexpr static bool is_noexcept_qualified = true;
	};

	//const volatile noexcept
	template<typename R, typename ...Args>
	struct function_traits<R (Args...) const volatile noexcept> : function_traits<R (Args...) const volatile>
	{
		constexpr static bool is_noexcept_qualified = true;
	};

	//const volatile noexcept...,
	template<typename R, typename ...Args>
	struct function_traits<R (Args..., ...) const volatile noexcept>
		: function_traits<R (Args..., ...)const volatile >
	{
		constexpr static bool is_noexcept_qualified = true;
	};

	//noexcept &
	template<typename R, typename ...Args>
	struct function_traits<R (Args...) & noexcept> : function_traits<R (Args...) &>
	{
		constexpr static bool is_noexcept_qualified = true;
	};

	//noexcept & ...,
	template<typename R, typename ...Args>
	struct function_traits<R (Args..., ...) & noexcept> : function_traits<R (Args..., ...) &>
	{
		constexpr static bool is_noexcept_qualified = true;
	};

	//const noexcept &
	template<typename R, typename ...Args>
	struct function_traits<R (Args...) const & noexcept> : function_traits<R (Args...) const &>
	{
		constexpr static bool is_noexcept_qualified = true;
	};

	//const noexcept & ...,
	template<typename R, typename ...Args>
	struct function_traits<R (Args..., ...) const & noexcept> : function_traits<R (Args..., ...) const &>
	{
		constexpr static bool is_noexcept_qualified = true;
	};

	//volatile noexcept &
	template<typename R, typename ...Args>
	struct function_traits<R (Args...) volatile & noexcept> : function_traits<R (Args...) volatile &>
	{
		constexpr static bool is_noexcept_qualified = true;
	};

	//volatile noexcept & ...,
	template<typename R, typename ...Args>
	struct function_traits<R (Args..., ...) volatile & noexcept> : function_traits<R (Args..., ...) volatile &>
	{
		constexpr static bool is_noexcept_qualified = true;
	};

	//const volatile noexcept &
	template<typename R, typename ...Args>
	struct function_traits<R (Args...) const volatile & noexcept> : function_traits<R (Args...) const volatile &>
	{
		constexpr static bool is_noexcept_qualified = true;
	};

	//const volatile noexcept & ...,
	template<typename R, typename ...Args>
	struct function_traits<R (Args..., ...) const volatile & noexcept>
		: function_traits<R (Args..., ...) const volatile &>
	{
		constexpr static bool is_noexcept_qualified = true;
	};

	//noexcept &&
	template<typename R, typename ...Args>
	struct function_traits<R (Args...) && noexcept> : function_traits<R (Args...) &&>
	{
		constexpr static bool is_noexcept_qualified = true;
	};

	//noexcept && ...,
	template<typename R, typename ...Args>
	struct function_traits<R (Args..., ...) && noexcept> : function_traits<R (Args..., ...) &&>
	{
		constexpr static bool is_noexcept_qualified = true;
	};

	//const noexcept &&
	template<typename R, typename ...Args>
	struct function_traits<R (Args...) const && noexcept> : function_traits<R (Args...) const &&>
	{
		constexpr static bool is_noexcept_qualified = true;
	};

	//const noexcept && ...,
	template<typename R, typename ...Args>
	struct function_traits<R (Args..., ...) const && noexcept> : function_traits<R (Args..., ...) const &&>
	{
		constexpr static bool is_noexcept_qualified = true;
	};

	//volatile noexcept &&
	template<typename R, typename ...Args>
	struct function_traits<R (Args...) volatile && noexcept> : function_traits<R (Args...) volatile &&>
	{
		constexpr static bool is_noexcept_qualified = true;
	};

	//volatile noexcept && ...,
	template<typename R, typename ...Args>
	struct function_traits<R (Args..., ...) volatile && noexcept> : function_traits<R (Args..., ...) volatile &&>
	{
		constexpr static bool is_noexcept_qualified = true;
	};

	//const volatile noexcept &&
	template<typename R, typename ...Args>
	struct function_traits<R (Args...) const volatile && noexcept> : function_traits<R (Args...) const volatile &&>
	{
		constexpr static bool is_noexcept_qualified = true;
	};

	//const volatile noexcept && ...,
	template<typename R, typename ...Args>
	struct function_traits<R (Args..., ...) const volatile && noexcept>
		: function_traits<R (Args..., ...) const volatile &&>
	{
		constexpr static bool is_noexcept_qualified = true;
	};
};
