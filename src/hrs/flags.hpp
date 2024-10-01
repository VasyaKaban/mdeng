#pragma once

#include "debug.hpp"
#include <limits>
#include <type_traits>

namespace hrs
{
    template<typename T>
    requires std::is_enum_v<T>
    struct flags
    {
        using underlying_mask_type = std::underlying_type_t<T>;
        constexpr static underlying_mask_type FULL_MASK =
            std::numeric_limits<underlying_mask_type>::max();

        underlying_mask_type mask;

        constexpr flags(T _enum_value) noexcept
            : mask(static_cast<underlying_mask_type>(_enum_value))
        {}

        constexpr flags(underlying_mask_type _mask = {}) noexcept
            : mask(_mask)
        {}

        constexpr flags(const flags&) = default;
        constexpr flags(flags&&) = default;
        constexpr flags& operator=(const flags&) = default;
        constexpr flags& operator=(flags&&) = default;

        constexpr explicit operator underlying_mask_type() const noexcept
        {
            return mask;
        }

        constexpr flags& operator=(underlying_mask_type _mask) noexcept
        {
            mask = _mask;
            return *this;
        }

        constexpr flags& operator=(T _enum_value) noexcept
        {
            mask = _enum_value;
            return *this;
        }

        constexpr flags operator|(flags fl) const noexcept
        {
            return mask | fl.mask;
        }

        constexpr flags operator&(flags fl) const noexcept
        {
            return mask & fl.mask;
        }

        constexpr flags operator^(flags fl) const noexcept
        {
            return mask ^ fl.mask;
        }

        constexpr flags operator~() const noexcept
        {
            return ~mask;
        }

        constexpr flags& operator|=(flags fl) noexcept
        {
            mask |= fl.mask;
            return *this;
        }

        constexpr flags& operator&=(flags fl) noexcept
        {
            mask &= fl.mask;
            return *this;
        }

        constexpr flags& operator^=(flags fl) noexcept
        {
            mask ^= fl.mask;
            return *this;
        }

        constexpr flags operator|(underlying_mask_type umask) const noexcept
        {
            return mask | umask;
        }

        constexpr flags operator&(underlying_mask_type umask) const noexcept
        {
            return mask & umask;
        }

        constexpr flags operator^(underlying_mask_type umask) const noexcept
        {
            return mask ^ umask;
        }

        constexpr flags& operator|=(underlying_mask_type umask) noexcept
        {
            mask |= umask;
            return *this;
        }

        constexpr flags& operator&=(underlying_mask_type umask) noexcept
        {
            mask &= umask;
            return *this;
        }

        constexpr flags& operator^=(underlying_mask_type umask) noexcept
        {
            mask ^= umask;
            return *this;
        }

        constexpr flags operator|(T enum_value) const noexcept
        {
            return mask | static_cast<underlying_mask_type>(enum_value);
        }

        constexpr flags operator&(T enum_value) const noexcept
        {
            return mask & static_cast<underlying_mask_type>(enum_value);
        }

        constexpr flags operator^(T enum_value) const noexcept
        {
            return mask ^ static_cast<underlying_mask_type>(enum_value);
        }

        constexpr flags& operator|=(T enum_value) noexcept
        {
            mask |= static_cast<underlying_mask_type>(enum_value);
            return *this;
        }

        constexpr flags& operator&=(T enum_value) noexcept
        {
            mask &= static_cast<underlying_mask_type>(enum_value);
            return *this;
        }

        constexpr flags& operator^=(T enum_value) noexcept
        {
            mask ^= static_cast<underlying_mask_type>(enum_value);
            return *this;
        }

        constexpr flags operator<<(std::size_t bits) const noexcept
        {
            return (mask << bits);
        }

        constexpr flags operator>>(std::size_t bits) const noexcept
        {
            return (mask >> bits);
        }

        constexpr flags& operator<<=(std::size_t bits) noexcept
        {
            mask <<= bits;
            return *this;
        }

        constexpr flags& operator>>=(std::size_t bits) noexcept
        {
            mask >>= bits;
            return *this;
        }

        constexpr explicit operator bool() const noexcept
        {
            return mask != 0;
        }

        constexpr flags fill_first(std::size_t count) const noexcept
        {
            hrs::assert_true_debug(std::numeric_limits<underlying_mask_type>::digits >= count,
                                   "Number of bits={} must be bigger than number of shifts={}!",
                                   std::numeric_limits<underlying_mask_type>::digits,
                                   count);

            return (count == 0 ? 1 : (1 << count) - 1);
        }

        constexpr flags fill_last(std::size_t count) const noexcept
        {
            hrs::assert_true_debug(std::numeric_limits<underlying_mask_type>::digits >= count,
                                   "Number of bits={} must be bigger than number of shifts={}!",
                                   std::numeric_limits<underlying_mask_type>::digits,
                                   count);

            return ~fill_first(std::numeric_limits<underlying_mask_type>::digits - count);
        }

        template<std::integral I>
        constexpr auto operator+(I value) const noexcept
        {
            using out_type = std::common_type_t<underlying_mask_type, I>;
            return static_cast<out_type>(mask) + value;
        }

        template<std::integral I>
        constexpr auto operator-(I value) const noexcept
        {
            using out_type = std::common_type_t<underlying_mask_type, I>;
            return static_cast<out_type>(mask) - value;
        }

        template<std::integral I>
        constexpr auto operator*(I value) const noexcept
        {
            using out_type = std::common_type_t<underlying_mask_type, I>;
            return static_cast<out_type>(mask) * value;
        }

        template<std::integral I>
        constexpr auto operator/(I value) const noexcept
        {
            using out_type = std::common_type_t<underlying_mask_type, I>;
            return static_cast<out_type>(mask) / value;
        }

        template<std::integral I>
        constexpr flags& operator+=(I value) noexcept
        {
            mask += value;
            return *this;
        }

        template<std::integral I>
        constexpr flags& operator-=(I value) noexcept
        {
            mask -= value;
            return *this;
        }

        template<std::integral I>
        constexpr flags& operator*=(I value) noexcept
        {
            mask *= value;
            return *this;
        }

        template<std::integral I>
        constexpr flags& operator/=(I value) noexcept
        {
            mask /= value;
            return *this;
        }
    };

    template<typename T>
    requires std::is_enum_v<T>
    constexpr flags<T> operator|(T enum_value0, T enum_value1) noexcept
    {
        return flags<T>(enum_value0) | enum_value1;
    }

    template<typename T>
    requires std::is_enum_v<T>
    constexpr flags<T> operator&(T enum_value0, T enum_value1) noexcept
    {
        return flags<T>(enum_value0) & enum_value1;
    }

    template<typename T>
    requires std::is_enum_v<T>
    constexpr flags<T> operator^(T enum_value0, T enum_value1) noexcept
    {
        return flags<T>(enum_value0) ^ enum_value1;
    }
};
