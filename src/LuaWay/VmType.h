#pragma once

#include <lua5.1/lua.hpp>
#include "hrs/meta/enum_meta.hpp"

namespace LuaWay
{
	enum class VmType
	{
		None = LUA_TNONE,
		Nil = LUA_TNIL,
		Bool = LUA_TBOOLEAN,
		LightUserData = LUA_TLIGHTUSERDATA,
		Number = LUA_TNUMBER,
		String = LUA_TSTRING,
		Table = LUA_TTABLE,
		Function = LUA_TFUNCTION,
		UserData = LUA_TUSERDATA,
		Thread = LUA_TTHREAD,
		CFunction
	};

	VmType GetType(lua_State *state, int index) noexcept;
};

#include "hrs/meta/enum_meta_def.hpp"

HRS_REFL_ENUM_BEGIN_EXISTED(LuaWay::VmType,
							None,
							Nil,
							Bool,
							LightUserData,
							Number,
							String,
							Table,
							Function,
							UserData,
							Thread,
							CFunction)
HRS_REFL_ENUM_BODY()
HRS_REFL_ENUM_END()

#include "hrs/meta/enum_meta_undef.hpp"
