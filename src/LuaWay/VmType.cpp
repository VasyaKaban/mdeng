#include "VmType.h"

#include "hrs/debug.hpp"

namespace LuaWay
{
    VmType GetType(lua_State* state, int index) noexcept
    {
        hrs::assert_true_debug(state != nullptr, "Lua state isn't created yet!");

        VmType vm_type = static_cast<VmType>(lua_type(state, index));
        if(vm_type == VmType::Function)
            if(lua_iscfunction(state, index))
                vm_type = VmType::CFunction;

        return vm_type;
    }
};
