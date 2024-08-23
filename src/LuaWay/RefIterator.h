#pragma once

#include <lua5.1/lua.hpp>
#include <utility>

namespace LuaWay
{
	class Ref;

	class RefIterator
	{
	private:
		friend class Ref;

		RefIterator(lua_State *_state, int _iterable_ref, int _key_ref) noexcept;
	public:
		RefIterator();
		~RefIterator();
		RefIterator(const RefIterator &ri) noexcept;
		RefIterator(RefIterator &&ri) noexcept;
		RefIterator & operator=(const RefIterator &ri) noexcept;
		RefIterator & operator=(RefIterator &&ri) noexcept;

		std::pair<Ref, Ref> operator*() const noexcept;//key, value

		RefIterator operator++(int) noexcept;
		RefIterator & operator++() noexcept;

		bool operator==(const RefIterator &ri) const noexcept;

	private:
		void destroy() noexcept;
	private:
		lua_State *state;
		int iterable_ref;
		int key_ref;
	};
};
