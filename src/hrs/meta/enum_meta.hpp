#pragma once

#include <type_traits>
#include <optional>
#include <string_view>
#include "../variadic.hpp"
#include "../static_string/algorithm.hpp"
#include "split_namespace.hpp"
#include "meta_attributes.hpp"

namespace hrs
{
	template<typename E>
		requires std::is_enum_v<E>
	struct enum_meta;

	namespace detail
	{
		template<hrs::static_string ...names>
		struct enum_names_collector
		{
			constexpr static auto yield() noexcept
			{
				return std::tuple{retrieve_namespace_and_name<names>().second...};
			}
		};

		template<hrs::static_string names>
		constexpr auto collect_enum_names() noexcept
		{
			if constexpr(names == "")
				return std::tuple<>{};
			else
			{
				constexpr auto sc = hrs::split_cursor_create<names, ",">();
				return hrs::loop_and_yield<sc, enum_names_collector>();
			}
		}

		template<typename E,
				  hrs::static_string _name,
				  hrs::static_string _names,
				  E ..._values>
			struct enum_meta_base
		{
			using refl_enum = E;
			constexpr static auto values = std::tuple{_values...};
			constexpr static auto names = collect_enum_names<_names>();
			constexpr static std::size_t count = std::tuple_size_v<decltype(names)>;
			constexpr static auto name = detail::retrieve_namespace_and_name<_name>().second;
			constexpr static auto namespaces = detail::retrieve_namespace_and_name<_name>().first;

			template<std::size_t Index>
			constexpr static std::string_view get_name(E value) noexcept
			{
				if constexpr(Index == count)
					return "";
				else
				{
					const auto &pvalue = std::get<Index>(values);
					const auto &pname = std::get<Index>(names);
					if(value == pvalue)
						return std::string_view{pname.data, pname.size};
					else
						return get_name<Index + 1>(value);
				}
			}

			constexpr static std::string_view get_name(E value) noexcept
			{
				return get_name<0>(value);
			}

			template<std::size_t Index>
			constexpr static std::optional<E> get_value(std::string_view name) noexcept
			{
				if constexpr(Index == count)
					return {};
				else
				{
					const auto &pvalue = std::get<Index>(values);
					const auto &pname = std::get<Index>(names);
					std::string_view target_name{pname.data, pname.size};
					if(name == target_name)
						return pvalue;
					else
						return get_value<Index + 1>(name);
				}
			}

			constexpr static std::optional<E> get_value(std::string_view name) noexcept
			{
				return get_value<0>(name);
			}

			template<std::size_t Index, E value>
			constexpr static auto get_name() noexcept
			{
				static_assert(Index != count, "No name for desired value!");
				const auto &pvalue = std::get<Index>(values);
				const auto &pname = std::get<Index>(names);
				if constexpr(value == pvalue)
					return pname;
				else
					return get_name<Index + 1, value>();
			}

			template<E value>
			constexpr static auto get_name() noexcept
			{
				return get_name<0, value>();
			}

			template<std::size_t Index, hrs::static_string name>
			constexpr static E get_value() noexcept
			{
				static_assert(Index != count, "No value for desired name!");
				const auto &pvalue = std::get<Index>(values);
				const auto &pname = std::get<Index>(names);
				if constexpr(name == pname)
					return pvalue;
				else
					return get_value<Index + 1, name>();
			}

			template<hrs::static_string name>
			constexpr static E get_value() noexcept
			{
				return get_value<0, name>();
			}
		};
	};

	template<typename E>
		requires std::is_enum_v<E>
	struct enum_meta
		: detail::enum_meta_base<E, "", "">,
		  meta_attributes<> {};
};
