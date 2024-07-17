#pragma once

#include <functional>
#include <string_view>
#include "../flags.hpp"
#include "test_property.h"

namespace hrs
{
	namespace test
	{
		class test_data
		{
		public:
			test_data(std::function<void ()> &&_func,
					 std::string_view _name,
					 std::string_view _tag,
					 hrs::flags<test_property> _properties);

			test_data(const test_data &) = default;
			test_data(test_data &&) = default;
			test_data & operator=(const test_data &) = default;
			test_data & operator=(test_data &&) = default;

			void operator()() const;

			const std::string & get_name() const noexcept;
			const std::string & get_tag() const noexcept;
			hrs::flags<test_property> get_properties() const noexcept;

		private:
			std::function<void ()> func;
			std::string name;
			std::string tag;
			hrs::flags<test_property> properties;
		};
	};
};
