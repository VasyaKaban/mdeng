#pragma once

#include <cstdint>

namespace hrs
{
	template<std::size_t Index, typename T, typename ...Args>
		requires (Index < (sizeof...(Args) + 1))
	struct variadic_arg_type
	{
		using type = typename variadic_arg_type<Index - 1, Args...>::type;
	};

	template<typename T, typename ...Args>
	struct variadic_arg_type<0, T, Args...>
	{
		using type = T;
	};

	template<std::size_t Index, typename ...Args>
		requires (Index < sizeof...(Args))
	using variadic_arg_type_t = typename variadic_arg_type<Index, Args...>::type;

	template<typename ...Args>
	struct variadic
	{
		template<std::size_t Index>
			requires (Index < sizeof...(Args))
		using arg = variadic_arg_type_t<Index, Args...>;

		constexpr static std::size_t SIZE = sizeof...(Args);
	};
};
