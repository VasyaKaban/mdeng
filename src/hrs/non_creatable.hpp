#pragma once

#include <utility>

namespace hrs
{
	struct non_copy_constructible
	{
		non_copy_constructible() = default;
		non_copy_constructible(const non_copy_constructible &) = delete;
		non_copy_constructible(non_copy_constructible &&) = default;
		non_copy_constructible & operator=(const non_copy_constructible &) = default;
		non_copy_constructible & operator=(non_copy_constructible &&) = default;
	};

	struct non_move_constructible
	{
		non_move_constructible() = default;
		non_move_constructible(const non_move_constructible &) = default;
		non_move_constructible(non_move_constructible &&) = delete;
		non_move_constructible & operator=(const non_move_constructible &) = default;
		non_move_constructible & operator=(non_move_constructible &&) = default;
	};

	struct non_copy_assignable
	{
		non_copy_assignable() = default;
		non_copy_assignable(const non_copy_assignable &) = default;
		non_copy_assignable(non_copy_assignable &&) = default;
		non_copy_assignable & operator=(const non_copy_assignable &) = delete;
		non_copy_assignable & operator=(non_copy_assignable &&) = default;
	};

	struct non_move_assignable
	{
		non_move_assignable() = default;
		non_move_assignable(const non_move_assignable &) = default;
		non_move_assignable(non_move_assignable &&) = default;
		non_move_assignable & operator=(const non_move_assignable &) = default;
		non_move_assignable & operator=(non_move_assignable &&) = delete;
	};

	struct non_copyable : non_copy_constructible, non_copy_assignable {};

	struct non_movable : non_move_constructible, non_move_assignable {};
};
