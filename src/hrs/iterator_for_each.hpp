#pragma once

#include <ranges>

namespace hrs
{
	template<std::ranges::input_range R, typename F>
		requires
			std::is_invocable_r_v<void, F, std::ranges::iterator_t<R>> ||
			std::is_invocable_r_v<bool, F, std::ranges::iterator_t<R>>
	void iterator_for_each(R &&rng, const F &func)
		noexcept(std::is_nothrow_invocable_r_v<void, F, std::ranges::iterator_t<R>> ||
				 std::is_nothrow_invocable_r_v<bool, F, std::ranges::iterator_t<R>>										   )
	{
		auto begin_it = std::ranges::begin(std::forward<R>(rng));
		auto end_it = std::ranges::end(std::forward<R>(rng));

		for(; begin_it != end_it; begin_it++)
		{
			if constexpr(std::is_invocable_r_v<void, F, std::ranges::iterator_t<R>>)
				func(begin_it);
			else
			{
				bool res = func(begin_it);
				if(res)
					break;
			}
		}
	}
};
