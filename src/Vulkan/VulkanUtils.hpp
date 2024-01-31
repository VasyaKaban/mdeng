#pragma once

#include "VulkanInclude.hpp"

namespace FireLand
{
	constexpr bool IsBadExtent(const vk::Extent2D extent) noexcept
	{
		return (extent.width == 0 || extent.height == 0);
	}
};
