#pragma once

#include "Data.h"

namespace FireLand
{
	Data::Data(const std::byte *_data) noexcept
		: data(_data) {}

	bool Data::IsNonNullData() const noexcept
	{
		return data;
	}
};

