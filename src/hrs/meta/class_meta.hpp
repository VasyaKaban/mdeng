#pragma once

#include "../member_class.hpp"
#include "../static_string/static_string.hpp"
#include "../variadic.hpp"
#include "meta_attributes.hpp"
#include "split_namespace.hpp"
#include <tuple>

namespace hrs
{
    namespace detail
    {
        template<typename C,
                 hrs::static_string _name,
                 hrs::non_type_instantiation<meta_attributes> _attributes>
        struct class_meta_base : _attributes
        {
            constexpr static auto name = detail::retrieve_namespace_and_name<_name>().second;
            constexpr static auto namespaces = detail::retrieve_namespace_and_name<_name>().first;
            using parents = hrs::variadic<>;
            using refl_class = C;
            using member_fields = hrs::variadic<>;
            using static_fields = hrs::variadic<>;
            using using_fields = hrs::variadic<>;
        };
    };

    template<auto _ptr, hrs::static_string _name, auto... _attrs>
    struct member_field : public meta_attributes<_attrs...>
    {
        using type = member_class_type<decltype(_ptr)>::field_type;
        constexpr static auto name = _name;
        constexpr static auto ptr = _ptr;
        using class_type = typename member_class_type<decltype(_ptr)>::class_type;

        template<typename C>
        requires(std::is_class_v<class_type> &&
                 std::is_base_of_v<class_type, std::remove_cvref_t<C>>) ||
                (std::is_union_v<class_type>)
        constexpr static auto& get(C&& obj) noexcept
        {
            return std::forward<C>(obj).*ptr;
        }
    };

    template<auto _ptr, hrs::static_string _name, typename C, auto... _attrs>
    struct static_field : public meta_attributes<_attrs...>
    {
        using type = std::remove_pointer_t<decltype(_ptr)>;
        constexpr static auto name = _name;
        constexpr static auto ptr = _ptr;
        using class_type = C;

        constexpr static auto& get() noexcept
        {
            return *ptr;
        }
    };

    template<typename T, hrs::static_string _name, typename C, auto... _attrs>
    struct using_field : public meta_attributes<_attrs...>
    {
        using type = T;
        constexpr static auto name = _name;
        using class_type = C;
    };

    template<typename C, typename... Classes>
    consteval bool is_derived_from(hrs::variadic<Classes...>) noexcept
    {
        return (std::is_base_of_v<Classes, C> && ...);
    }

    template<typename C>
    struct class_meta : detail::class_meta_base<C, "", meta_attributes<>>
    {};
};
