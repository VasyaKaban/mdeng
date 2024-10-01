#include "demangle.h"
#include <cxxabi.h>

namespace hrs
{
    namespace ia64_abi
    {
        std::string demangle(const char* mangled_name)
        {
            int demangle_status;
            char* name =
                __cxxabiv1::__cxa_demangle(mangled_name, nullptr, nullptr, &demangle_status);

            if(!(demangle_status == 0 || demangle_status == -2))
                return "";

            std::string demangled_name;
            try
            {
                if(demangle_status == 0)
                {
                    demangled_name = name;
                    free(name);
                }
                else
                    demangled_name = mangled_name;
            }
            catch(...)
            {
                if(demangle_status == 0)
                    free(name);

                throw;
            }

            return demangled_name;
        }
    };
};
