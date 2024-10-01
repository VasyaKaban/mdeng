#pragma once

#include "test_config.h"
#include "test_data.h"
#include <list>
#include <map>
#include <unordered_set>

namespace hrs
{
    namespace test
    {
        enum class test_result
        {
            success,
            failed,
            ignored,
            success_due_failure
        };

        class assert_exception
        {
        public:
            assert_exception(std::string_view _assert_message,
                             std::string_view _description,
                             const std::source_location& _loc = std::source_location::current());

            assert_exception(std::string_view _assert_message,
                             const std::source_location& _loc = std::source_location::current());

            assert_exception(const assert_exception&) = default;
            assert_exception(assert_exception&&) = default;
            assert_exception& operator=(const assert_exception&) = default;
            assert_exception& operator=(assert_exception&&) = default;

            assert_exception& set_description(std::string_view _description);

            template<typename... Args>
            assert_exception& set_description_fmt(std::format_string<Args...> fmt, Args&&... args)
            {
                return set_description(std::format(fmt, std::forward<Args>(args)...));
            }

            const std::string& get_assert_message() const noexcept;
            const std::string& get_description() const noexcept;
            const std::source_location& get_location() const noexcept;
        private:
            std::string assert_message;
            std::string description;
            std::source_location loc;
        };

        class environment
        {
        public:
            struct test_group_comparator
            {
                using is_transparent = void;

                bool operator()(const std::string& str1, const std::string& str2) const noexcept
                {
                    return str1 < str2;
                }

                bool operator()(const std::string& str, std::string_view sv) const noexcept
                {
                    return str < sv;
                }

                bool operator()(std::string_view sv, const std::string& str) const noexcept
                {
                    return sv < str;
                }

                bool operator()(std::string_view sv1, std::string_view sv2) const noexcept
                {
                    return sv1 < sv2;
                }
            };

            using tests_t = std::map<std::string, std::list<test_data>, test_group_comparator>;

            using output_f = void(std::size_t global_num,
                                  std::size_t num_within_group,
                                  const std::string& group,
                                  const test_data& test,
                                  test_result test_res,
                                  std::exception_ptr err);

            using end_output_f = void(std::size_t test_count,
                                      std::size_t success_count,
                                      std::size_t failed_count,
                                      std::size_t ignored_count,
                                      std::size_t success_due_failure_count,
                                      const tests_t& tests);

            class config
            {
            public:
                friend class environment;

                struct ignore_groups_hasher
                {
                    using is_transparent = void;

                    std::size_t operator()(const std::string& str) const noexcept
                    {
                        return std::hash<std::string>{}(str);
                    }

                    std::size_t operator()(std::string_view sv) const noexcept
                    {
                        return std::hash<std::string_view>{}(sv);
                    }
                };

                using ignore_group_names_container =
                    std::unordered_set<std::string, ignore_groups_hasher, std::equal_to<>>;

                config() = default;
                config(std::function<output_f>&& _output_function,
                       std::function<end_output_f>&& _end_output_function,
                       ignore_group_names_container&& _ignore_group_names = {});

                ~config() = default;
                config(const config&) = default;
                config(config&&) = default;
                config& operator=(const config&) = default;
                config& operator=(config&&) = default;

                void set_ouput_function(std::function<output_f>&& _output_function);
                void set_output_end_function(std::function<end_output_f>&& _end_output_function);
                void set_ignore_group_names(ignore_group_names_container&& _ignore_group_names);
                void erase_ignore_group_name(std::string_view name);
                void insert_ignore_group_name(std::string_view name);
            private:
                std::function<output_f> output_function = DEFAULT_OUTPUT;
                std::function<end_output_f> end_output_function = DEFAULT_END_OUTPUT;
                ignore_group_names_container ignore_group_names;
            };

            environment();
            environment(const config& _cfg);
            ~environment() = default;

            void add_test(std::function<void()>&& func, const test_config& t_cfg = {});

            void add_test(void (*func)(), const test_config& t_cfg = {});

            void run();

            void set_config(config&& _cfg) noexcept;

            static environment& get_global_environment();

            static void DEFAULT_OUTPUT(std::size_t global_num,
                                       std::size_t num_within_group,
                                       const std::string& group,
                                       const test_data& test,
                                       test_result test_res,
                                       std::exception_ptr err);

            static void DEFAULT_END_OUTPUT(std::size_t test_count,
                                           std::size_t success_count,
                                           std::size_t failed_count,
                                           std::size_t ignored_count,
                                           std::size_t success_due_failure_count,
                                           const tests_t& tests);
        private:
            void add_test(test_data&& test, std::string_view group);
        private:
            config cfg;
            tests_t tests;
        };
    };
};
