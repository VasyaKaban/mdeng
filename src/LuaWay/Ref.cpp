#include "Ref.h"
#include "RefIterator.h"

namespace LuaWay
{
	Ref::Ref(lua_State *_state, int _ref) noexcept
		: state(_state),
		  ref(_ref) {}

	Ref::Ref() noexcept
		: state(nullptr),
		  ref(LUA_REFNIL) {}

	Ref::~Ref()
	{
		Unref();
	}

	Ref::Ref(const Ref &r) noexcept
		: state(r.state)
	{
		if(!r.IsCreated())
			ref = LUA_REFNIL;
		else
		{
			Stack<Ref>::Push(r.state, r);
			int _ref = luaL_ref(r.state, LUA_REGISTRYINDEX);
			ref = _ref;
		}
	}

	Ref::Ref(Ref &&r) noexcept
		: state(std::exchange(r.state, nullptr)),
		  ref(r.ref) {}

	Ref & Ref::operator=(const Ref &r) noexcept
	{
		Unref();

		state = r.state;
		if(!r.IsCreated())
			ref = LUA_REFNIL;
		else
		{
			Stack<Ref>::Push(r.state, r);
			int _ref = luaL_ref(r.state, LUA_REGISTRYINDEX);
			ref = _ref;
		}

		return *this;
	}

	Ref & Ref::operator=(Ref &&r) noexcept
	{
		Unref();

		state = std::exchange(r.state, nullptr);
		ref = r.ref;

		return *this;
	}

	bool Ref::IsCreated() const noexcept
	{
		return state != nullptr;
	}

	void Ref::Unref() noexcept
	{
		if(!IsCreated())
			return;

		luaL_unref(state, LUA_REGISTRYINDEX, ref);
		state = nullptr;
	}

	VmType Ref::GetType() const noexcept
	{
		if(!IsCreated())
			return VmType::None;

		//hrs::assert_true_debug(IsCreated(), "Lua reference isn't created yet!");

		Stack<Ref>::Push(state, *this);
		VmType vm_type = LuaWay::GetType(state, -1);
		lua_pop(state, 1);
		return vm_type;
	}

	bool Ref::Holds(VmType vm_type) const noexcept
	{
		return this->GetType() == vm_type;
	}

	lua_State * Ref::GetState() const noexcept
	{
		return state;
	}

	int Ref::GetRawIndex() const noexcept
	{
		return ref;
	}

	Ref Ref::GetMetatable() const noexcept
	{
		hrs::assert_true_debug(IsCreated(), "Lua reference isn't created yet!");

		Stack<Ref>::Push(state, *this);
		int res = lua_getmetatable(state, -1);
		if(res == 0)
		{
			lua_pop(state, 1);
			return Ref{};
		}

		Ref mt = Stack<Ref>::Retrieve(state, -1);

		lua_pop(state, 2);
		return mt;
	}

	void Ref::SetMetatable(Ref mt) const noexcept
	{
		hrs::assert_true_debug(IsCreated(), "Lua reference isn't created yet!");

		Stack<Ref>::Push(state, *this);
		Stack<Ref>::Push(state, mt);
		lua_setmetatable(state, -2);
		lua_pop(state, 1);
	}

	std::size_t Ref::GetLength() const noexcept
	{
		hrs::assert_true_debug(IsCreated(), "Lua reference isn't created yet!");

		Stack<Ref>::Push(state, *this);
		std::size_t len = lua_objlen(state, -1);
		lua_pop(state, 1);
		return len;
	}

	void Ref::SetEnv(Ref env) const noexcept
	{
		hrs::assert_true_debug(IsCreated(), "Lua reference isn't created yet!");
		hrs::assert_true_debug(IsCreated(), "Lua env reference isn't created yet!");

		Stack<Ref>::Push(state, *this);
		Stack<Ref>::Push(state, env);
		lua_setfenv(state, -2);
		lua_pop(state, 1);
	}

	RefIterator Ref::begin() const noexcept
	{
		hrs::assert_true_debug(IsCreated(), "Lua reference isn't created yet!");

		lua_rawseti(state, LUA_REGISTRYINDEX, ref);
		lua_pushnil(state);
		//ref, nil
		int res = lua_next(state, -2);
		if(res == 0)
		{
			//ref
			lua_pop(state, 1);
			return end();
		}

		//ref, key, value
		lua_pop(state, 1);
		//ref, key
		RefIterator iter(state, ref, luaL_ref(state, LUA_REGISTRYINDEX));
		//ref
		lua_pop(state, 1);
		return iter;
	}

	RefIterator Ref::end() const noexcept
	{
		return RefIterator{};
	}

	static int ref_dump_writer(lua_State *state,
							   const void *p,
							   std::size_t sz,
							   void *ud)
	{
		std::string *out = std::launder(reinterpret_cast<std::string *>(ud));
		out->append(static_cast<const char *>(p), sz);
		return 0;
	}

	hrs::expected<std::string, Status> Ref::Dump() const
	{
		hrs::assert_true_debug(IsCreated(), "Lua reference isn't created yet!");

		Stack<Ref>::Push(state, *this);
		if(GetType() != VmType::Function)
		{
			lua_pop(state, 1);
			return "";
		}

		std::string out;
		out.reserve(1024);
		int res = lua_dump(state, ref_dump_writer, &out);
		if(res != 0)
		{
			//func, error
			Status status(static_cast<StatusCode>(res), lua_tostring(state, -1));
			lua_pop(state, 2);
			return status;
		}

		//func
		lua_pop(state, 1);
		return out;
	}

	void Stack<Ref>::Push(lua_State *state, const Type &value) noexcept
	{
		lua_rawgeti(state, LUA_REGISTRYINDEX, value.GetRawIndex());
	}

	void Stack<Ref>::Push(lua_State *state, Type &&value) noexcept
	{
		lua_rawgeti(state, LUA_REGISTRYINDEX, value.GetRawIndex());
	}

	Stack<Ref>::Type Stack<Ref>::Retrieve(lua_State *state, int index) noexcept
	{
		lua_rawgeti(state, LUA_REGISTRYINDEX, LUA_RIDX_MAINTHREAD);
		lua_State *main_thread = lua_tothread(state, -1);
		lua_pop(state, 1);

		lua_pushvalue(state, index);
		int raw = luaL_ref(state, LUA_REGISTRYINDEX);
		return Ref(main_thread, raw);
	}

	bool Stack<Ref>::ConvertibleFromVm(VmType vm_type) noexcept
	{
		return true;
	}
};
