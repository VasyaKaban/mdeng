#include "VM.h"
#include "Common.h"

namespace LuaWay
{
	VM::VM(lua_State *_state) noexcept
		: VMBase(_state) {}

	VM::VM()
		: VMBase(nullptr) {}

	VM::~VM()
	{
		Close();
	}

	VM::VM(VM &&vm) noexcept
		: VMBase(std::exchange(vm.state, nullptr)) {}

	VM & VM::operator=(VM &&vm) noexcept
	{
		Close();

		state = std::exchange(vm.state, nullptr);

		return *this;
	}

	std::optional<VM> VM::Open(bool open_std_libs, int stack_size) noexcept
	{
		lua_State *_state = luaL_newstate();
		if(!_state)
			return {};

		if(open_std_libs)
			luaL_openlibs(_state);

		lua_checkstack(_state, stack_size);

		lua_pushthread(_state);
		int _ref = luaL_ref(_state, LUA_REGISTRYINDEX);
		hrs::assert_true_debug(_ref == LUA_RIDX_MAINTHREAD,
							   "LUA_RIDX_MAINTHREAD HAS DIFFERENT REFERENCE VALUE = {}! MUST BE = {}!",
							   _ref,
							   LUA_RIDX_MAINTHREAD);

		return VM(_state);
	}

	void VM::Close() noexcept
	{
		if(!IsOpen())
			return;

		lua_close(state);
		state = nullptr;
	}
};
