#pragma once

#include <cstddef>
#include <type_traits>
#include <utility>

namespace hrs
{
	namespace detail
	{
		template<std::size_t N, typename T, typename ...Args>
			requires(N < sizeof...(Args) + 1)
		struct nth
		{
			using type = nth<N - 1, Args...>::type;
		};

		template<typename T, typename ...Args>
		struct nth<0, T, Args...>
		{
			using type = T;
		};

		template<std::size_t N, typename T, typename ...Args>
			requires(N < sizeof...(Args) + 1)
		using nth_t = nth<N, T, Args...>;
	};

	template<typename ...Args>
	struct variadic
	{
		constexpr static std::size_t COUNT = sizeof...(Args);

		template<std::size_t Index>
			requires (Index < COUNT)
		struct nth
		{
			using type = detail::nth<Index, Args...>::type;
		};

		template<std::size_t Index>
			requires (Index < COUNT)
		using nth_t = nth<Index>::type;

		template<template<typename ...> typename F>
		constexpr static auto apply() noexcept
		{
			return F<Args...>();
		}
	};

	template<typename ...Args1, typename ...Args2>
	constexpr auto operator+(variadic<Args1...>, variadic<Args2...>) noexcept
	{
		return variadic<Args1..., Args2...>{};
	}

	template<std::size_t Index, typename A, typename ...Args>
	constexpr auto & nth_argument(A &&arg, Args &&...args) noexcept
	{
		if constexpr(Index == 0)
			return arg;
		else
			return nth_argument<Index - 1>(std::forward<Args>(args)...);
	}
};
