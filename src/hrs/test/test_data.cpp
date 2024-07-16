#include "test_data.h"

namespace hrs
{
	namespace test
	{
		test_data::test_data(std::function<void ()> &&_func,
							 std::string_view _name,
							 std::string_view _tag,
							 hrs::flags<test_property> _properties)
			: func(std::move(_func)),
			  name(_name),
			  tag(_tag),
			  properties(_properties) {}

		void test_data::operator()() const
		{
			func();
		}

		const std::string & test_data::get_name() const noexcept
		{
			return name;
		}

		const std::string & test_data::get_tag() const noexcept
		{
			return tag;
		}

		hrs::flags<test_property> test_data::get_properties() const noexcept
		{
			return properties;
		}
	};
};
