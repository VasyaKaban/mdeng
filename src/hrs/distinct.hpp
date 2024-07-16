#pragma once

#include <tuple>

namespace hrs
{
	namespace detail
	{
		template<auto value, std::size_t Index, std::size_t Cmp_Index, auto cmp, auto ...values>
			requires
				requires(decltype(value) v, decltype(cmp) c)
				{
					v == c;
				}
		constexpr bool is_distinct_non_type_value() noexcept
		{
			if constexpr(value == cmp && Index != Cmp_Index)
				return false;
			else
			{
				if constexpr(sizeof...(values) != 0)
					return is_distinct_non_type_value<value, Index, Cmp_Index + 1, values...>();
				else
					return true;
			}
		}

		template<auto ...values>
		constexpr bool is_distinct_non_type() noexcept
		{
			if constexpr(sizeof...(values) == 0)
				return false;
			else
			{
				constexpr auto tpl = std::tuple{values...};
				return [&tpl]<std::size_t ...Indices>(const std::index_sequence<Indices...> &)
				{
					return (is_distinct_non_type_value<std::get<Indices>(tpl), Indices, 0, values...>() && ...);
				}(std::make_index_sequence<sizeof...(values)>());
			}
		}
	};

	template<auto ...values>
	concept distinct_non_type = detail::is_distinct_non_type<values...>();
};
