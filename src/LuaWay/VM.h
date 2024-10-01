#pragma once

#include "VMBase.h"
#include "hrs/non_creatable.hpp"

namespace LuaWay
{
    class VM : public hrs::non_copyable, public VMBase
    {
    private:
        VM(lua_State* _state) noexcept;
    public:
        VM();
        ~VM();
        VM(VM&& vm) noexcept;
        VM& operator=(VM&& vm) noexcept;

        static std::optional<VM> Open(bool open_std_libs, int stack_size = LUA_MINSTACK) noexcept;

        void Close() noexcept;
    };
};
