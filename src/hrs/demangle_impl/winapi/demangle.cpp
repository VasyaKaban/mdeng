#include "demangle.h"
#include <DbgHelp.h>
#include <Windows.h>
#include <string>

namespace hrs
{
    namespace winapi
    {
        std::string demangle(const char* mangled_name)
        {
            std::string demangled_name(MAX_SYM_NAME, '\0');
            auto count = UnDecorateSymbolName(mangled_name,
                                              demangled_name.data(),
                                              MAX_SYM_NAME,
                                              UNDNAME_COMPLETE);
            demangled_name.resize(count);

            return demangled_name;
        }
    };
};
