#pragma once

#include "Allocator.h"

namespace FireLand
{
	struct BoundedBufferSize : public hrs::non_copyable
	{
		BoundedBuffer bounded_buffer;
		vk::DeviceSize size;

		BoundedBufferSize(BoundedBuffer &&_bounded_buffer = {}, vk::DeviceSize _size = {}) noexcept
			: bounded_buffer(std::move(_bounded_buffer)), size(_size) {}

		BoundedBufferSize(BoundedBufferSize &&bbs) noexcept
			: bounded_buffer(std::move(bbs.bounded_buffer)),
			  size(bbs.size) {}

		BoundedBufferSize & operator=(BoundedBufferSize &&bbs) noexcept
		{
			bounded_buffer = std::move(bbs.bounded_buffer);
			size = bbs.size;

			return *this;
		}
	};

	struct BoundedImageSize : public hrs::non_copyable
	{
		BoundedImage bounded_image;
		vk::DeviceSize size;

		BoundedImageSize(BoundedImage &&_bounded_image = {}, vk::DeviceSize _size = {}) noexcept
			: bounded_image(std::move(_bounded_image)), size(_size) {}

		BoundedImageSize(BoundedImageSize &&bbs) noexcept
			: bounded_image(std::move(bbs.bounded_image)),
			  size(bbs.size) {}

		BoundedImageSize & operator=(BoundedImageSize &&bbs) noexcept
		{
			bounded_image = std::move(bbs.bounded_image);
			size = bbs.size;

			return *this;
		}
	};
};
