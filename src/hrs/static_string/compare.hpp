#pragma once

#include "static_string.hpp"
#include <cstring>
#include <string>
#include <string_view>

namespace hrs
{
    template<char_type C, std::size_t N>
    std::strong_ordering operator<=>(const static_string<C, N>& sstr, const C* cstr) noexcept
    {
        std::size_t len = std::strlen(cstr);
        if(N > len)
            return std::strong_ordering::greater;
        else if(N < len)
            return std::strong_ordering::less;

        int res = std::memcmp(sstr.data, cstr, N * sizeof(C));
        if(res == 0)
            return std::strong_ordering::equal;
        else if(res == -1)
            return std::strong_ordering::less;
        else
            return std::strong_ordering::greater;
    }

    template<char_type C, std::size_t N>
    bool operator==(const static_string<C, N>& sstr, const C* cstr) noexcept
    {
        return (sstr <=> cstr) == std::strong_ordering::equal;
    }

    template<char_type C, std::size_t N>
    std::strong_ordering operator<=>(const static_string<C, N>& sstr,
                                     std::basic_string_view<C> svstr) noexcept
    {
        std::size_t len = svstr.size();
        if(N > len)
            return std::strong_ordering::greater;
        else if(N < len)
            return std::strong_ordering::less;

        int res = std::memcmp(sstr.data, svstr.data(), N * sizeof(C));
        if(res == 0)
            return std::strong_ordering::equal;
        else if(res == -1)
            return std::strong_ordering::less;
        else
            return std::strong_ordering::greater;
    }

    template<char_type C, std::size_t N>
    bool operator==(const static_string<C, N>& sstr, std::basic_string_view<C> svstr) noexcept
    {
        return (sstr <=> svstr) == std::strong_ordering::equal;
    }

    template<char_type C, std::size_t N>
    std::strong_ordering operator<=>(const static_string<C, N>& sstr,
                                     const std::basic_string<C>& str) noexcept
    {
        std::size_t len = str.size();
        if(N > len)
            return std::strong_ordering::greater;
        else if(N < len)
            return std::strong_ordering::less;

        int res = std::memcmp(sstr.data, str.data(), N * sizeof(C));
        if(res == 0)
            return std::strong_ordering::equal;
        else if(res == -1)
            return std::strong_ordering::less;
        else
            return std::strong_ordering::greater;
    }

    template<char_type C, std::size_t N>
    bool operator==(const static_string<C, N>& sstr, const std::basic_string<C>& str) noexcept
    {
        return (sstr <=> str) == std::strong_ordering::equal;
    }
};
