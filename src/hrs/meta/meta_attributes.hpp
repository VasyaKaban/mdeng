#pragma once

#include "../one_of.hpp"
#include <optional>
#include <tuple>

namespace hrs
{
    template<auto... _attrs>
    struct meta_attributes
    {
        constexpr static auto attributes = std::tuple{_attrs...};
    private:
        template<std::size_t Index, typename T>
        constexpr static bool have(T&& value) noexcept
        {
            if constexpr(Index == sizeof...(_attrs))
                return false;
            else if constexpr(requires { std::get<Index>(attributes) == std::forward<T>(value); })
                if(std::forward<T>(value) == std::get<Index>(attributes))
                    return true;
                else
                    return have<Index + 1>(std::forward<T>(value));
            else
                return have<Index + 1>(std::forward<T>(value));
        }

        template<std::size_t Index, typename T>
        constexpr static std::optional<std::reference_wrapper<const T>> get(T&& value) noexcept
        {
            if constexpr(Index == sizeof...(_attrs))
                return {};
            else if constexpr(requires { std::get<Index>(attributes) == std::forward<T>(value); })
                if(std::forward<T>(value) == std::get<Index>(attributes))
                    return std::get<Index>(attributes);
                else
                    return get<Index + 1>(std::forward<T>(value));
            else
                return get<Index + 1>(std::forward<T>(value));
        }
    public:
        template<typename T>
        constexpr static bool have(T&& value) noexcept
        {
            return have<0>(std::forward<T>(value));
        }

        template<typename T>
        constexpr static std::optional<std::reference_wrapper<const T>> get(T&& value) noexcept
        {
            return get<0>(std::forward<T>(value));
        }

        template<auto value>
        constexpr static bool have() noexcept
        {
            return hrs::one_of_non_type<value, _attrs...>;
        }

        template<auto value>
        constexpr static const auto& get() noexcept
        {
            constexpr auto opt = get(value);
            static_assert(opt, "No desired attribute!");
            return opt->get();
        }

        template<typename T>
        constexpr static bool have() noexcept
        {
            return hrs::one_of_type<T, decltype(_attrs)...>;
        }
    };
};
