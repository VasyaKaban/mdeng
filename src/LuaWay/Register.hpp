#pragma once

#include <tuple>
#include <cassert>
#include <format>
#include "hrs/function_traits.hpp"
#include "hrs/member_class.hpp"
#include "NativeType.h"

namespace LuaWay
{
	namespace detail
	{
		[[noreturn]] void emit_error(lua_State *state, std::string_view message) noexcept
		{
			lua_pushlstring(state, message.data(), message.size());
			lua_error(state);
			assert(false);
		}

		template<auto func, typename R, typename ...Args, std::size_t ...Indices>
		int call_plain(lua_State *state, std::index_sequence<Indices...>) noexcept
		{
#warning ADD SYNC FOR REF.state because it can be called from coroutine!!! -> CHECK LUA_RIDX_MAINTHREAD
			if constexpr(std::same_as<R, void>)
			{
				func(std::forward<Args>(Stack<std::remove_cvref_t<Args>>::
										Retrieve(state, static_cast<int>(Indices) -
														static_cast<int>(sizeof...(Args))))...);

				return 0;
			}
			else
			{
				Stack<std::remove_cvref_t<R>>::Push(state,
					func(std::forward<Args>(Stack<std::remove_cvref_t<Args>>::
											Retrieve(state, static_cast<int>(Indices) -
															static_cast<int>(sizeof...(Args))))
						 ...));

				return 1;
			}
		}

		template<auto func, typename C, typename R, typename ...Args, std::size_t ...Indices>
		int call_member(lua_State *state, C *obj, std::index_sequence<Indices...>) noexcept
		{
			if constexpr(std::same_as<R, void>)
			{
				(obj->*func)(std::forward<Args>(Stack<std::remove_cvref_t<Args>>::
												Retrieve(state, static_cast<int>(Indices) -
																static_cast<int>(sizeof...(Args))))
							 ...);

				return 0;
			}
			else
			{
				Stack<std::remove_cvref_t<R>>::Push(state,
					(obj->*func)(std::forward<Args>(Stack<std::remove_cvref_t<Args>>::
													Retrieve(state, static_cast<int>(Indices) -
																	static_cast<int>(sizeof...(Args))))
								 ...));

				return 1;
			}
		}

		template<auto func, typename C, typename R, typename ...Args>
		int function_wrapper(lua_State *state) noexcept
		{
			using vars = hrs::variadic<Args...>;
			int stack_args_count = lua_gettop(state);
			//call(arg0, arg1, arg2)
			//        B  B + 1 B + 2
			//                 top
			//2 -1
			//1 -2
			//0 - 3

			if constexpr(std::same_as<C, void>)//plain
			{
				if(stack_args_count < sizeof...(Args))
					emit_error(state, std::format("Too few arguments! Expected = {} but received = {}!",
												  sizeof...(Args),
												  stack_args_count));

				return call_plain<func, R, Args...>(state, std::make_index_sequence<sizeof...(Args)>{});
			}
			else//class member
			{
				if(stack_args_count < sizeof...(Args) + 1)
					emit_error(state, std::format("Too few arguments! Expected = {} but received = {}!",
												  sizeof...(Args) + 1,
												  stack_args_count));

				C *obj = static_cast<C *>(lua_touserdata(state, -static_cast<int>(sizeof...(Args) + 1)));
				return call_member<func, C, R, Args...>(state,
														obj,
														std::make_index_sequence<sizeof...(Args)>{});
			}
		}

		template<auto func, bool is_class_member>
		struct select_func_class_tratis;

		template<auto func>
		struct select_func_class_tratis<func, false>
		{
			using class_t = void;
			using f_traits = hrs::function_traits<std::remove_pointer_t<decltype(func)>>;
		};

		template<auto func>
		struct select_func_class_tratis<func, true>
		{
			using class_t = hrs::member_class_type_class_t<decltype(func)>;
			using f_traits = hrs::function_traits<hrs::member_class_type_field_t<decltype(func)>>;
		};

		template<typename C>
		int destructor_wrapper(lua_State *state) noexcept
		{
			int stack_args_count = lua_gettop(state);
			if(stack_args_count < 1)
				emit_error(state, "Class object isn't provided!");

			C *obj = static_cast<C *>(lua_touserdata(state, -1));
			obj->~C();
			return 0;
		}

		template<typename C, typename ...Args>
		int constructor_wrapper(lua_State *state) noexcept
		{
			int stack_args_count = lua_gettop(state);
			if(stack_args_count < sizeof...(Args) + 1)
				emit_error(state, std::format("Too few arguments! Expected = {} but received = {}!",
											  sizeof...(Args) + 1,
											  stack_args_count));

			[state]<std::size_t ...Indices>(std::index_sequence<Indices...>)
			{
				Stack<std::remove_cvref_t<C>>::Push(state,
					C(std::forward<Args>(Stack<std::remove_cvref_t<Args>>::
										 Retrieve(state, static_cast<int>(Indices) -
														 static_cast<int>(sizeof...(Args))))
						 ...));
				//mt, args, obj
				//args = 2
				//-4
				//mt -> -(sizeof...(Args) + 2)
				lua_pushvalue(state, -static_cast<int>(sizeof...(Args) + 2));
				lua_setmetatable(state, -2);

			}(std::make_index_sequence<sizeof...(Args)>{});

			return 1;
		}
	};

	template<auto func>
		requires
			std::is_function_v<std::remove_pointer_t<decltype(func)>> ||
			(hrs::is_class_member_v<decltype(func)> &&
			 std::is_function_v<typename hrs::member_class_type<decltype(func)>::field_type>)
	CFunction WrapFunction() noexcept
	{
		using class_t =
			detail::select_func_class_tratis<func, hrs::is_class_member_v<decltype(func)>>::class_t;

		using f_traits =
			detail::select_func_class_tratis<func, hrs::is_class_member_v<decltype(func)>>::f_traits;

		if constexpr(!std::same_as<typename f_traits::return_type, void>)
			static_assert(Pushable<typename f_traits::return_type>, "Return type must be Pushable!");

		return []<typename ...Args>(hrs::variadic<Args...>)
		{
			static_assert(((Retrievable<Args>) && ...), "Function arguments must be Retrievable!");
			return detail::function_wrapper<func, class_t, typename f_traits::return_type, Args...>;
		}(typename f_traits::arguments{});
	}

	template<Pushable C, Retrievable ...Args>
		requires std::constructible_from<C, Args...>
	CFunction WrapConstructor() noexcept
	{
		return detail::constructor_wrapper<C, Args...>;
	}

	template<typename C>
	CFunction WrapDestructor() noexcept
	{
		return detail::destructor_wrapper<C>;
	}
};
