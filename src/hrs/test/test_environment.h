#pragma once

#include <map>
#include <list>
#include "test_data.h"

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

		class test_assert_exception
		{
		public:
			test_assert_exception(std::string_view _assert_message,
								  std::string_view _description,
								  const std::source_location &_loc = std::source_location::current());

			test_assert_exception(std::string_view _assert_message,
								  const std::source_location &_loc = std::source_location::current());

			test_assert_exception(const test_assert_exception &) = default;
			test_assert_exception(test_assert_exception &&) = default;
			test_assert_exception & operator=(const test_assert_exception &) = default;
			test_assert_exception & operator=(test_assert_exception &&) = default;

			test_assert_exception & set_description(std::string_view _description) &;
			test_assert_exception && set_description(std::string_view _description) &&;

			const std::string & get_assert_message() const noexcept;
			const std::string & get_description() const noexcept;
			const std::source_location & get_location() const noexcept;

		private:
			std::string assert_message;
			std::string description;
			std::source_location loc;
		};

		class test_environment
		{
		public:
			struct test_group_comparator
			{
				using is_transparent = void;

				bool operator()(const std::string &str1, const std::string &str2) const noexcept
				{
					return str1 < str2;
				}

				bool operator()(const std::string &str, std::string_view sv) const noexcept
				{
					return str < sv;
				}

				bool operator()(std::string_view sv, const std::string &str) const noexcept
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
								  const std::string &group,
								  const test_data &test,
								  test_result test_res,
								  std::exception_ptr err);

			using end_output_f = void(std::size_t test_count,
									  std::size_t success_count,
									  std::size_t failed_count,
									  std::size_t ignored_count,
									  std::size_t success_due_failure_count,
									  const tests_t &tests);

			test_environment() = default;
			~test_environment() = default;

			void add_test(std::function<void ()> &&func,
						  std::string_view name,
						  std::string_view tag = {},
						  std::string_view group = {},
						  hrs::flags<test_property> properties = test_property::none);

			void add_test(void (*func)(),
						  std::string_view name,
						  std::string_view tag = {},
						  std::string_view group = {},
						  hrs::flags<test_property> properties = test_property::none);

			/*template<typename F, typename A>
			void add_test(F &&_func,
						  std::string_view name,
						  std::string_view tag = {},
						  std::string_view group = {},
						  hrs::flags<test_property> properties = test_property::none,
						  A &&_arg = {})
			{
				auto wrapped = [func = std::forward<F>(_func), arg = std::forward<A>(_arg)]() -> void
				{
					func(arg);
				};

				add_test(std::move(wrapped), name, tag, group, properties);
			}*/

			void run();

			void set_output_function(std::function<output_f> &&out_func) noexcept;
			void set_end_output_function(std::function<end_output_f> &&out_func) noexcept;

			static test_environment & get_global_environment();

			static void DEFAULT_OUTPUT(std::size_t global_num,
									   std::size_t num_within_group,
									   const std::string &group,
									   const test_data &test,
									   test_result test_res,
									   std::exception_ptr err);

			static void DEFAULT_END_OUTPUT(std::size_t test_count,
										   std::size_t success_count,
										   std::size_t failed_count,
										   std::size_t ignored_count,
										   std::size_t success_due_failure_count,
										   const tests_t &tests);

		private:
			void add_test(test_data &&test, std::string_view group);
		private:
			std::function<output_f> output_function = DEFAULT_OUTPUT;
			std::function<end_output_f> end_output_function = DEFAULT_END_OUTPUT;
			tests_t tests;

		};
	};
};
