#pragma once

#include "Stack.h"
#include "hrs/instantiation.hpp"
#include "hrs/ref.hpp"

#if LUA_VERSION_NUM < 502
#    define LUA_RIDX_MAINTHREAD 1
#endif

namespace LuaWay
{
    namespace detail
    {
        template<bool is_ref, typename T>
        struct ref_select;

        template<typename T>
        struct ref_select<false, T>
        {
            using type = T;
        };

        template<typename T>
        struct ref_select<true, T>
        {
            using type = hrs::ref<T>;
        };

        template<typename T>
        using ref_select_t = ref_select<std::is_reference_v<T>, T>::type;

        template<Retrievable T>
        struct retrieve_value_type_for_non_ref_wrapper
        {
            using plain_t = decltype(Stack<T>::Retrieve(static_cast<lua_State*>(nullptr), -1));
            using out_t = ref_select_t<plain_t>;
        };

        template<Retrievable T>
        auto retrieve_value_for_non_ref_wrapper(lua_State* state,
                                                int index) noexcept(NoexceptRetrievable<T>)
        {
            using out_t = retrieve_value_type_for_non_ref_wrapper<T>::out_t;

            if constexpr(hrs::type_instantiation<out_t, hrs::ref>)
                return hrs::ref{Stack<T>::Retrieve(state, index)};
            else
                return Stack<T>::Retrieve(state, index);
        }

        template<Retrievable T>
        decltype(auto)
        retrieve_value_for_non_ref_wrapper_and_deref(lua_State* state,
                                                     int index) noexcept(NoexceptRetrievable<T>)
        {
            return hrs::deref_or_get(retrieve_value_for_non_ref_wrapper<T>(state, index));
        }
    };
};
