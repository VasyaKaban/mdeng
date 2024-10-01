#pragma once

#include <type_traits>

namespace hrs
{
    template<typename>
    struct member_class_type
    {};

    template<typename C, typename T>
    struct member_class_type<T C::*>
    {
        using field_type = T;
        using class_type = C;
    };

    template<typename T>
    using member_class_type_class_t = member_class_type<T>::class_type;

    template<typename T>
    using member_class_type_field_t = member_class_type<T>::field_type;

    template<typename>
    struct is_class_member : std::false_type
    {};

    template<typename C, typename T>
    struct is_class_member<T C::*> : std::true_type
    {};

    template<typename T>
    constexpr inline bool is_class_member_v = is_class_member<T>::value;
}
