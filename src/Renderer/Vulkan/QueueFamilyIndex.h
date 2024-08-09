#pragma once

#include <cstdint>
#include "VulkanInclude.h"

namespace FireLand
{
	struct QueueFamilyIndex
	{
		VkQueue queue;
		std::uint32_t family_index;
	};
};
