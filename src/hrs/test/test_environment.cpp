#include "test_environment.h"

namespace hrs
{
	namespace test
	{
		test_assert_exception::test_assert_exception(std::string_view _assert_message,
													 std::string_view _description,
													 const std::source_location &_loc)
			: assert_message(_assert_message),
			  description(_description),
			  loc(_loc) {}

		test_assert_exception::test_assert_exception(std::string_view _assert_message,
													 const std::source_location &_loc)
			: assert_message(_assert_message),
			  loc(_loc) {}

		test_assert_exception & test_assert_exception::set_description(std::string_view _description) &
		{
			description = _description;
			return *this;
		}

		test_assert_exception && test_assert_exception::set_description(std::string_view _description) &&
		{
			description = _description;
			return std::move(*this);
		}

		const std::string & test_assert_exception::get_assert_message() const noexcept
		{
			return assert_message;
		}

		const std::string & test_assert_exception::get_description() const noexcept
		{
			return description;
		}

		const std::source_location & test_assert_exception::get_location() const noexcept
		{
			return loc;
		}

		void test_environment::add_test(std::function<void ()> &&func,
										std::string_view name,
										std::string_view tag,
										std::string_view group,
										hrs::flags<test_property> properties)
		{
			test_data test(std::move(func), name, tag, properties);
			add_test(std::move(test), group);
		}

		void test_environment::add_test(void (*func)(),
										std::string_view name,
										std::string_view tag,
										std::string_view group,
										hrs::flags<test_property> properties)
		{
			test_data test(func, name, tag, properties);
			add_test(std::move(test), group);
		}

		void test_environment::run()
		{
			std::size_t success_count = 0;
			std::size_t failed_count = 0;
			std::size_t ignored_count = 0;
			std::size_t success_due_failure_count = 0;

			std::size_t global_i = 0;
			for(const auto &[group_name, group_tests] : tests)
			{
				std::size_t within_group_i = 0;
				for(const test_data &test : group_tests)
				{
					std::exception_ptr err = std::exception_ptr(nullptr);
					test_result test_res = test_result::success;
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

					output_function(global_i, within_group_i, group_name, test, test_res, err);
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

			end_output_function(global_i,
								success_count,
								failed_count,
								ignored_count,
								success_due_failure_count,
								tests);
		}

		void test_environment::set_output_function(std::function<output_f> &&out_func) noexcept
		{
			if(out_func)
				output_function = std::move(out_func);
		}

		void test_environment::set_end_output_function(std::function<end_output_f> &&out_func) noexcept
		{
			if(out_func)
				end_output_function = std::move(out_func);
		}

		test_environment & test_environment::get_global_environment()
		{
			static test_environment global_env;
			return global_env;
		}

		void test_environment::DEFAULT_OUTPUT(std::size_t global_num,
											  std::size_t num_within_group,
											  const std::string &group,
											  const test_data &test,
											  test_result test_res,
											  std::exception_ptr err)
		{	
			constexpr auto success_fmt = "#(g:{}/w:{}) group: {} [{}] Test: {} -> {}\n";
			constexpr auto msg_fmt = "#(g:{}/w:{}) group: {} [{}] Test: {} -> {} ({})\n";
			constexpr auto assert_ex_fmt = "#(g:{}/w:{}) group: {} [{}] Test: {} -> {} ({})"
										   " -> source: l:c({}:{}), in file: {}, function: {} with description: {}\n";

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
				std::ostream &os = (test_res == test_result::failed ? std::cerr : std::clog);
				try
				{
					std::rethrow_exception(err);
				}
				catch(const test_assert_exception &tae)
				{
					const auto &loc = tae.get_location();
					os<<std::format(assert_ex_fmt,
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
				catch(const std::exception &ex)
				{
					os<<std::format(msg_fmt,
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
					os<<std::format(msg_fmt,
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
				std::clog<<std::format(success_fmt,
										 global_num,
										 num_within_group,
										 group,
										 test.get_tag(),
										 test.get_name(),
										 test_res_str);
			}
		}

		void test_environment::DEFAULT_END_OUTPUT(std::size_t test_count,
												  std::size_t success_count,
												  std::size_t failed_count,
												  std::size_t ignored_count,
												  std::size_t success_due_failure_count,
												  const tests_t &tests)
		{
			std::clog<<std::format("SUMMARY: Test count: {}; Success: {}; Failed: {}; Ignored: {};"
									 " Success due failure: {}\n",
									 test_count,
									 success_count,
									 failed_count,
									 ignored_count,
									 success_due_failure_count);
		}

		void test_environment::add_test(test_data &&test, std::string_view group)
		{
			auto group_it = tests.find(group);
			if(group_it == tests.end())
				group_it = tests.emplace(group, std::list<test_data>{}).first;

			group_it->second.push_back(std::move(test));
		}
	};
};
