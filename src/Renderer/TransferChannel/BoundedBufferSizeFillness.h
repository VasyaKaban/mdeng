#pragma once

#include "../Allocator/BoundedSize.h"

namespace FireLand
{
	struct BoundedBufferSizeFillness : public BoundedBufferSize
	{
		VkDeviceSize fillness;

		BoundedBufferSizeFillness(BoundedBufferSize &&bbs = {},
								  VkDeviceSize _fillness = 0) noexcept;

		~BoundedBufferSizeFillness() = default;
		BoundedBufferSizeFillness(BoundedBufferSizeFillness &&bbsf) noexcept;
		BoundedBufferSizeFillness & operator=(BoundedBufferSizeFillness &&bbsf) noexcept;

		bool IsFull() const noexcept;

		//sets fillness after appending new size required in mem_req
		//returns aligned fillness without size
		std::optional<VkDeviceSize> Append(const hrs::mem_req<VkDeviceSize> &req) noexcept;
	};
};
