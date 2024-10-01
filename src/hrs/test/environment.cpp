#include "environment.h"

namespace hrs
{
    namespace test
    {
        assert_exception::assert_exception(std::string_view _assert_message,
                                           std::string_view _description,
                                           const std::source_location& _loc)
            : assert_message(_assert_message),
              description(_description),
              loc(_loc)
        {}

        assert_exception::assert_exception(std::string_view _assert_message,
                                           const std::source_location& _loc)
            : assert_message(_assert_message),
              loc(_loc)
        {}

        assert_exception& assert_exception::set_description(std::string_view _description)
        {
            description = _description;
            return *this;
        }

        const std::string& assert_exception::get_assert_message() const noexcept
        {
            return assert_message;
        }

        const std::string& assert_exception::get_description() const noexcept
        {
            return description;
        }

        const std::source_location& assert_exception::get_location() const noexcept
        {
            return loc;
        }

        environment::config::config(std::function<output_f>&& _output_function,
                                    std::function<end_output_f>&& _end_output_function,
                                    ignore_group_names_container&& _ignore_group_names)
            : output_function(std::move(_output_function)),
              end_output_function(std::move(_end_output_function)),
              ignore_group_names(std::move(_ignore_group_names))
        {
            if(!output_function)
                output_function = environment::DEFAULT_OUTPUT;

            if(!end_output_function)
                end_output_function = environment::DEFAULT_END_OUTPUT;
        }

        void environment::config::set_ouput_function(std::function<output_f>&& _output_function)
        {
            if(_output_function)
                output_function = std::move(_output_function);
        }

        void environment::config::set_output_end_function(
            std::function<end_output_f>&& _end_output_function)
        {
            if(_end_output_function)
                end_output_function = std::move(_end_output_function);
        }

        void environment::config::set_ignore_group_names(
            ignore_group_names_container&& _ignore_group_names)
        {
            ignore_group_names = std::move(_ignore_group_names);
        }

        void environment::config::erase_ignore_group_name(std::string_view name)
        {
            auto it = ignore_group_names.find(name);
            if(it != ignore_group_names.end())
                ignore_group_names.erase(it);
        }

        void environment::config::insert_ignore_group_name(std::string_view name)
        {
            auto it = ignore_group_names.find(name);
            if(it != ignore_group_names.end())
                ignore_group_names.insert(std::string(name));
        }

        environment::environment()
            : cfg{DEFAULT_OUTPUT, DEFAULT_END_OUTPUT}
        {}

        environment::environment(const environment::config& _cfg)
            : cfg(_cfg)
        {}

        void environment::add_test(std::function<void()>&& func, const test_config& t_cfg)
        {
            test_data test(std::move(func),
                           t_cfg.get_name(),
                           t_cfg.get_tag(),
                           t_cfg.get_properties());
            add_test(std::move(test), t_cfg.get_group());
        }

        void environment::add_test(void (*func)(), const test_config& t_cfg)
        {
            test_data test(func, t_cfg.get_name(), t_cfg.get_tag(), t_cfg.get_properties());
            add_test(std::move(test), t_cfg.get_group());
        }

        void environment::run()
        {
            std::size_t success_count = 0;
            std::size_t failed_count = 0;
            std::size_t ignored_count = 0;
            std::size_t success_due_failure_count = 0;

            std::size_t global_i = 0;
            for(const auto& [group_name, group_tests]: tests)
            {
                bool ignore_group =
                    cfg.ignore_group_names.find(group_name) != cfg.ignore_group_names.end();

                std::size_t within_group_i = 0;
                for(const test_data& test: group_tests)
                {
                    std::exception_ptr err = std::exception_ptr(nullptr);
                    test_result test_res = test_result::success;
                    if(ignore_group)
                    {
                        test_res = test_result::ignored;
                    }
                    else
                    {
                        try
                        {
                            if(test.get_properties() & test_property::ignore)
                                test_res = test_result::ignored;
                            else
                                test();
                        }
                        catch(...)
                        {
                            if(test.get_properties() & test_property::may_fail)
                                test_res = test_result::success_due_failure;
                            else
                                test_res = test_result::failed;

                            err = std::current_exception();
                        }
                    }

                    cfg.output_function(global_i, within_group_i, group_name, test, test_res, err);
                    global_i++;
                    within_group_i++;

                    switch(test_res)
                    {
                        case test_result::success:
                            success_count++;
                            break;
                        case test_result::failed:
                            failed_count++;
                            break;
                        case test_result::ignored:
                            ignored_count++;
                            break;
                        case test_result::success_due_failure:
                            success_due_failure_count++;
                            break;
                    }
                }
            }

            cfg.end_output_function(global_i,
                                    success_count,
                                    failed_count,
                                    ignored_count,
                                    success_due_failure_count,
                                    tests);
        }

