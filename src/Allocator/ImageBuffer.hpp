#pragma once

#include "../Vulkan/VulkanInclude.hpp"
#include "../hrs/debug.hpp"

namespace FireLand
{
	struct ImageBuffer
	{
		vk::DeviceMemory memory;
		vk::DeviceSize size;
		std::byte *map_ptr;

		constexpr ImageBuffer(vk::DeviceMemory _memory = {},
							  vk::DeviceSize _size = {},
							  std::byte *_map_ptr = nullptr)
			: memory(_memory), size(_size), map_ptr(_map_ptr) {}

		constexpr ImageBuffer(const ImageBuffer &) noexcept = default;

		constexpr ImageBuffer(ImageBuffer &&img_buf) noexcept
			: memory(img_buf.memory), size(img_buf.size), map_ptr(img_buf.map_ptr)
		{
			img_buf.memory = nullptr;
			img_buf.size = 0;
			img_buf.map_ptr = nullptr;
		}

		constexpr auto operator=(const ImageBuffer &) noexcept -> ImageBuffer & = default;

		constexpr auto operator=(ImageBuffer &&img_buf) noexcept -> ImageBuffer &
		{
			memory = img_buf.memory;
			size = img_buf.size;
			map_ptr = img_buf.map_ptr;
			img_buf.memory = nullptr;
			img_buf.size = 0;
			img_buf.map_ptr = nullptr;
			return *this;
		}

		constexpr explicit operator bool() const noexcept
		{
			return memory;
		}

		constexpr auto IsMapped() const noexcept -> bool
		{
			return map_ptr != nullptr;
		}

		auto Destroy(vk::Device device) -> void
		{
			if(*this)
			{
				hrs::assert_true_debug(device, "Device isn't created yet!");
				if(map_ptr)
					device.unmapMemory(memory);
				device.free(memory);
			}
		}
	};
}
