#pragma once

#include "meta/enum_meta.hpp"
#include <bit>
#include <concepts>
#include <string_view>
#include <type_traits>
#include <typeinfo>

namespace hrs
{
    using error_enum_value = unsigned long long int;

    namespace detail
    {
        template<typename E>
        requires std::is_enum_v<E>
        constexpr error_enum_value enum_to_value(E e) noexcept
        {
            using enum_cast_t = std::conditional_t<std::is_signed_v<std::underlying_type_t<E>>,
                                                   signed long long int,
                                                   unsigned long long int>;

            return std::bit_cast<error_enum_value>(static_cast<enum_cast_t>(e));
        }

        template<typename E>
        requires std::is_enum_v<E>
        constexpr E value_to_enum(error_enum_value value) noexcept
        {
            using enum_cast_t = std::conditional_t<std::is_signed_v<std::underlying_type_t<E>>,
                                                   signed long long int,
                                                   unsigned long long int>;

            return static_cast<E>(std::bit_cast<enum_cast_t>(value));
        }
    };

    class error
    {
    public:
        constexpr error() noexcept
            : value(0),
              id(nullptr)
        {}

        ~error() = default;

        template<typename E>
        requires std::is_enum_v<E>
        constexpr error(E e) noexcept
            : value(detail::enum_to_value(e)),
              id(&typeid(E))
        {}

        constexpr error(const error&) = default;
        constexpr error& operator=(const error&) = default;

        constexpr explicit operator bool() const noexcept
        {
            return id != nullptr;
        }

        constexpr bool is_empty() const noexcept
        {
            return id == nullptr;
        }

        constexpr void clear() noexcept
        {
            id = nullptr;
        }

        template<typename E>
        requires std::is_enum_v<E>
        constexpr bool holds() const noexcept
        {
            return *id == typeid(E);
        }

        template<typename E>
        requires std::is_enum_v<E>
        constexpr E revive() const noexcept
        {
            return (holds<E>() ? detail::value_to_enum<E>(value) : E{});
        }

        template<typename E>
        requires std::is_enum_v<E>
        constexpr std::string_view get_name() const noexcept
        {
            if(*id == typeid(E))
                return enum_meta<E>::get_name(detail::value_to_enum<E>(value));

            return "";
        }

        constexpr error_enum_value get_raw_value() const noexcept
        {
            return value;
        }

        template<typename E>
        requires std::is_enum_v<E>
        constexpr bool operator==(E e) const noexcept
        {
            if(holds<E>())
                return detail::value_to_enum<E>(value) == e;

            return false;
        }

        constexpr bool operator==(const error&) const = default;
    private:
        template<typename E, typename... Enums, typename F>
        constexpr bool visit_one(F&& f) const noexcept
        {
            if(*id == typeid(E))
            {
                if constexpr(std::invocable<F, E>)
                    std::forward<F>(f)(detail::value_to_enum<E>(value));

                return true;
            }

            if constexpr(sizeof...(Enums) != 0)
                return visit_one<Enums...>(std::forward<F>(f));
            else
                return false;
        }
    public:
        template<typename... Enums, typename F>
        requires(std::is_enum_v<Enums> && ...) &&
                ((std::invocable<F, Enums> && ...) || std::invocable<F, error_enum_value> ||
                 std::invocable<F>)
        constexpr void visit(F&& f) const noexcept
        {
            //if id == nullptr -> F::operator(void) <-> finally()
            //if id != Enums -> F::operator(enum_value) <-> catch(...)
            //if id == Enums[id] -> F::operator(value.get<Enums>) <-> catch(Enums)
            if(id == nullptr)
            {
                if constexpr(std::invocable<F>)
                    std::forward<F>(f)();
            }
            else if constexpr(sizeof...(Enums) == 0)
            {
                if constexpr(std::invocable<F, error_enum_value>)
                    std::forward<F>(f)(value);
            }
            else
            {
                bool visit_res = visit_one<Enums...>(std::forward<F>(f));
                if constexpr(std::invocable<F, error_enum_value>)
                {
                    if(!visit_res)
                        std::forward<F>(f)(value);
                }
            }
        }
    private:
        error_enum_value value;
        const std::type_info* id;
    };
};
