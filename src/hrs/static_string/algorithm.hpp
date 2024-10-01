#pragma once

#include "static_string.hpp"

namespace hrs
{
	namespace detail
	{
		template<hrs::static_string str, std::size_t Index, decltype(str)::char_t value, decltype(str)::char_t ...values>
		constexpr bool trim_check_value() noexcept
		{
			if constexpr(sizeof...(values) == 0)
				return str.data[Index] == value;
			else
				return trim_check_value<str, Index, values...>();
		}

		template<hrs::static_string str, std::size_t Index, bool dir_front, decltype(str)::char_t ...values>
		constexpr std::size_t trim_find_index() noexcept
		{
			if constexpr(dir_front)
			{
				if constexpr(Index == str.size)
					return Index;
				else
				{
					if constexpr(trim_check_value<str, Index, values...>())
						return trim_find_index<str, Index + 1, dir_front, values...>();
					else
						return Index;
				}
			}
			else
			{
				if constexpr(Index == 0)
					return Index;
				else
				{
					if constexpr(trim_check_value<str, Index - 1, values...>())
						return trim_find_index<str, Index - 1, dir_front, values...>();
					else
						return Index;
				}
			}
		}

		template<std::size_t str_size, bool dir_front>
		constexpr std::size_t select_trim_start_index() noexcept
		{
			if constexpr(dir_front)
				return 0;
			else
				return str_size;
		}

		template<static_string _haystack, std::size_t Start_Index, std::size_t End_Index>
		constexpr auto find_bound_cursor_set_value() noexcept
		{
			using haystack_t = decltype(_haystack);
			if constexpr(Start_Index == haystack_t::npos || End_Index == haystack_t::npos)
				return std::byte{0};
			else
				return _haystack.template substr<Start_Index, End_Index - Start_Index>();
		}
	};

	template<hrs::static_string str, bool dir_front, decltype(str)::char_t ...values>
	constexpr auto trim() noexcept
	{
		if constexpr(sizeof...(values) == 0 || str.size == 0)
			return str;
		else
		{
			constexpr std::size_t index = detail::trim_find_index<str,
																  detail::select_trim_start_index<str.size,
																								  dir_front>(),
																  dir_front, values...>();
			if constexpr(dir_front)
				return str.template substr_back<str.size - index>();
			else
				return str.template substr_front<index>();
		}
	}


	template<static_string _haystack, static_string _needle, std::size_t _Index>
	struct find_cursor
	{
		using haystack_t = decltype(_haystack);
		using needle_t = decltype(_needle);

		constexpr static auto haystack = _haystack;
		constexpr static auto needle = _needle;

		constexpr static auto value = _Index;


		constexpr static auto next() noexcept
		{
			using haystack_t = decltype(haystack);
			if constexpr(_Index == haystack_t::npos)
				return find_cursor{};
			else
			{
				constexpr std::size_t index = haystack.template find<_Index + needle.size>(needle);
				return find_cursor<haystack, needle, index>{};
			}
		}

		constexpr static bool is_end() noexcept
		{
			return _Index == haystack_t::npos;
		}
	};

	template<find_cursor _fit, static_string _value, bool _is_end>
	struct split_cursor
	{
		using find_cursor_t = decltype(_fit);
		using value_t = decltype(_value);

		constexpr static auto fit = _fit;
		constexpr static auto value = _value;

		constexpr static auto next() noexcept
		{
			if constexpr(fit.is_end())
				return split_cursor<fit, _value, true>{};
			else
			{
				constexpr auto it = fit.next();
				constexpr auto value = it.haystack.template substr<fit.value + fit.needle.size, it.value - (fit.value + fit.needle.size)>();
				return split_cursor<it, value, false>{};
			}
		}

		constexpr static bool is_end() noexcept
		{
			return _is_end;
		}
	};

