#pragma once

#include <concepts>
#include "debug.hpp"

namespace hrs
{
	template<std::unsigned_integral T>
	constexpr auto is_power_of_two(T alignment) noexcept -> bool
	{
		assert_true_debug(alignment != 0, "Alignment must be > 0!");
		return (alignment & (alignment - 1)) == 0;
	}

	template<std::unsigned_integral T>
	constexpr auto is_multiple_of(T size, T alignment) noexcept -> bool
	{
		return (size % alignment) == 0;
	}

	template<std::unsigned_integral T>
	constexpr auto round_up_size_to_alignment(T size, T alignment) noexcept -> T
	{
		assert_true_debug(is_power_of_two(alignment), "Alignment isn't power of 2!");
		if(is_multiple_of(size, alignment))
			return size;

		return (size < alignment ? alignment : (size / alignment) * alignment + alignment);
	}

	template<std::unsigned_integral T>
	struct block
	{
		T size;
		T offset;

		constexpr block(T _size = {}, T _offset = {}) noexcept : size(_size), offset(_offset)
		{}

		constexpr block(const block &) noexcept = default;
		constexpr block(block &&) noexcept = default;
		constexpr auto operator=(const block &) noexcept -> block & = default;
		constexpr auto operator=(block &&) noexcept -> block & = default;
	};
}
