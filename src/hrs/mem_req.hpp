#pragma once

#include <concepts>
#include <algorithm>
#include "block.hpp"

namespace hrs
{
	template<std::unsigned_integral T>
	struct mem_req
	{
		T size;
		T alignment;

		constexpr mem_req(T _size = {}, T _alignment = {}) noexcept
			: size(_size), alignment(_alignment) {}

		constexpr mem_req(const mem_req &) = default;
		constexpr mem_req & operator=(const mem_req &) = default;
		constexpr bool operator==(const mem_req &) const noexcept = default;

		constexpr bool is_size_multiple_of_alignment() const noexcept
		{
			return hrs::is_multiple_of(size, alignment);
		}

		constexpr bool is_alignment_power_of_two() const noexcept
		{
			return hrs::is_power_of_two(alignment);
		}
	};

	template<typename T, typename ...Args>
	constexpr mem_req<std::size_t> union_size_alignment() noexcept
	{
		mem_req<std::size_t> req;
		req.size = std::max(sizeof(T), sizeof(Args)...);
		req.alignment = std::max(alignof(T), alignof(Args)...);

		req.size = hrs::round_up_size_to_alignment(req.size, req.alignment);
		return req;
	}
};
