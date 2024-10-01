#include "RefIterator.h"
#include "Ref.h"
#include "hrs/debug.hpp"

namespace LuaWay
{
    RefIterator::RefIterator(lua_State* _state, int _iterable_ref, int _key_ref) noexcept
        : state(_state),
          iterable_ref(_iterable_ref),
          key_ref(_key_ref)
    {}

    RefIterator::RefIterator()
        : state(nullptr),
          iterable_ref(LUA_REFNIL),
          key_ref(LUA_REFNIL)
    {}

    RefIterator::~RefIterator()
    {
        destroy();
    }

    RefIterator::RefIterator(const RefIterator& ri) noexcept
        : state(ri.state)
    {
        if(ri.state == nullptr)
        {
            iterable_ref = LUA_REFNIL;
            key_ref = LUA_REFNIL;
        }
        else
        {
            lua_rawgeti(ri.state, LUA_REGISTRYINDEX, ri.iterable_ref);
            iterable_ref = luaL_ref(ri.state, LUA_REGISTRYINDEX);

            lua_rawgeti(ri.state, LUA_REGISTRYINDEX, ri.key_ref);
            key_ref = luaL_ref(ri.state, LUA_REGISTRYINDEX);
        }
    }

    RefIterator::RefIterator(RefIterator&& ri) noexcept
        : state(std::exchange(ri.state, nullptr)),
          iterable_ref(ri.iterable_ref),
          key_ref(ri.key_ref)
    {}

    RefIterator& RefIterator::operator=(const RefIterator& ri) noexcept
    {
        destroy();

        state = ri.state;
        if(ri.state == nullptr)
        {
            iterable_ref = LUA_REFNIL;
            key_ref = LUA_REFNIL;
        }
        else
        {
            lua_rawgeti(ri.state, LUA_REGISTRYINDEX, ri.iterable_ref);
            iterable_ref = luaL_ref(ri.state, LUA_REGISTRYINDEX);

            lua_rawgeti(ri.state, LUA_REGISTRYINDEX, ri.key_ref);
            key_ref = luaL_ref(ri.state, LUA_REGISTRYINDEX);
        }

        return *this;
    }

    RefIterator& RefIterator::operator=(RefIterator&& ri) noexcept
    {
        destroy();

        state = std::exchange(ri.state, nullptr);
        iterable_ref = ri.iterable_ref;
        key_ref = ri.key_ref;

        return *this;
    }

    std::pair<Ref, Ref> RefIterator::operator*() const noexcept //key, value
    {
        hrs::assert_true_debug(state != nullptr, "RefIterator must be valid!");

        lua_rawgeti(state, LUA_REGISTRYINDEX, iterable_ref);
        lua_rawgeti(state, LUA_REGISTRYINDEX, key_ref);
        //iter, key
        lua_gettable(state, -2);
        //iter, value
        int value_ref = luaL_ref(state, LUA_REGISTRYINDEX);
        //iter
        lua_pop(state, 1);

        return std::pair{Ref(state, key_ref), Ref(state, value_ref)};
    }

    RefIterator RefIterator::operator++(int) noexcept
    {
        RefIterator out(*this);
        ++(*this);
        return out;
    }

    RefIterator& RefIterator::operator++() noexcept
    {
        hrs::assert_true_debug(state != nullptr, "RefIterator must be valid!");

        lua_rawgeti(state, LUA_REGISTRYINDEX, iterable_ref);
        lua_rawgeti(state, LUA_REGISTRYINDEX, key_ref);
        //iter, key
        int res = lua_next(state, -2);
        if(res == 0)
        {
            //iter
            lua_pop(state, 1);
            destroy();
            return *this;
        }

        //iter, key, value
        luaL_unref(state, LUA_REGISTRYINDEX, key_ref);
        lua_pop(state, 1);
        //iter, key
        key_ref = luaL_ref(state, LUA_REGISTRYINDEX);
        //iter
        lua_pop(state, 1);

        return *this;
    }

    bool RefIterator::operator==(const RefIterator& ri) const noexcept
    {
        if(state != ri.state)
            return false;

        if(iterable_ref != ri.iterable_ref)
            return false;

        return key_ref == ri.key_ref;
    }

    void RefIterator::destroy() noexcept
    {
        if(state == nullptr)
            return;

        luaL_unref(state, LUA_REGISTRYINDEX, iterable_ref);
        luaL_unref(state, LUA_REGISTRYINDEX, key_ref);

        state = nullptr;
    }
};
