/**
 * @file
 *
 * Used for formatted assertions
 */

#pragma once

#include <cstdlib>
#include <cstdint>
#include <format>
#include <iostream>
#include <source_location>

namespace hrs
{
	/**
	 * @brief HasFormatter
	 * @tparam T a type that is checked for std::formatter support
	 *
	 * Checks whether type supports std::formatter
	 */
	template<typename T>
	concept HasFormatter = requires(T &&t)
	{
		{std::formatter<std::remove_reference_t<T>>{}};
	};

	/**
	 * @brief The assert_true class
	 *
	 * Used for assertion with format stringn and source location
	 */
	template<HasFormatter ...Args>
	struct assert_true
	{
		/**
		 * @brief assert_true
		 * @param condition expression that is compared with 'true'
		 * @param fmt format string
		 * @param args arguments for format string
		 * @param loc source location where assert_true was called
		 */
		constexpr assert_true(bool condition,
							  std::format_string<Args...> fmt,
							  Args &&...args,
							  const std::source_location &loc = std::source_location::current())
		{
			if(!condition)
			{
				std::cerr<<std::format("ASSERTION FAILED -> {}:{}:{}:{}\n",
										 loc.file_name(),
										 loc.function_name(),
										 loc.column(),
										 loc.line());
				std::cerr<<"MESSAGE: "<<std::format(fmt, std::forward<Args>(args)...)<<"\n";
				abort();
			}
		}
	};

	template<HasFormatter ...Args>
	assert_true(bool, std::format_string<Args...>, Args &&...) -> assert_true<Args...>;

	/**
	 * @brief The assert_true_debug class
	 *
	 * Used for assertion like assert_true class do but only on debug
	 */
	template<HasFormatter ...Args>
	struct assert_true_debug
	{
		/**
		 * @brief assert_true_debug
		 * @param condition expression that is compared with 'true'
		 * @param fmt format string
		 * @param args arguments for format string
		 * @param loc source location where assert_true was called
		 */
		constexpr assert_true_debug(bool condition,
									std::format_string<Args...> fmt,
									Args &&...args,
									const std::source_location &loc = std::source_location::current())
		{
		#ifndef NDEBUG
			assert_true(condition, fmt, std::forward<Args>(args)...);
		#endif
		}
	};

	template<HasFormatter ...Args>
	assert_true_debug(bool, std::format_string<Args...>, Args &&...) -> assert_true_debug<Args...>;

	template<std::ranges::input_range R>
	bool is_iterator_part_of_range(const R &rng, std::ranges::iterator_t<R> iterator) noexcept
	{
		for(auto it = std::ranges::begin(rng); it != std::ranges::end(rng); it++)
			if(it == iterator)
				return true;

		return false;
	}

	template<std::ranges::input_range R>
	bool is_iterator_part_of_range_debug(const R &rng, std::ranges::iterator_t<R> iterator) noexcept
	{
	#ifndef NDEBUG
		return is_iterator_part_of_range_debug(rng, iterator);
	#endif

		return true;
	}

	template<std::invocable<std::source_location> F>
	void execute_on_debug(F &&func, const std::source_location &loc = std::source_location::current())
	{
	#ifndef NDEBUG
		std::forward<F>(func)(loc);
	#endif
	}
};
