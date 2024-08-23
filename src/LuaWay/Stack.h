#pragma once

#include <concepts>
#include <utility>
#include "VmType.h"

namespace LuaWay
{
	template<typename T>
	struct Stack
	{
		using Type = T;

		static bool ConvertibleFromVm(VmType vm_type) = delete;
	};

	template<typename T>
	concept Pushable = requires(lua_State *state, T &&value)
	{
		Stack<std::remove_cvref_t<T>>::Push(state, std::forward<T>(value));
	};

	template<typename T>
	concept NoexceptPushable = requires(lua_State *state, T &&value)
	{
		{Stack<std::remove_cvref_t<T>>::Push(state, std::forward<T>(value))} noexcept;
	};

	template<typename T>
	concept Retrievable = requires(lua_State *state)
	{
		{Stack<std::remove_cvref_t<T>>::Retrieve(state, 0)} -> std::convertible_to<std::remove_cvref_t<T>>;
	};

	template<typename T>
	concept NoexceptRetrievable = requires(lua_State *state)
	{
		{Stack<std::remove_cvref_t<T>>::Retrieve(state, 0)} noexcept
			  -> std::convertible_to<std::remove_cvref_t<T>>;
	};
};