	template<static_string _haystack, static_string _start, static_string _end, std::size_t _Start_Index, std::size_t _End_Index>
	struct find_bound_cursor
	{
		using haystack_t = decltype(_haystack);
		//using needle_t = decltype(_needle);

		constexpr static auto haystack = _haystack;
		//constexpr static auto needle = _needle;

			   //constexpr static auto value = std::pair{_Start_Index, _End_Index};
		constexpr static auto value = detail::find_bound_cursor_set_value<_haystack, _Start_Index, _End_Index>();

		constexpr static std::size_t StartIndex = _Start_Index;
		constexpr static std::size_t EndIndex = _End_Index;

			   //Hello {.0x} from the {255}!
			   //                      s  e
		constexpr static auto next() noexcept
		{
			using haystack_t = decltype(haystack);
			if constexpr(_Start_Index == haystack_t::npos || _End_Index == haystack_t::npos)
				return find_bound_cursor{};
			else
			{
				constexpr std::size_t start_index = haystack.template find<_End_Index + 1>(_start);
				if constexpr(start_index == haystack_t::npos)
				{
					constexpr std::size_t end_index = haystack.template find<_End_Index + 1>(_end);
					return find_bound_cursor<_haystack, _start, _end, haystack_t::npos, end_index>{};
				}
				else
				{
					constexpr std::size_t end_index = haystack.template find<start_index + 1>(_end);
					if constexpr(end_index == haystack_t::npos)
						return find_bound_cursor<_haystack, _start, _end, start_index + 1, haystack_t::npos>{};
					else
						return find_bound_cursor<_haystack, _start, _end, start_index + 1, end_index>{};
				}
			}
		}

		constexpr static bool is_end() noexcept
		{
			return _Start_Index == haystack_t::npos || _End_Index == haystack_t::npos;
		}

		constexpr static bool is_fulfilled_bound() noexcept
		{
			return _Start_Index != haystack_t::npos && _End_Index != haystack_t::npos;
		}
	};

	template<static_string haystack, static_string start, static_string end>
	constexpr auto find_bound_cursor_create() noexcept
	{
		using haystack_t = decltype(haystack);
		constexpr std::size_t start_index = haystack.template find(start);
		if constexpr(start_index == haystack_t::npos)
		{
			constexpr std::size_t end_index = haystack.template find(end);
			return find_bound_cursor<haystack, start, end, haystack_t::npos, end_index>{};
		}
		else
		{
			constexpr std::size_t end_index = haystack.template find<start_index + 1>(end);
			if constexpr(end_index == haystack_t::npos)
				return find_bound_cursor<haystack, start, end, start_index + 1, haystack_t::npos>{};
			else
				return find_bound_cursor<haystack, start, end, start_index + 1, end_index>{};
		}
	}

	template<static_string haystack, static_string needle>
	constexpr auto find_cursor_create() noexcept
	{
		return find_cursor<haystack, needle, haystack.find(needle)>{};
	}

	template<static_string haystack, static_string needle>
	constexpr auto split_cursor_create() noexcept
	{
		constexpr auto it = find_cursor_create<haystack, needle>();
		if constexpr(it.is_end())
		{
			constexpr auto value = haystack.template substr<0, haystack.size>();
			return split_cursor<it, value, false>{};
		}
		else
		{
			constexpr auto value = haystack.template substr<0, it.value>();
			return split_cursor<it, value, false>{};
		}
	}

	template<auto cursor, template<auto ...> typename F, auto ...results>
	constexpr auto loop_and_yield() noexcept
	{
		if constexpr(cursor.is_end())
			return F<results...>::yield();
		else
		{
			return loop_and_yield<cursor.next(), F, results..., cursor.value>();
		}
	}

	template<auto cursor, template<typename ...> typename F, typename ...results>
	constexpr auto loop_and_yield() noexcept
	{
		if constexpr(cursor.is_end())
			return F<results...>::yield();
		else
		{
			return loop_and_yield<cursor.next(), F, results..., decltype(cursor.value)>();
		}
	}
};
