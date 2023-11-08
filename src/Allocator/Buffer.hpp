#pragma once

#include "../Vulkan/VulkanInclude.hpp"
#include "../hrs/debug.hpp"

namespace FireLand
{
	struct Buffer
	{
		vk::DeviceMemory memory;
		vk::Buffer buffer;
		vk::DeviceSize size;
		std::byte *map_ptr;

		constexpr Buffer(vk::DeviceMemory _memory = {},
						 vk::Buffer _buffer = {},
						 vk::DeviceSize _size = {},
						 std::byte *_map_ptr = nullptr)
			: memory(_memory), buffer(_buffer),
			  size(_size), map_ptr(_map_ptr) {}

		constexpr Buffer(const Buffer &) noexcept = default;

		constexpr Buffer(Buffer &&buf) noexcept
			: memory(buf.memory), buffer(buf.buffer),
			  size(buf.size), map_ptr(buf.map_ptr)
		{
			buf.memory = nullptr;
			buf.buffer = nullptr;
			buf.size = 0;
			buf.map_ptr = nullptr;
		}

		constexpr auto operator=(const Buffer &) noexcept -> Buffer & = default;

		constexpr auto operator=(Buffer &&buf) noexcept -> Buffer &
		{
			memory = buf.memory;
			buffer = buf.buffer;
			size = buf.size;
			map_ptr = buf.map_ptr;
			buf.memory = nullptr;
			buf.buffer = nullptr;
			buf.size = 0;
			buf.map_ptr = nullptr;
			return *this;
		}

		constexpr explicit operator bool() const noexcept
		{
			return memory && buffer;
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
				device.destroy(buffer);
				device.free(memory);
			}
		}
	};
};
