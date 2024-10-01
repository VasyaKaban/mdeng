#include "VMView.h"

namespace LuaWay
{
    VMView::VMView()
        : VMBase(nullptr)
    {}

    VMView::VMView(lua_State* _state) noexcept
        : VMBase(_state)
    {}

    VMView::VMView(const VMBase& vm) noexcept
        : VMBase(vm.GetState())
    {}

    VMView::VMView(VMView&& vm) noexcept
        : VMBase(std::exchange(vm.state, nullptr))
    {}

    VMView& VMView::operator=(VMView&& vm) noexcept
    {
        state = std::exchange(vm.state, nullptr);
        return *this;
    }

    bool VMView::IsMainThread() const noexcept
    {
        if(!state)
            return false;

        int res = lua_pushthread(state);
        lua_pop(state, 1);
        return res == 1;
    }
};
