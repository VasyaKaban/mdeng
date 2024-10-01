#pragma once

#include <type_traits>

namespace hrs
{
    template<typename C, typename O>
    struct copy_const
    {
        using type = std::conditional_t<std::is_const_v<C>, std::add_const_t<O>, O>;
    };

    template<typename C, typename O>
    using copy_const_t = copy_const<C, O>::type;

    template<typename C, typename O>
    struct copy_volatile
    {
        using type = std::conditional_t<std::is_volatile_v<C>, std::add_const_t<O>, O>;
    };

    template<typename C, typename O>
    using copy_volatile_t = copy_volatile<C, O>::type;

    template<typename C, typename O>
    struct copy_cv
    {
        using type = copy_const_t<C, copy_volatile_t<C, O>>;
    };

    template<typename C, typename O>
    using copy_cv_t = copy_cv<C, O>::type;

    template<typename C, typename O>
    struct copy_lvalue_reference
    {
        using type =
            std::conditional_t<std::is_lvalue_reference_v<C>, std::add_lvalue_reference_t<O>, O>;
    };

    template<typename C, typename O>
    using copy_lvalue_reference_t = copy_lvalue_reference<C, O>::type;

    template<typename C, typename O>
    struct copy_rvalue_reference
    {
        using type =
            std::conditional_t<std::is_rvalue_reference_v<C>, std::add_rvalue_reference_t<O>, O>;
    };

    template<typename C, typename O>
    using copy_rvalue_reference_t = copy_rvalue_reference<C, O>::type;

    template<typename C, typename O>
    struct copy_reference
    {
        using type = std::conditional_t<std::is_lvalue_reference_v<C>,
                                        copy_lvalue_reference_t<C, O>,
                                        copy_rvalue_reference_t<C, O>>;
    };

    template<typename C, typename O>
    using copy_reference_t = copy_reference<C, O>::type;

    template<typename C, typename O>
    struct copy_cvref
    {
        using type = copy_reference_t<C, copy_cv_t<C, O>>;
    };

    template<typename C, typename O>
    using copy_cvref_t = copy_cvref<C, O>::type;
};
