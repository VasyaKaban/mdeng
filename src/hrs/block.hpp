/**
 * @file
 *
 * Represents the block class
 */

#pragma once

#include <concepts>

namespace hrs
{
    /**
	 * @brief is_power_of_two
	 * @tparam T must satisfy the unsigned_integral concept
	 * @param alignment
	 * @return true if alignment is power of two, otherwise false
	 */
    template<std::unsigned_integral T>
    constexpr bool is_power_of_two(T alignment) noexcept
    {
        //assert_true_debug(alignment != 0, "Alignment must be > 0!");
        return (alignment && ((alignment & (alignment - 1)) == 0));
    }

    /**
	 * @brief is_multiple_of
	 * @tparam T must satisfy the unsigned_integral concept
	 * @param size
	 * @param alignment
	 * @return true if size is multiple of alignment, otherwise false
	 */
    template<std::unsigned_integral T>
    constexpr bool is_multiple_of(T size, std::type_identity_t<T> alignment) noexcept
    {
        return (size % alignment) == 0;
    }

    /**
	 * @brief round_up_size_to_alignment
	 * @tparam T must satisfy the unsigned_integral concept
	 * @param size
	 * @param alignment
	 * @return size rounded up to alignment
	 */
    template<std::unsigned_integral T>
    constexpr T round_up_size_to_alignment(T size, std::type_identity_t<T> alignment) noexcept
    {
        //assert_true_debug(is_power_of_two(alignment), "Alignment isn't a power of 2!");
        //if(is_multiple_of(size, alignment))
        //	return size;

        return (size <= alignment ? alignment : (size / alignment) * alignment + alignment);
    }

    /**
	 * @brief The block class
	 * @tparam T must satisfy the unsigned_integral concept
	 *
	 */
    template<std::unsigned_integral T>
    struct block
    {
        T size; ///<block size
        T offset; ///<block offset

        constexpr block(T _size = {}, T _offset = {}) noexcept
            : size(_size),
              offset(_offset)
        {}

        constexpr block(const block&) noexcept = default;
        constexpr block& operator=(const block&) noexcept = default;
        constexpr bool operator==(const block&) const noexcept = default;
    };
}
