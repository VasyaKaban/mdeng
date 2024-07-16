#pragma once

#include "../static_string/algorithm.hpp"

namespace hrs
{
	namespace detail
	{
		template<auto ...names>
		struct names_collector
		{
			constexpr static auto yield() noexcept
			{
				return std::tuple{hrs::trim<hrs::trim<names, true, ' '>(), false, ' '>()...};
			}
		};

		template<hrs::static_string str>
		constexpr auto split_namespace() noexcept
		{
			constexpr auto sc = hrs::split_cursor_create<str, "::">();
			return hrs::loop_and_yield<sc, names_collector>();
		}

		template<hrs::static_string str>
		constexpr auto retrieve_namespace_and_name() noexcept
		{
			constexpr auto nss = split_namespace<str>();
			if constexpr(std::tuple_size_v<decltype(nss)> == 1)
				return std::pair{std::tuple{}, std::get<0>(nss)};
			else
			{
				constexpr auto make_pair = []<std::size_t ...Indices>(const auto &namespaces,
																	  std::index_sequence<Indices...>)
				{
					return std::pair{std::tuple{std::get<Indices>(namespaces)...},
									 std::get<std::tuple_size_v<std::remove_cvref_t<decltype(namespaces)>> - 1>(namespaces)};
				};

				if constexpr(std::get<0>(nss).size == 0)
				{
					constexpr auto seq = std::make_index_sequence<std::tuple_size_v<decltype(nss)> - 2>{};
					constexpr auto create_index_seq = []<std::size_t ...Indices>(std::index_sequence<Indices...>)
					{
						return std::integer_sequence<std::size_t, (Indices + 1)...>{};
					};

					return make_pair(nss, create_index_seq(seq));
				}
				else
					return make_pair(nss, std::make_index_sequence<std::tuple_size_v<decltype(nss)> - 1>{});
			}
		}
	};
};
