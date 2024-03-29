/**
 * @file
 *
 * Represents scoped_call class
 */

#pragma once

#include <concepts>
#include <utility>
#include <optional>

namespace hrs
{
	/**
	 * @brief The scoped_call class
	 * @tparam F must satisfy invocable concept
	 *
	 * Implements deferred call to a function object.
	 * When it's destructor is called it calls a function object.
	 * Useful in case when you have an object that doesn't frees it's resources by itself.
	 */
	template<std::invocable F>
	class scoped_call
	{
	public:
		constexpr scoped_call() = default;

		/**
		 * @brief scoped_call
		 * @param _func lvalue reference to functional object
		 *
		 * Available if type F satisfy copy constructible constraint
		 */
		constexpr scoped_call(const F &_func)
			noexcept(std::is_nothrow_copy_constructible_v<F>)
			requires std::is_copy_constructible_v<F>
			: func(_func) {}

		/**
		 * @brief scoped_call
		 * @param _func rvalue reference to functional object
		 *
		 * Available if type F satisfy move constructible constraint
		 */
		constexpr scoped_call(F &&_func)
			noexcept(std::is_nothrow_copy_constructible_v<F>)
			requires std::is_copy_constructible_v<F>
			: func(std::move(_func)) {}

		/**
		 * @brief ~scoped_call
		 *
		 * Calls function object
		 */
		constexpr ~scoped_call()
		{
			if(func)
				func.value();
		}

		/**
		 * @brief scoped_call
		 *
		 * Available if type F satisfy copy constructible constraint
		 */
		constexpr scoped_call(const scoped_call &d_destroy)
			noexcept(std::is_nothrow_copy_constructible_v<F>)
			requires std::is_copy_constructible_v<F> : func(d_destroy.func) {}

		/**
		 * @brief scoped_call
		 *
		 * Available if type F satisfy move constructible constraint
		 */
		constexpr scoped_call(scoped_call &&d_destroy)
			noexcept(std::is_nothrow_move_constructible_v<F>)
			requires std::is_move_constructible_v<F> : func(std::move(d_destroy.func)) {}

		/**
		 * @brief operator=
		 * @param d_destroy lvalue reference to scoped_call object
		 *
		 * Available if type F satisfy copy assignable constraint
		 */
		scoped_call & operator=(const scoped_call &d_destroy)
			noexcept(std::is_nothrow_copy_assignable_v<F>)
			requires std::is_copy_assignable_v<F>
		{
			func = d_destroy.func;
			return *this;
		}

		/**
		 * @brief operator=
		 * @param _func lvalue reference to functional object
		 *
		 * Available if type F satisfy copy assignable constraint
		 */
		scoped_call & operator=(const F &_func)
			noexcept(std::is_nothrow_copy_assignable_v<F>)
			requires std::is_copy_assignable_v<F>
		{
			func = std::move<F>(_func);
			return *this;
		}

		/**
		 * @brief operator=
		 * @param _func rvalue reference to functional object
		 *
		 * Available if type F satisfy move assignable constraint
		 */
		scoped_call & operator=(F &&_func)
			noexcept(std::is_nothrow_move_assignable_v<F>)
			requires std::is_move_assignable_v<F>
		{
			func = std::move<F>(_func);
			return *this;
		}

		/**
		 * @brief operator=
		 * @param d_destroy rvalue reference to scoped_call object
		 *
		 * Available if type F satisfy move assignable constraint
		 */
		scoped_call & operator=(scoped_call &&d_destroy)
			noexcept(std::is_nothrow_move_assignable_v<F>)
			requires std::is_move_assignable_v<F>
		{
			func = std::move(d_destroy.func);
			return *this;
		}

		/**
		 * @brief operator =
		 *
		 * Drops inner functional object
		 */
		scoped_call & operator=(const std::nullopt_t &) noexcept
		{
			func.reset();
			return *this;
		}

		/**
		 * @brief operator bool
		 *
		 * Checks whether functional object exists or not
		 */
		constexpr explicit operator bool() const noexcept
		{
			return func;
		}

		/**
		 * @brief Drop
		 *
		 * Sets nullopt to inner object to prevent functional object from being called
		 */
		constexpr void drop() noexcept
		{
			func = std::nullopt;
		}

		/**
		 * @brief Call
		 *
		 * Explicitly calls inner functional object
		 */
		constexpr void call() const noexcept(std::is_nothrow_invocable_v<F>)
		{
			if(func)
				func();
		}

	private:
		std::optional<F> func;///<functional object
	};
};
