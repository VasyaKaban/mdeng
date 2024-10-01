#include "frame.h"
#include "stacktrace_init.h"
#include <DbgHelp.h>
#include <libloaderapi.h>

namespace hrs
{
    namespace winapi
    {
        static const detail::stacktrace_init& get_init_stacktrace_var() noexcept
        {
            static detail::stacktrace_init init_stacktrace_var;
            return init_stacktrace_var;
        }

        frame::frame(native_symbol_t _symbol) noexcept
            : symbol(_symbol)
        {}

        std::string frame::function_name(bool demangle) const
        {
            if(!symbol)
                return "";

            return get_symbol_info(true, demangle);
        }

        std::string frame::object_path() const
        {
            if(!symbol)
                return "";

            return get_symbol_info(false, true);
        }

        frame::native_symbol_t frame::native_handle() noexcept
        {
            return symbol;
        }

        const frame::native_symbol_t frame::native_handle() const noexcept
        {
            return symbol;
        }

        std::string frame::get_symbol_info(bool get_only_symbol_name, bool demangle) const
        {
            const auto& stacktrace_init_var = get_init_stacktrace_var();

            if(!stacktrace_init_var.is_created())
                return "";

            DWORD64 addr = reinterpret_cast<DWORD64>(symbol);
            DWORD64 disp = 0;
            auto res = SymFromAddr(stacktrace_init_var.get_process_handle(),
                                   addr,
                                   &disp,
                                   stacktrace_init_var.get_symbol_info());
            if(res == FALSE)
                return "";

            if(get_only_symbol_name)
                return std::string(stacktrace_init_var.get_symbol_info()->Name,
                                   stacktrace_init_var.get_symbol_info()->NameLen);
            else
            {
                HMODULE module_addr;
                auto get_module_res = GetModuleHandleExA(
                    GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
                    reinterpret_cast<LPCSTR>(stacktrace_init_var.get_symbol_info()->ModBase),
                    &module_addr);
                if(!get_module_res)
                    return "";

                std::string module_name(MAX_PATH, '\0');
                auto count =
                    GetModuleFileNameA(module_addr, module_name.data(), module_name.size());
                module_name.resize(count);
                return module_name;
            }
        }
    };
};
