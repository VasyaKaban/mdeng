#pragma once

#include <tuple>

namespace hrs
{
	template<typename T>
	concept tuple_like = requires(T &&tpl)
	{
		{std::tuple_size_v<std::remove_cvref_t<T>>} -> std::convertible_to<std::size_t>;
		std::get<0>(std::forward<T>(tpl));
		typename std::tuple_element_t<0, std::remove_cvref_t<T>>;
	};

	template<typename F, typename T, typename ...Args>
	constexpr void tuple_loop_one_element(F &&func, T &&arg, Args &&...args)
	{
		std::forward<F>(func)(std::forward<T>(arg));
		if constexpr(sizeof...(Args) != 0)
			tuple_loop_one_element(std::forward<F>(func), std::forward<Args>(args)...);
	}

	template<tuple_like T, typename F>
		//requires (std::invocable<F, Args> && ...)
	constexpr void tuple_loop(T &&tpl, F &&func)
	{
		std::apply([&]<typename ...Args>(Args &&...args)
		{
			static_assert((std::invocable<F, Args> && ...));

			if constexpr(sizeof...(Args) == 0)
				return;

			tuple_loop_one_element(std::forward<F>(func), std::forward<Args>(args)...);
		}, std::forward<T>(tpl));
	}

	template<typename F, typename T, typename ...Args>
	constexpr void tuple_iterate_one_element(F &&func, T &&arg, Args &&...args)
	{
		if(!std::forward<F>(func)(std::forward<T>(arg)))
			return;

		if constexpr(sizeof...(Args) != 0)
			return tuple_iterate_one_element(std::forward<F>(func),
											 std::forward<Args>(args)...);
	}

	template<tuple_like T, typename F>
	constexpr void tuple_iterate(T &&tpl, F &&func)
	{
		std::apply([&]<typename ...Args>(Args &&...args)
		{
			static_assert((std::invocable<F, Args> && ...) &&
						  (std::convertible_to<std::invoke_result_t<F, Args>, bool> && ...));

			if constexpr(sizeof...(Args) == 0)
				return;

			tuple_iterate_one_element(std::forward<F>(func), std::forward<Args>(args)...);
		}, std::forward<T>(tpl));
	}
};
