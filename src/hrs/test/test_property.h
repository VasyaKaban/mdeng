#pragma once

namespace hrs
{
	namespace test
	{
		enum class test_property
		{
			none = 0,
			ignore = 1 << 0,
			may_fail = 1 << 1
		};
	};
};
