/**
 * @file
 *
 * Used for formatted assertions
 */

#pragma once

#include "stacktrace.hpp"
#include <cstdint>
#include <cstdlib>
#include <format>
#include <iostream>
#include <source_location>

namespace hrs
{
    namespace detail
    {
        template<typename... Args>
        [[noreturn]] void write_data_and_terminate(std::FILE* fd,
                                                   std::format_string<Args...> fmt,
                                                   const std::source_location& loc,
                                                   Args&&... args) noexcept
        {
            try
            {
                const stacktrace stack_trace(32, 1);
                std::string stacktrace_str;
                stacktrace_str.reserve(stack_trace.size() * 512);
                for(std::size_t i = 0; i < stack_trace.size(); i++)
                    stacktrace_str.append(std::format("#{}: {}: {}\n",
                                                      i,
                                                      stack_trace[i].object_path(),
                                                      stack_trace[i].function_name()));

                auto str = std::format(
                    "ASSERTION FAILED!\nFile: {}\nFunction: {}\nLine:column: "
                    "{}:{}\n"
                    "Message: {}\n"
                    "Stacktrace:\n{}",
                    loc.file_name(),
                    loc.function_name(),
                    loc.line(),
                    loc.column(),
                    std::format(fmt, std::forward<Args>(args)...),
                    stacktrace_str);

                std::fwrite(str.data(), sizeof(char), str.size(), fd);
                std::fflush(fd);
            }
            catch(...)
            {}
            abort();
        }
    };

    template<typename... Args>
    struct assert_true
    {
        assert_true(std::FILE* fd,
                    bool condition,
                    std::format_string<Args...> fmt,
                    Args&&... args,
                    const std::source_location& loc = std::source_location::current()) noexcept
        {
            if(!condition)
                detail::write_data_and_terminate(fd, fmt, loc, std::forward<Args>(args)...);
        }

        assert_true(bool condition,
                    std::format_string<Args...> fmt,
                    Args&&... args,
                    const std::source_location& loc = std::source_location::current()) noexcept
        {
            if(!condition)
                detail::write_data_and_terminate(stderr, fmt, loc, std::forward<Args>(args)...);
        }
    };

    template<typename... Args>
    assert_true(std::FILE* fd, bool, std::format_string<Args...>, Args&&...)
        -> assert_true<Args...>;

    template<typename... Args>
    assert_true(bool, std::format_string<Args...>, Args&&...) -> assert_true<Args...>;

    template<typename... Args>
    struct assert_true_debug
    {
        assert_true_debug(
            std::FILE* fd,
            bool condition,
            std::format_string<Args...> fmt,
            Args&&... args,
            const std::source_location& loc = std::source_location::current()) noexcept
        {
#ifndef NDEBUG
            if(!condition)
                detail::write_data_and_terminate(fd, fmt, loc, std::forward<Args>(args)...);
#endif
        }

        assert_true_debug(
            bool condition,
            std::format_string<Args...> fmt,
            Args&&... args,
            const std::source_location& loc = std::source_location::current()) noexcept
        {
#ifndef NDEBUG
            if(!condition)
                detail::write_data_and_terminate(stderr, fmt, loc, std::forward<Args>(args)...);
#endif
        }
    };

    template<typename... Args>
    assert_true_debug(std::FILE* fd, bool, std::format_string<Args...>, Args&&...)
        -> assert_true_debug<Args...>;

    template<typename... Args>
    assert_true_debug(bool, std::format_string<Args...>, Args&&...) -> assert_true_debug<Args...>;

    template<std::ranges::input_range R>
    bool is_iterator_part_of_range(const R& rng,
                                   decltype(std::ranges::cbegin(rng)) iterator) noexcept
    {
        for(auto it = std::ranges::cbegin(rng); it != std::ranges::cend(rng); it++)
            if(it == iterator)
                return true;

        return false;
    }

    template<std::ranges::input_range R>
    bool is_iterator_part_of_range_debug(const R& rng,
                                         decltype(std::ranges::cbegin(rng)) iterator) noexcept
    {
#ifndef NDEBUG
        return is_iterator_part_of_range(rng, iterator);
#endif

        return true;
    }

    template<std::invocable<std::source_location> F>
    void execute_on_debug(F&& func,
                          const std::source_location& loc = std::source_location::current())
    {
#ifndef NDEBUG
        std::forward<F>(func)(loc);
#endif
    }
};
