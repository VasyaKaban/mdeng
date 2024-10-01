#include "NativeType.h"

namespace LuaWay
{
    //Nil
    void Stack<Nil>::Push(lua_State* state, Type value) noexcept
    {
        lua_pushnil(state);
    }

    Stack<Nil>::Type Stack<Nil>::Retrieve(lua_State* state, int index) noexcept
    {
        return nil;
    }

    bool Stack<Nil>::ConvertibleFromVm(VmType vm_type) noexcept
    {
        return vm_type == VmType::Nil;
    }

    //Bool
    void Stack<Bool>::Push(lua_State* state, Type value) noexcept
    {
        lua_pushboolean(state, value);
    }

    Stack<Bool>::Type Stack<Bool>::Retrieve(lua_State* state, int index) noexcept
    {
        return lua_toboolean(state, index);
    }

    bool Stack<Bool>::ConvertibleFromVm(VmType vm_type) noexcept
    {
        return vm_type == VmType::Bool;
    }

    //LightUserData
    void Stack<LightUserData>::Push(lua_State* state, Type value) noexcept
    {
        lua_pushlightuserdata(state, value.data);
    }

    Stack<LightUserData>::Type Stack<LightUserData>::Retrieve(lua_State* state, int index) noexcept
    {
        return LightUserData{lua_touserdata(state, index)};
    }

    bool Stack<LightUserData>::ConvertibleFromVm(VmType vm_type) noexcept
    {
        return vm_type == VmType::LightUserData;
    }

    //Number
    void Stack<Number>::Push(lua_State* state, Type value) noexcept
    {
        lua_pushnumber(state, value);
    }

    Stack<Number>::Type Stack<Number>::Retrieve(lua_State* state, int index) noexcept
    {
        return lua_tonumber(state, index);
    }

    bool Stack<Number>::ConvertibleFromVm(VmType vm_type) noexcept
    {
        return vm_type == VmType::Number;
    }

    //Integer
    void Stack<Integer>::Push(lua_State* state, Type value) noexcept
    {
        lua_pushinteger(state, value);
    }

    Stack<Integer>::Type Stack<Integer>::Retrieve(lua_State* state, int index) noexcept
    {
        return lua_tointeger(state, index);
    }

    bool Stack<Integer>::ConvertibleFromVm(VmType vm_type) noexcept
    {
        return vm_type == VmType::Number;
    }

    //String
    void Stack<String>::Push(lua_State* state, const Type& value) noexcept
    {
        lua_pushlstring(state, value.c_str(), value.size());
    }

    Stack<String>::Type Stack<String>::Retrieve(lua_State* state, int index)
    {
        std::size_t len = 0;
        const char* str = lua_tolstring(state, index, &len);
        return String{str, len};
    }

    bool Stack<String>::ConvertibleFromVm(VmType vm_type) noexcept
    {
        return vm_type == VmType::String;
    }

    //UserData
    Stack<UserData>::Type Stack<UserData>::Retrieve(lua_State* state, int index) noexcept
    {
        return UserData{lua_touserdata(state, index)};
    }

    bool Stack<UserData>::ConvertibleFromVm(VmType vm_type) noexcept
    {
        return vm_type == VmType::UserData;
    }

    //Thread
    Stack<Thread>::Type Stack<Thread>::Retrieve(lua_State* state, int index) noexcept
    {
        return lua_tothread(state, index);
    }

    bool Stack<Thread>::ConvertibleFromVm(VmType vm_type) noexcept
    {
        return vm_type == VmType::Thread;
    }

    //CFunction
    void Stack<CFunction>::Push(lua_State* state, Type value) noexcept
    {
        lua_pushcfunction(state, value);
    }

    Stack<CFunction>::Type Stack<CFunction>::Retrieve(lua_State* state, int index) noexcept
    {
        return lua_tocfunction(state, index);
    }

    bool Stack<CFunction>::ConvertibleFromVm(VmType vm_type) noexcept
    {
        return vm_type == VmType::CFunction;
    }
};
