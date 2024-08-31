#pragma once

#include <optional>
#include "hrs/expected.hpp"
#include "hrs/debug.hpp"
#include "hrs/ref.hpp"
#include "Common.h"
#include "Status.h"
#include "FunctionResult.h"

namespace LuaWay
{
	struct RefCallPlain {};
	struct RefCallProtected {};

	class RefIterator;

	class Ref
	{
	private:
		Ref(lua_State *_state, int _ref) noexcept;
	public:
		friend struct Stack<Ref>;
		friend class RefIterator;
		friend class VMBase;

		Ref() noexcept;
		~Ref();
		Ref(const Ref &r) noexcept;
		Ref(Ref &&r) noexcept;
		Ref & operator=(const Ref &r) noexcept;
		Ref & operator=(Ref &&r) noexcept;

		bool IsCreated() const noexcept;
		void Unref() noexcept;

		VmType GetType() const noexcept;

		bool Holds(VmType vm_type) const noexcept;

		lua_State * GetState() const noexcept;
		int GetRawIndex() const noexcept;

		Ref GetMetatable() const noexcept;
		void SetMetatable(Ref mt) const noexcept;

		std::size_t GetLength() const noexcept;

		void SetEnv(Ref env) const noexcept;

		template<Retrievable T>
		std::optional<T> Retrieve() const noexcept(NoexceptRetrievable<T>);

		template<typename Call, Pushable ...Args>
			requires std::same_as<Call, RefCallPlain> || std::same_as<Call, RefCallProtected>
		auto Call(Args &&...args) const;

		template<Pushable ...Args>
		hrs::expected<FunctionResult, Status> operator()(Args &&...args) const;

		template<Pushable K>
		Ref GetRef(K &&key) const noexcept(NoexceptPushable<K>);

		template<Pushable K>
		Ref GetRefRaw(K &&key) const noexcept(NoexceptPushable<K>);

		template<Retrievable T, Pushable K>
		auto Get(K &&key) const noexcept(NoexceptPushable<K> && NoexceptRetrievable<T>);

		template<Retrievable T, Pushable K>
		auto GetRaw(K &&key) const noexcept(NoexceptPushable<K> && NoexceptRetrievable<T>);

		template<Pushable K, Pushable V>
		void Set(K &&key, V &&value) const noexcept(NoexceptPushable<K> && NoexceptPushable<V>);

		template<Pushable K, Pushable V>
		void SetRaw(K &&key, V &&value) const noexcept(NoexceptPushable<K> && NoexceptPushable<V>);

		template<Retrievable T>
		auto As() const noexcept(NoexceptRetrievable<T>);

		RefIterator begin() const noexcept;
		RefIterator end() const noexcept;

		hrs::expected<std::string, Status> Dump() const;

	private:
		lua_State *state;
		int ref;
	};

	template<>
	struct Stack<Ref>
	{
		using Type = Ref;

		static void Push(lua_State *state, const Type &value) noexcept;
		static void Push(lua_State *state, Type &&value) noexcept;

		static Type Retrieve(lua_State *state, int index) noexcept;

		static bool ConvertibleFromVm(VmType vm_type) noexcept;
	};

	template<Retrievable T>
	std::optional<T> Ref::Retrieve() const noexcept(NoexceptRetrievable<T>)
	{
		VmType vm_type = LuaWay::GetType(state, -1);
		if(!Stack<T>::ConvertibleFromVm(vm_type))
			return {};

		return Stack<T>::Retrieve(state, -1);
	}

	template<typename Call, Pushable ...Args>
		requires std::same_as<Call, RefCallPlain> || std::same_as<Call, RefCallProtected>
	auto Ref::Call(Args &&...args) const
	{
		hrs::assert_true_debug(IsCreated(), "Lua reference isn't created yet!");

		using Out = std::conditional_t<std::same_as<Call, RefCallPlain>,
									   FunctionResult,
									   hrs::expected<FunctionResult, Status>>;

		lua_getref(state, ref);
		((Stack<std::remove_cvref_t<Args>>::Push(std::forward<Args>(args))), ...);
		int pre_push_index = lua_gettop(state);
		int post_call_index;
		if constexpr(std::same_as<Call, RefCallProtected>)
		{
			int res = lua_pcall(state, sizeof...(args), LUA_MULTRET, 0);
			post_call_index = lua_gettop(state);
			if(res != 0)//error
			{
				//message
				Status status(static_cast<StatusCode>(res), lua_tostring(state, -1));
				lua_pop(state, 1);
				return Out{status};
			}
		}
		else
		{
			lua_call(state, sizeof...(args), LUA_MULTRET);
			post_call_index = lua_gettop(state);
		}

		//<post>, results, <pre>
		std::size_t result_count = post_call_index - pre_push_index;

		FunctionResult result;
		result.reserve(result_count);
		for(std::size_t i = 1; i <= result_count; i++)
		{
			auto obj = Stack<Ref>::Retrieve(state, -i);
			result.push_back(std::move(obj));
		}

		lua_pop(state, result_count);
		return Out{result};
	}

