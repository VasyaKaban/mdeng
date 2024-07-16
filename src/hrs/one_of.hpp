#pragma once

#include <concepts>
#include <type_traits>

namespace hrs
{
	template<typename T, typename ...Types>
	concept one_of_type = (std::same_as<T, Types> || ...);

	template<auto t, auto u>
	constexpr bool is_same_non_type() noexcept
	{
		if constexpr(requires {t == u;})
		{
			return t == u;
		}
		else
			return false;
	}

	template<auto val, auto ...values>
	concept one_of_non_type = (is_same_non_type<val, values>() || ...);
};
