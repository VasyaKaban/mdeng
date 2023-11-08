#pragma once

#include <cstdlib>
#include <cstdint>
#include <format>
#include <iostream>
#include <source_location>

namespace hrs
{
	template<typename T>
	concept HasFormatter = requires(T &&t)
	{
		{std::formatter<std::remove_reference_t<T>>{}};
	};

	template<HasFormatter ...Args>
	struct assert_true
	{
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

	template<HasFormatter ...Args>
	struct assert_true_debug
	{
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
};
