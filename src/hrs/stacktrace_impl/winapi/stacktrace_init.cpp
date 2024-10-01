#include "stacktrace_init.h"
#include "../../scoped_call.hpp"
#include <DbgHelp.h>

namespace hrs
{
    namespace winapi
    {
        namespace detail
        {
            stacktrace_init::stacktrace_init()
                : symbol_info(nullptr)
            {
                auto target_process = GetCurrentProcess();
                HANDLE dup_process;
                BOOL sym_inited = FALSE;
                PSYMBOL_INFO _symbol_info = nullptr;
                if(DuplicateHandle(target_process,
                                   target_process,
                                   target_process,
                                   &dup_process,
                                   0,
                                   FALSE,
                                   DUPLICATE_SAME_ACCESS) == FALSE)
                    return;

                auto cleanup = hrs::scoped_call(
                    [dup_process, sym_inited, _symbol_info]()
                    {
                        if(_symbol_info)
                            std::free(_symbol_info);

                        if(sym_inited)
                            SymCleanup(dup_process);

                        CloseHandle(dup_process);
                    });

                sym_inited = SymInitialize(dup_process, nullptr, TRUE);
                if(sym_inited == FALSE)
                    return;

                auto options = SymGetOptions();
                SymSetOptions(options | SYMOPT_UNDNAME);

                _symbol_info = static_cast<PSYMBOL_INFO>(
                    std::calloc(1, sizeof(SYMBOL_INFO) + sizeof(char) * MAX_SYM_NAME + 1));

                if(!_symbol_info)
                    return;

                _symbol_info->SizeOfStruct = sizeof(SYMBOL_INFO);
                _symbol_info->MaxNameLen = MAX_SYM_NAME;

                cleanup.drop();

                process = dup_process;
                symbol_info = _symbol_info;
            }

            stacktrace_init::~stacktrace_init()
            {
                if(!is_created())
                    return;

                std::free(symbol_info);
                SymCleanup(process);
                CloseHandle(process);
            }

            HANDLE stacktrace_init::get_process_handle() const noexcept
            {
                return process;
            }

            PSYMBOL_INFO stacktrace_init::get_symbol_info() const noexcept
            {
                return symbol_info;
            }

            bool stacktrace_init::is_created() const noexcept
            {
                return symbol_info != nullptr;
            }
        };
    };
};