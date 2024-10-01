#pragma once

#include <concepts>
#include <utility>

namespace hrs
{
    template<std::invocable F>
    void on_thread_exit(F&& _func) noexcept(
        std::is_nothrow_constructible_v<std::remove_reference_t<F>, F>)
    {
        thread_local struct on_thread_exit_storage
        {
            using func_t = std::remove_reference_t<F>;
            func_t func;

            on_thread_exit_storage(F _func) noexcept(std::is_nothrow_constructible_v<func_t, F>)
                : func(_func)
            {}

            ~on_thread_exit_storage()
            {
                func();
            }
        } on_thread_exit_var(std::forward<F>(_func));
    };
};