        void environment::set_config(config&& _cfg) noexcept
        {
            cfg = std::move(_cfg);
        }

        environment& environment::get_global_environment()
        {
            static environment global_env;
            return global_env;
        }

        void environment::DEFAULT_OUTPUT(std::size_t global_num,
                                         std::size_t num_within_group,
                                         const std::string& group,
                                         const test_data& test,
                                         test_result test_res,
                                         std::exception_ptr err)
        {
            constexpr auto success_fmt = "#(g:{}/w:{}) group: {} [{}] Test: {} -> {}\n";
            constexpr auto msg_fmt = "#(g:{}/w:{}) group: {} [{}] Test: {} -> {} ({})\n";
            constexpr auto assert_ex_fmt =
                "#(g:{}/w:{}) group: {} [{}] Test: {} -> {} ({})"
                " -> source: l:c({}:{}), in file: {}, function: {} with "
                "description: {}\n";

            std::string_view test_res_str;
            switch(test_res)
            {
                case test_result::success:
                    test_res_str = "Success";
                    break;
                case test_result::failed:
                    test_res_str = "Failed";
                    break;
                case test_result::success_due_failure:
                    test_res_str = "Succes due failure";
                    break;
                case test_result::ignored:
                    test_res_str = "Ignored";
                    break;
            }

            if(test_res == test_result::failed || test_res == test_result::success_due_failure)
            {
                std::ostream& os = (test_res == test_result::failed ? std::cerr : std::clog);
                try
                {
                    std::rethrow_exception(err);
                }
                catch(const assert_exception& tae)
                {
                    const auto& loc = tae.get_location();
                    os << std::format(assert_ex_fmt,
                                      global_num,
                                      num_within_group,
                                      group,
                                      test.get_tag(),
                                      test.get_name(),
                                      test_res_str,
                                      tae.get_assert_message(),
                                      loc.line(),
                                      loc.column(),
                                      loc.file_name(),
                                      loc.function_name(),
                                      tae.get_description());
                }
                catch(const std::exception& ex)
                {
                    os << std::format(msg_fmt,
                                      global_num,
                                      num_within_group,
                                      group,
                                      test.get_tag(),
                                      test.get_name(),
                                      test_res_str,
                                      ex.what());
                }
                catch(...)
                {
                    os << std::format(msg_fmt,
                                      global_num,
                                      num_within_group,
                                      group,
                                      test.get_tag(),
                                      test.get_name(),
                                      test_res_str,
                                      "Unknown exception!");
                }
            }
            else
            {
                std::clog << std::format(success_fmt,
                                         global_num,
                                         num_within_group,
                                         group,
                                         test.get_tag(),
                                         test.get_name(),
                                         test_res_str);
            }
        }

        void environment::DEFAULT_END_OUTPUT(std::size_t test_count,
                                             std::size_t success_count,
                                             std::size_t failed_count,
                                             std::size_t ignored_count,
                                             std::size_t success_due_failure_count,
                                             const tests_t& tests)
        {
            std::clog << std::format(
                "SUMMARY: Test count: {}; Success: {}; Failed: {}; Ignored: {};"
                " Success due failure: {}\n",
                test_count,
                success_count,
                failed_count,
                ignored_count,
                success_due_failure_count);
        }

        void environment::add_test(test_data&& test, std::string_view group)
        {
            auto group_it = tests.find(group);
            if(group_it == tests.end())
                group_it = tests.emplace(group, std::list<test_data>{}).first;

            group_it->second.push_back(std::move(test));
        }
    };
};
