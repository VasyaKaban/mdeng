#include "VMBase.h"
#include "Common.h"
#include "Stack.h"
#include "VmType.h"
#include <lua.hpp>

namespace LuaWay
{
    VMBase::VMBase(lua_State* _state) noexcept
        : state(_state)
    {}

    void VMBase::RegisterLibrary(const char* library_name,
                                 std::span<const luaL_Reg> functions) const noexcept
    {
        hrs::assert_true_debug(IsOpen(), "Lua VM isn't opened yet!");

        luaL_register(state, library_name, functions.data());
        lua_pop(state, 1);
    }

    bool VMBase::IsOpen() const noexcept
    {
        return state != nullptr;
    }

    hrs::expected<Ref, Status> VMBase::LoadString(const char* str) const noexcept
    {
        int res = load_string(str);
        if(res != 0)
        {
            //error
            Status status(static_cast<StatusCode>(res), lua_tostring(state, -1));
            lua_pop(state, 1);
            return status;
        }

        //chunk
        int ref = luaL_ref(state, LUA_REGISTRYINDEX);
        return Ref(state, ref);
    }

    hrs::expected<Ref, Status> VMBase::LoadFile(const std::filesystem::path fpath) const noexcept
    {
        int res = load_file(fpath);
        if(res != 0)
        {
            //error
            Status status(static_cast<StatusCode>(res), lua_tostring(state, -1));
            lua_pop(state, 1);
            return status;
        }

        //chunk
        int ref = luaL_ref(state, LUA_REGISTRYINDEX);
        return Ref(state, ref);
    }

    hrs::expected<FunctionResult, Status> VMBase::DoString(const char* str, Ref env) const noexcept
    {
        int res = load_string(str);
        if(res != 0)
        {
            //error
            Status status(static_cast<StatusCode>(res), lua_tostring(state, -1));
            lua_pop(state, 1);
            return status;
        }
        return do_chunk(env);
    }

    hrs::expected<FunctionResult, Status> VMBase::DoFile(const std::filesystem::path fpath,
                                                         Ref env) const noexcept
    {
        int res = load_file(fpath);
        if(res != 0)
        {
            //error
            Status status(static_cast<StatusCode>(res), lua_tostring(state, -1));
            lua_pop(state, 1);
            return status;
        }
        return do_chunk(env);
    }

    Thread VMBase::GetState() const noexcept
    {
        return state;
    }

    Ref VMBase::AllocateUserData(std::size_t size) const noexcept
    {
        hrs::assert_true_debug(IsOpen(), "Lua VM isn't opened yet!");

        lua_newuserdata(state, size);
        int ref = luaL_ref(state, LUA_REGISTRYINDEX);
        return Ref(state, ref);
    }

    Ref VMBase::CreateTable(int array_count, int table_count) const noexcept
    {
        hrs::assert_true_debug(IsOpen(), "Lua VM isn't opened yet!");

        lua_createtable(state, array_count, table_count);
        int ref = luaL_ref(state, LUA_REGISTRYINDEX);
        return Ref(state, ref);
    }

    Ref VMBase::CreateThread(CFunction c_func) const noexcept
    {
        hrs::assert_true_debug(IsOpen(), "Lua VM isn't opened yet!");

        if(!c_func)
            return {};

        Thread co = lua_newthread(state);
        if(!co)
            return {};

        lua_pushcfunction(co, c_func);
        int _ref = luaL_ref(state, LUA_REGISTRYINDEX);
        return Ref(state, _ref);
    }

    Ref VMBase::CreateThread(const Ref& r) const noexcept
    {
        hrs::assert_true_debug(IsOpen(), "Lua VM isn't opened yet!");

        VmType vm_type = r.GetType();
        if(!(vm_type == VmType::CFunction || vm_type == VmType::Function))
            return {};

        Thread co = lua_newthread(state);
        if(!co)
            return {};

        Stack<Ref>::Push(co, r);
        int _ref = luaL_ref(state, LUA_REGISTRYINDEX);
        return Ref(state, _ref);
    }

    CFunction VMBase::SetAtPanic(CFunction at_panic_func) const noexcept
    {
        hrs::assert_true_debug(IsOpen(), "Lua VM isn't opened yet!");

        return lua_atpanic(state, at_panic_func);
    }

    void VMBase::SetAllocator(lua_Alloc allocator, void* data) const noexcept
    {
        hrs::assert_true_debug(IsOpen(), "Lua VM isn't opened yet!");

        lua_setallocf(state, allocator, data);
    }

