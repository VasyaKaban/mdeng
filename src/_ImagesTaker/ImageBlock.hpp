#pragma once

#include "../Vulkan/VulkanInclude.hpp"
#include "../hrs/block.hpp"

namespace FireLand
{
	struct ImageBlock
	{
		vk::Image image;
		hrs::block<vk::DeviceSize> block;

		constexpr ImageBlock(vk::Image _image, const hrs::block<vk::DeviceSize> &_block) noexcept
			: image(_image), block(_block) {}
		~ImageBlock() = default;
		ImageBlock(const ImageBlock &) = default;
		ImageBlock( ImageBlock &&) = default;
		ImageBlock & operator=(const ImageBlock &) = default;
		ImageBlock & operator=(ImageBlock &&) = default;
	};
};
