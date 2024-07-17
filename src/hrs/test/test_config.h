#pragma once

#include <string>
#include "../flags.hpp"
#include "test_property.h"

namespace hrs
{
	namespace test
	{
		class test_config
		{
		public:
			test_config() noexcept;
			~test_config() = default;
			test_config(const test_config &) = default;
			test_config(test_config &&) = default;
			test_config & operator=(const test_config &) = default;
			test_config & operator=(test_config &&) = default;

			test_config & set_name(std::string_view _name);
			test_config & set_group(std::string_view _group);
			test_config & set_tag(std::string_view _tag);
			test_config & set_properties(hrs::flags<test_property> _properties);

			std::string_view get_name() const noexcept;
			std::string_view get_group() const noexcept;
			std::string_view get_tag() const noexcept;
			hrs::flags<test_property> get_properties() const noexcept;
		private:
			std::string name;
			std::string group;
			std::string tag;
			hrs::flags<test_property> properties;
		};
	};
};
