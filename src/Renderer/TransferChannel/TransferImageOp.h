#pragma once

#include "../Vulkan/VulkanInclude.h"
#include "hrs/block.hpp"

namespace FireLand
{
	struct TransferImageOpRegion
	{
		VkImageSubresourceLayers subresource_layers;
		VkExtent3D image_extent;
		VkDeviceSize data_offset;
		std::size_t data_index;
	};
};

