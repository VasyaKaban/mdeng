#pragma once

#include "../Vulkan/VulkanInclude.h"
#include "hrs/block.hpp"

namespace FireLand
{
	struct TransferBufferOpRegion
	{
		//TransferChannel has fucntions like:
		// CopyBuffer(vector<TransferBufferOpRegion> regions, vector<const std::byte *> datas)
		//data_index -> index of data within datas vector used for this Op
		//dst_buffer_offset -> destination buffer offset
		//data_blk -> offset within datas[data_index] and size of datas[data_index]
		hrs::block<VkDeviceSize> data_blk;
		VkDeviceSize dst_buffer_offset;
		std::size_t data_index;
	};
};
