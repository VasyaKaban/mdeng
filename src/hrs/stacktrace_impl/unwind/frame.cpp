#include "frame.h"
#include "../../demangle.hpp"
#include <dlfcn.h>

namespace hrs
{
    namespace unwind
    {
        frame::frame(native_symbol_t _symbol) noexcept
            : symbol(_symbol)
        {}

        std::string frame::function_name(bool demangle) const
        {
            if(!symbol)
                return "";

            Dl_info info;
            int res = dladdr(symbol, &info);
            if(res == 0)
                return "";

            if(!info.dli_sname)
                return "";

            if(!demangle)
                return info.dli_sname;

            return hrs::demangle(info.dli_sname);
        }

        std::string frame::object_path() const
        {
            if(!symbol)
                return "";

            Dl_info info;
            int res = dladdr(symbol, &info);
            if(res == 0)
                return "";

            if(!info.dli_fname)
                return "";

            return info.dli_fname;
        }

        frame::native_symbol_t frame::native_handle() noexcept
        {
            return symbol;
        }

        const frame::native_symbol_t frame::native_handle() const noexcept
        {
            return symbol;
        }
    };
};
