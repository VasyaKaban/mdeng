#pragma once

#include <map>
#include <list>
#include <unordered_set>
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

			template<typename ...Args>
			test_assert_exception & set_description_fmt(std::format_string<Args...> fmt, Args &&...args) &
			{
				return set_description(std::format(fmt, std::forward<Args>(args)...));
			}

			template<typename ...Args>
			test_assert_exception && set_description_fmt(std::format_string<Args...> fmt, Args &&...args) &&
			{
				return std::move(*this).set_description(std::format(fmt, std::forward<Args>(args)...));
			}

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

			class test_config
			{
			public:

				friend class test_environment;

				struct ignore_groups_hasher
				{
					using is_transparent = void;

					std::size_t operator()(const std::string &str) const noexcept
					{
						return std::hash<std::string>{}(str);
					}

					std::size_t operator()(std::string_view sv) const noexcept
					{
						return std::hash<std::string_view>{}(sv);
					}
				};

				using ignore_group_names_container = std::unordered_set<std::string,
																		ignore_groups_hasher,
																		std::equal_to<>>;

				test_config() = default;
				test_config(std::function<output_f> &&_output_function,
							std::function<end_output_f> &&_end_output_function,
							ignore_group_names_container &&_ignore_group_names = {});

				~test_config() = default;
				test_config(const test_config &) = default;
				test_config(test_config &&) = default;
				test_config & operator=(const test_config &) = default;
				test_config & operator=(test_config &&) = default;

				void set_ouput_function(std::function<output_f> &&_output_function);
				void set_output_end_function(std::function<end_output_f> &&_end_output_function);
				void set_ignore_group_names(ignore_group_names_container &&_ignore_group_names);
				void erase_ignore_group_name(std::string_view name);
				void insert_ignore_group_name(std::string_view name);

			private:
				std::function<output_f> output_function = DEFAULT_OUTPUT;
				std::function<end_output_f> end_output_function = DEFAULT_END_OUTPUT;
				ignore_group_names_container ignore_group_names;
			};

			test_environment();
			test_environment(const test_config &cfg);
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

			void run();

			void set_config(test_config &&cfg) noexcept;

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
			test_config config;
			tests_t tests;

		};
	};
};
