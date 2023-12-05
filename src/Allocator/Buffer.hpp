/**
 * @file
 *
 * Represents buffer class of vulkan allocator
 */

#pragma once

#include "../Vulkan/VulkanInclude.hpp"
#include "../hrs/debug.hpp"
#include "Memory.hpp"

namespace FireLand
{
	/**
	 * @brief The Buffer class
	 *
	 * Encapsulates memory and buffer
	 */
	struct Buffer
	{
		Memory memory;///<Device memory
		vk::Buffer buffer;///<buffer that takes whole memory

		constexpr Buffer(Memory mem = {}, vk::Buffer _buffer = VK_NULL_HANDLE)
			: memory(mem), buffer(_buffer) {}

		constexpr Buffer(const Buffer &) noexcept = default;

		constexpr Buffer(Buffer &&buf) noexcept
			: memory(std::move(buf.memory)), buffer(buf.buffer)
		{
			buf.buffer = VK_NULL_HANDLE;
		}

		constexpr Buffer & operator=(const Buffer &) noexcept = default;

		constexpr Buffer & operator=(Buffer &&buf) noexcept
		{
			memory =std::move(buf.memory);
			buffer = buf.buffer;
			buf.buffer = VK_NULL_HANDLE;
			return *this;
		}

		/**
		 * @brief operator bool
		 *
		 * Checks whether object is created or not.
		 * Just checks that memory and buffer aren't VK_NULL_HANDLE
		 */
		constexpr explicit operator bool() const noexcept
		{
			return memory && buffer;
		}

		/**
		 * @brief Destroy
		 * @param device the device that created this buffer
		 *
		 * Destroys buffer and frees memory
		 * @warning Aborts if device isn't created!
		 */
		void Destroy(vk::Device device)
		{
			if(*this)
			{
				hrs::assert_true_debug(device, "Device isn't created yet!");
				device.destroy(buffer);
				memory.Free(device);

				buffer = VK_NULL_HANDLE;
			}
		}
	};
};
