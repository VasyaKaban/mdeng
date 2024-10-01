#pragma once

#define NOMINMAX
#include <Windows.h>
#include <DbgHelp.h>

#include "../../non_creatable.hpp"

namespace hrs
{
    namespace winapi
    {
        namespace detail
        {
            class stacktrace_init
                : hrs::non_copyable,
                  hrs::non_movable
            {
            public:
                stacktrace_init();
                ~stacktrace_init();

                HANDLE get_process_handle() const noexcept;
                PSYMBOL_INFO get_symbol_info() const noexcept;

                bool is_created() const noexcept;

            private:
                HANDLE process;
                PSYMBOL_INFO symbol_info; 
            };
        };
    };
};