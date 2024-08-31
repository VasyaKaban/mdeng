#pragma once

#include <cstring>
#include <string>
#include <variant>
#include "Stack.h"

namespace LuaWay
{
	class Ref;

	struct _Nil {};

	constexpr inline _Nil nil;

	struct _LightUserData
	{
		void *data;
	};

	struct _UserData
	{
		void *data;
	};

	using Nil = _Nil;
	using Bool = bool;
	using LightUserData = _LightUserData;
	using Number = lua_Number;
	using Integer = lua_Integer;
	using String = std::string;
	using UserData = _UserData;
	using Thread = lua_State *;
	using CFunction = lua_CFunction;

	template<>
	struct Stack<Nil>
	{
		using Type = Nil;

		static void Push(lua_State *state, Type value) noexcept;
		static Type Retrieve(lua_State *state, int index) noexcept;
		static bool ConvertibleFromVm(VmType vm_type) noexcept;
	};

	template<>
	struct Stack<Bool>
	{
		using Type = Bool;

		static void Push(lua_State *state, Type value) noexcept;
		static Type Retrieve(lua_State *state, int index) noexcept;
		static bool ConvertibleFromVm(VmType vm_type) noexcept;
	};

	template<>
	struct Stack<LightUserData>
	{
		using Type = LightUserData;

		static void Push(lua_State *state, Type value) noexcept;
		static Type Retrieve(lua_State *state, int index) noexcept;
		static bool ConvertibleFromVm(VmType vm_type) noexcept;
	};

	template<>
	struct Stack<Number>
	{
		using Type = Number;

		static void Push(lua_State *state, Type value) noexcept;
		static Type Retrieve(lua_State *state, int index) noexcept;
		static bool ConvertibleFromVm(VmType vm_type) noexcept;
	};

	template<>
	struct Stack<Integer>
	{
		using Type = Integer;

		static void Push(lua_State *state, Type value) noexcept;
		static Type Retrieve(lua_State *state, int index) noexcept;
		static bool ConvertibleFromVm(VmType vm_type) noexcept;
	};

	template<>
	struct Stack<String>
	{
		using Type = String;

		static void Push(lua_State *state, const Type &value) noexcept;

		static Type Retrieve(lua_State *state, int index);

		static bool ConvertibleFromVm(VmType vm_type) noexcept;
	};

	template<>
	struct Stack<UserData>
	{
		using Type = UserData;

		static Type Retrieve(lua_State *state, int index) noexcept;
		static bool ConvertibleFromVm(VmType vm_type) noexcept;
	};

	template<>
	struct Stack<Thread>
	{
		using Type = Thread;

		static Type Retrieve(lua_State *state, int index) noexcept;
		static bool ConvertibleFromVm(VmType vm_type) noexcept;
	};

	template<>
	struct Stack<CFunction>
	{
		using Type = CFunction;

		static void Push(lua_State *state, Type value) noexcept;
		static Type Retrieve(lua_State *state, int index) noexcept;
		static bool ConvertibleFromVm(VmType vm_type) noexcept;
	};

	template<std::size_t N>
	struct Stack<char [N]>
	{
		using Type = char [N];

		static void Push(lua_State *state, const Type &value) noexcept
		{
			lua_pushstring(state, value);
		}

		static bool ConvertibleFromVm(VmType vm_type) noexcept
		{
			return vm_type == VmType::String;
		}
	};
};
