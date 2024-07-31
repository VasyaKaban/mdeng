#pragma once

#include "Bounded.h"

namespace FireLand
{
	struct BoundedBufferSize : public BoundedBuffer
	{
		VkDeviceSize size;

		BoundedBufferSize(BoundedBuffer &&bb = {},
						  VkDeviceSize _size = 0) noexcept;

		~BoundedBufferSize() = default;

		BoundedBufferSize(BoundedBufferSize &&buf) noexcept;
		BoundedBufferSize & operator=(BoundedBufferSize &&buf) noexcept;
	};

	struct BoundedImageSize : public BoundedImage
	{
		VkDeviceSize size;

		BoundedImageSize(BoundedImage &&bi = {},
						 VkDeviceSize _size = 0) noexcept;

		~BoundedImageSize() = default;

		BoundedImageSize(BoundedImageSize &&img) noexcept;
		BoundedImageSize & operator=(BoundedImageSize &&img) noexcept;
	};
};
