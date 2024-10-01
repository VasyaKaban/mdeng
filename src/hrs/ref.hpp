#pragma once

#include "instantiation.hpp"
#include "non_creatable.hpp"

namespace hrs
{
    template<typename R>
    requires std::is_reference_v<R>
    class ref : public hrs::non_copyable, public hrs::non_movable
    {
    public:
        constexpr ref(R _ref_value) noexcept
            : ref_value(std::forward<R>(_ref_value))
        {}

        constexpr ref(const ref& _r) noexcept
            : ref_value(std::forward<R>(_r.ref_value))
        {}

        ~ref() = default;

        constexpr R operator*() const noexcept
        {
            if constexpr(std::is_lvalue_reference_v<R>)
                return ref_value;
            else
                return std::move(ref_value);
        }
    private:
        R ref_value;
    };

    template<typename R>
    ref(R&&) -> ref<
        std::conditional_t<std::is_lvalue_reference_v<R>, R, std::add_rvalue_reference_t<R>>>;

    template<typename T>
    constexpr decltype(auto) deref_or_get(T&& value) noexcept
    {
        if constexpr(hrs::type_instantiation<T, hrs::ref>)
            return *std::forward<T>(value);
        else
            return std::forward<T>(value);
    }
};