    void VMBase::StopGC() const noexcept
    {
        hrs::assert_true_debug(IsOpen(), "Lua VM isn't opened yet!");

        lua_gc(state, LUA_GCSTOP, 0);
    }

    void VMBase::RestartGC() const noexcept
    {
        hrs::assert_true_debug(IsOpen(), "Lua VM isn't opened yet!");

        lua_gc(state, LUA_GCRESTART, 0);
    }

    void VMBase::CollectGC() const noexcept
    {
        hrs::assert_true_debug(IsOpen(), "Lua VM isn't opened yet!");

        lua_gc(state, LUA_GCCOLLECT, 0);
    }

    std::size_t VMBase::GetUsedMemory() const noexcept
    {
        hrs::assert_true_debug(IsOpen(), "Lua VM isn't opened yet!");

        std::size_t kbytes = lua_gc(state, LUA_GCCOUNT, 0);
        return kbytes * 1024;
    }

    void VMBase::SetGCPause(int pause) const noexcept
    {
        hrs::assert_true_debug(IsOpen(), "Lua VM isn't opened yet!");

        lua_gc(state, LUA_GCSETPAUSE, pause);
    }

    void VMBase::MakeGCStep(int step_size) const noexcept
    {
        hrs::assert_true_debug(IsOpen(), "Lua VM isn't opened yet!");

        lua_gc(state, LUA_GCSTEP, step_size);
    }

    void VMBase::SetGCStepMul(int step_mul) const noexcept
    {
        hrs::assert_true_debug(IsOpen(), "Lua VM isn't opened yet!");

        lua_gc(state, LUA_GCSETSTEPMUL, step_mul);
    }

    Ref VMBase::GetStackValue(int index) const noexcept
    {
        hrs::assert_true_debug(IsOpen(), "Lua VM isn't opened yet!");

        VmType vm_type = GetType(state, index);
        if(vm_type == VmType::None)
            return {};

        lua_pushvalue(state, index);
        int ref = luaL_ref(state, LUA_REGISTRYINDEX);
        return Ref(state, ref);
    }

    VmType VMBase::GetStackValueType(int index) const noexcept
    {
        hrs::assert_true_debug(IsOpen(), "Lua VM isn't opened yet!");

        return GetType(state, index);
    }

    std::size_t VMBase::GetStackFillness() const noexcept
    {
        hrs::assert_true_debug(IsOpen(), "Lua VM isn't opened yet!");

        return lua_gettop(state);
    }

    void VMBase::SetEnvForGlobal(const char* name, Ref env) const noexcept
    {
        hrs::assert_true_debug(IsOpen(), "Lua VM isn't opened yet!");
        hrs::assert_true_debug(IsOpen(), "Lua env reference isn't created yet!");

        lua_getglobal(state, name);
        Stack<Ref>::Push(state, env);
        lua_setfenv(state, -2);
        lua_pop(state, 1);
    }

    void VMBase::DropGlobal(const char* name) const noexcept
    {
        SetGlobal(name, nil);
    }

    Ref VMBase::GetGlobal(const char* name) const noexcept
    {
        hrs::assert_true_debug(IsOpen(), "Lua VM isn't opened yet!");

        lua_getglobal(state, name);
        int ref = luaL_ref(state, LUA_REGISTRYINDEX);
        return Ref(state, ref);
    }

    int VMBase::load_string(const char* str) const noexcept
    {
        hrs::assert_true_debug(IsOpen(), "Lua VM isn't opened yet!");

        return luaL_loadstring(state, str);
    }

    int VMBase::load_file(const std::filesystem::path& fpath) const noexcept
    {
        hrs::assert_true_debug(IsOpen(), "Lua VM isn't opened yet!");

        return luaL_loadfile(state, fpath.string().c_str());
    }

    hrs::expected<FunctionResult, Status> VMBase::do_chunk(Ref env) const noexcept
    {
        if(env.Holds(VmType::Table))
        {
            Stack<Ref>::Push(state, env);
            lua_setfenv(state, -2);
        }

        //chunk
        int pre_push_index = lua_gettop(state) - 1;
        int res = lua_pcall(state, 0, LUA_MULTRET, 0);
        int post_call_index = lua_gettop(state);
        if(res != 0) //error
        {
            //message
            Status status(static_cast<StatusCode>(res), lua_tostring(state, -1));
            lua_pop(state, 1);
            return status;
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
        return result;
    }
};
