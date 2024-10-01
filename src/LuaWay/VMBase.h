#pragma once

#include "FunctionResult.h"
#include "NativeType.h"
#include "Ref.h"
#include "Stack.h"
#include "Status.h"
#include "hrs/debug.hpp"
#include "hrs/expected.hpp"
#include <filesystem>
#include <optional>
#include <span>
#include <string_view>

namespace LuaWay
{
    class VMBase
    {
    protected:
        VMBase(lua_State* _state) noexcept;
    public:
        VMBase() = default;
        ~VMBase() = default;
        VMBase(const VMBase&) = default;
        VMBase(VMBase&& vm) = default;
        VMBase& operator=(const VMBase&) = default;
        VMBase& operator=(VMBase&& vm) = default;

        void RegisterLibrary(const char* library_name,
                             std::span<const luaL_Reg> functions) const noexcept;

        bool IsOpen() const noexcept;

        hrs::expected<Ref, Status> LoadString(const char* str) const noexcept;
        hrs::expected<Ref, Status> LoadFile(const std::filesystem::path fpath) const noexcept;

        hrs::expected<FunctionResult, Status> DoString(const char* str, Ref env) const noexcept;
        hrs::expected<FunctionResult, Status> DoFile(const std::filesystem::path fpath,
                                                     Ref env) const noexcept;

        Thread GetState() const noexcept;

        Ref AllocateUserData(std::size_t size) const noexcept;
        Ref CreateTable(int array_count, int table_count) const noexcept;

        Ref CreateThread(CFunction c_func) const noexcept;
        Ref CreateThread(const Ref& r) const noexcept;

        CFunction SetAtPanic(CFunction at_panic_func) const noexcept;
        void SetAllocator(lua_Alloc allocator, void* data) const noexcept;

        void StopGC() const noexcept;
        void RestartGC() const noexcept;
        void CollectGC() const noexcept;
        std::size_t GetUsedMemory() const noexcept;
        void SetGCPause(int pause) const noexcept;
        void MakeGCStep(int step_size) const noexcept;
        void SetGCStepMul(int step_mul) const noexcept;

        Ref GetStackValue(int index) const noexcept;
        VmType GetStackValueType(int index) const noexcept;
        std::size_t GetStackFillness() const noexcept;

        void SetEnvForGlobal(const char* name, Ref env) const noexcept;

        void DropGlobal(const char* name) const noexcept;

        Ref GetGlobal(const char* name) const noexcept;

        template<Retrievable T>
        auto GetGlobalAs(const char* name) const noexcept(NoexceptRetrievable<T>);

        template<Pushable T>
        void SetGlobal(const char* name, T&& value) const noexcept(NoexceptPushable<T>);

        template<Pushable T>
        Ref CreateRef(T&& value) const noexcept(NoexceptPushable<T>);
    private:
        int load_string(const char* str) const noexcept;
        int load_file(const std::filesystem::path& fpath) const noexcept;
        hrs::expected<FunctionResult, Status> do_chunk(Ref env) const noexcept;
    protected:
        lua_State* state;
    };

    template<Retrievable T>
    auto VMBase::GetGlobalAs(const char* name) const noexcept(NoexceptRetrievable<T>)
    {
        hrs::assert_true_debug(IsOpen(), "Lua VM isn't opened yet!");

        using out_t = detail::retrieve_value_type_for_non_ref_wrapper<T>::out_t;

        lua_getglobal(state, name);
        if(!Stack<T>::ConvertibleFromVm(GetType(state, -1)))
        {
            lua_pop(state, 1);
            return std::optional<out_t>{};
        }

        decltype(auto) out_value = detail::retrieve_value_for_non_ref_wrapper<T>(state, -1);
        lua_pop(state, 1);
        return std::optional<out_t>{out_value};
    }

    template<Pushable T>
    void VMBase::SetGlobal(const char* name, T&& value) const noexcept(NoexceptPushable<T>)
    {
        hrs::assert_true_debug(IsOpen(), "Lua VM isn't opened yet!");

        Stack<std::remove_cvref_t<T>>::Push(state, std::forward<T>(value));
        lua_setglobal(state, name);
    }

    template<Pushable T>
    Ref VMBase::CreateRef(T&& value) const noexcept(NoexceptPushable<T>)
    {
        hrs::assert_true_debug(IsOpen(), "Lua VM isn't opened yet!");

        Stack<std::remove_cvref_t<T>>::Push(state, std::forward<T>(value));
        int _ref = luaL_ref(state, LUA_REGISTRYINDEX);
        return Ref(state, _ref);
    }
};
