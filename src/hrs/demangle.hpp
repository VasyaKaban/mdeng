#pragma once

#if defined(unix) || defined(__unix) || defined(__unix__)
#    include "demangle_impl/ia64_abi/demangle.h"
#elif defined(_WIN32) || defined(_WIN64)
#    include "demangle_impl/winapi/demangle.h"
#else
#    error Unsupported OS!
#endif
namespace hrs
{
    std::string demangle(const char* mangled_name)
    {
#if defined(unix) || defined(__unix) || defined(__unix__)
        return ia64_abi::demangle(mangled_name);
#elif defined(_WIN32) || defined(_WIN64)
        return winapi::demangle(mangled_name);
#endif
    }
};
