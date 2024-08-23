#include "VMView.h"

namespace LuaWay
{
	VMView::VMView()
		: VMBase(nullptr) {}

	VMView::VMView(lua_State *_state) noexcept
		: VMBase(_state) {}

	VMView::VMView(const VMBase &vm) noexcept
		: VMBase(vm.GetState()) {}

	VMView::VMView(VMView &&vm) noexcept
		: VMBase(std::exchange(vm.state, nullptr)) {}

	VMView & VMView::operator=(VMView &&vm) noexcept
	{
		state = std::exchange(vm.state, nullptr);
		return *this;
	}
};
