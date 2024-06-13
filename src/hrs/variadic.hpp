#pragma once

#include <cstddef>

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
	};
};