	template<Pushable ...Args>
	hrs::expected<FunctionResult, Status> Ref::operator()(Args &&...args) const
	{
		return Call<RefCallProtected>(std::forward<Args>(args)...);
	}

	template<Pushable K>
	Ref Ref::GetRef(K &&key) const noexcept(NoexceptPushable<K>)
	{
		hrs::assert_true_debug(IsCreated(), "Lua reference isn't created yet!");

		Stack<Ref>::Push(state, *this);
		Stack<Ref>::Push(state, std::forward<K>(key));
		lua_gettable(state, -2);
		//ref, value
		int _ref = luaL_ref(state, LUA_REGISTRYINDEX);
		lua_pop(state, 1);
		return Ref(state, _ref);
	}

	template<Pushable K>
	Ref Ref::GetRefRaw(K &&key) const noexcept(NoexceptPushable<K>)
	{
		hrs::assert_true_debug(IsCreated(), "Lua reference isn't created yet!");

		Stack<Ref>::Push(state, *this);
		Stack<Ref>::Push(state, std::forward<K>(key));
		lua_rawget(state, -2);
		//ref, value
		int _ref = luaL_ref(state, LUA_REGISTRYINDEX);
		lua_pop(state, 1);
		return Ref(state, _ref);
	}

	template<Retrievable T, Pushable K>
	auto Ref::Get(K &&key) const noexcept(NoexceptPushable<K> && NoexceptRetrievable<T>)
	{
		hrs::assert_true_debug(IsCreated(), "Lua reference isn't created yet!");

		using out_t = detail::retrieve_value_type_for_non_ref_wrapper<T>::out_t;

		Stack<Ref>::Push(state, *this);
		Stack<Ref>::Push(state, std::forward<K>(key));
		lua_gettable(state, -2);
		//ref, value
		VmType vm_type = LuaWay::GetType(state, -1);
		if(!Stack<T>::ConvertibleFromVm(vm_type))
		{
			lua_pop(state, 2);
			return std::optional<out_t>{};
		}

		auto out_val = detail::retrieve_value_for_non_ref_wrapper<T>(state, -1);
		lua_pop(state, 2);
		return std::optional<out_t>{out_val};
	}

	template<Retrievable T, Pushable K>
	auto Ref::GetRaw(K &&key) const noexcept(NoexceptPushable<K> &&
											 NoexceptRetrievable<T>)
	{
		hrs::assert_true_debug(IsCreated(), "Lua reference isn't created yet!");

		using out_t = detail::retrieve_value_type_for_non_ref_wrapper<T>::out_t;

		Stack<Ref>::Push(state, *this);
		Stack<Ref>::Push(state, std::forward<K>(key));
		lua_rawget(state, -2);
		//ref, value
		VmType vm_type = LuaWay::GetType(state, -1);
		if(!Stack<T>::ConvertibleFromVm(vm_type))
		{
			lua_pop(state, 2);
			return std::optional<out_t>{};
		}

		auto out_val = detail::retrieve_value_for_non_ref_wrapper<T>(state, -1);
		lua_pop(state, 2);
		return std::optional<out_t>{out_val};
	}

	template<Pushable K, Pushable V>
	void Ref::Set(K &&key, V &&value) const noexcept(NoexceptPushable<K> && NoexceptPushable<V>)
	{
		hrs::assert_true_debug(IsCreated(), "Lua reference isn't created yet!");

		Stack<Ref>::Push(state, *this);
		Stack<K>::Push(state, std::forward<K>(key));
		Stack<V>::Push(state, std::forward<V>(value));
		//ref, key, value
		lua_settable(state, -3);
		lua_pop(state, 1);
	}

	template<Pushable K, Pushable V>
	void Ref::SetRaw(K &&key, V &&value) const noexcept(NoexceptPushable<K> && NoexceptPushable<V>)
	{
		hrs::assert_true_debug(IsCreated(), "Lua reference isn't created yet!");

		Stack<Ref>::Push(state, *this);
		Stack<std::remove_cvref_t<K>>::Push(state, std::forward<K>(key));
		Stack<std::remove_cvref_t<V>>::Push(state, std::forward<V>(value));
		//ref, key, value
		lua_rawset(state, -3);
		lua_pop(state, 1);
	}

	template<Retrievable T>
	auto Ref::As() const noexcept(NoexceptRetrievable<T>)
	{
		hrs::assert_true_debug(IsCreated(), "Lua reference isn't created yet!");

		using out_t = detail::retrieve_value_type_for_non_ref_wrapper<T>::out_t;

		Stack<Ref>::Push(state, *this);
		if(!Stack<T>::ConvertibleFromVm(LuaWay::GetType(state, -1)))
		{
			lua_pop(state, 1);
			return std::optional<out_t>{};
		}

		auto out_val = detail::retrieve_value_for_non_ref_wrapper<T>(state, -1);
		lua_pop(state, 1);
		return std::optional<out_t>{out_val};
	}
};
