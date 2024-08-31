#pragma once

#include "VMBase.h"

namespace LuaWay
{
	class VMView : public VMBase
	{
	public:
		VMView();
		VMView(lua_State *_state) noexcept;
		VMView(const VMBase &vm) noexcept;
		~VMView() = default;
		VMView(const VMView &) = default;
		VMView(VMView &&vm) noexcept;
		VMView & operator=(const VMView &) = default;
		VMView & operator=(VMView &&vm) noexcept;

		bool IsMainThread() const noexcept;
	};
};

