#include "test_config.h"

namespace hrs
{
    namespace test
    {
        test_config::test_config() noexcept
            : properties(test_property::none)
        {}

        test_config& test_config::set_name(std::string_view _name)
        {
            name = _name;
            return *this;
        }

        test_config& test_config::set_group(std::string_view _group)
        {
            group = _group;
            return *this;
        }

        test_config& test_config::set_tag(std::string_view _tag)
        {
            tag = _tag;
            return *this;
        }

        test_config& test_config::set_properties(hrs::flags<test_property> _properties)
        {
            properties = _properties;
            return *this;
        }

        std::string_view test_config::get_name() const noexcept
        {
            return name;
        }

        std::string_view test_config::get_group() const noexcept
        {
            return group;
        }

        std::string_view test_config::get_tag() const noexcept
        {
            return tag;
        }

        hrs::flags<test_property> test_config::get_properties() const noexcept
        {
            return properties;
        }
    };
};
